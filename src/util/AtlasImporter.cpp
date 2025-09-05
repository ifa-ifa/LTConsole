#include "AtlasImporter.h"
#include <replicant/stbl.h>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

bool AtlasImporter::run(const QString& inputDir, const QString& outputDir)
{
    qInfo() << "Starting Atlas data import...";
    qInfo() << "Input directory:" << inputDir;
    qInfo() << "Output directory:" << outputDir;

    QDir outDir(outputDir);
    if (!outDir.exists()) {
        qInfo() << "Output directory does not exist, creating it...";
        if (!outDir.mkpath(".")) {
            qCritical() << "FATAL: Could not create output directory!";
            return false;
        }
    }

    QDirIterator it(inputDir, { "*.settbll" }, QDir::Files, QDirIterator::Subdirectories);
    int filesProcessed = 0;
    while (it.hasNext()) {
        QString filePath = it.next();
        QFileInfo fileInfo(filePath);
        QString mapId = fileInfo.baseName();

        qInfo() << "Processing map:" << mapId;

        StblFile stblFile;
        if (!stblFile.loadFromFile(filePath.toStdString())) {
            qWarning() << "  -> FAILED to load or parse STBL file. Skipping.";
            continue;
        }

        const StblTable* pointTable = nullptr;
        for (const auto& table : stblFile.getTables()) {
            if (table.getName() == "point") {
                pointTable = &table;
                break;
            }
        }

        if (!pointTable) {
            qWarning() << "  -> Did not find a 'point' table in this file. Skipping.";
            continue;
        }

        QJsonArray gamePointsArray;
        for (size_t i = 0; i < pointTable->getRowCount(); ++i) {
            const StblRow& row = pointTable->getRow(i);

            if (row.size() < 5) continue;

            auto xOpt = row.at(2).getFloat();
            auto yOpt = row.at(3).getFloat();
            auto zOpt = row.at(4).getFloat();

            if (!xOpt || !yOpt || !zOpt) {
                qWarning() << "  -> Skipping a point because it's missing coordinate data.";
                continue;
            }

            auto nameOpt = row.at(0).getString();
            QString pointName;

            if (nameOpt && !nameOpt->empty()) {
                pointName = QString::fromStdString(*nameOpt);
            }
            else {

                pointName = QString("Point [%1, %2, %3]")
                    .arg(*xOpt, 0, 'f', 1)
                    .arg(*yOpt, 0, 'f', 1)
                    .arg(*zOpt, 0, 'f', 1);
            }

            QJsonObject pointObject;
            pointObject["name"] = pointName;

            QJsonObject posObject;
            posObject["x"] = *xOpt;
            posObject["y"] = *yOpt;
            posObject["z"] = *zOpt;

            pointObject["pos"] = posObject;
            gamePointsArray.append(pointObject);
        }

        qInfo() << "  -> Extracted" << gamePointsArray.count() << "game points.";

        QJsonObject rootObject;
        rootObject["mapId"] = mapId;
        rootObject["imageFile"] = QString("resource/maps/%1.png").arg(mapId);

        QJsonObject calibrationObject;
        calibrationObject["isCalibrated"] = false;
        calibrationObject["game_x_min"] = 0.0;
        calibrationObject["game_y_min"] = 0.0;
        calibrationObject["game_x_max"] = 0.0;
        calibrationObject["game_y_max"] = 0.0;
        calibrationObject["image_crop_x"] = 0;
        calibrationObject["image_crop_y"] = 0;
        calibrationObject["image_crop_w"] = 0;
        calibrationObject["image_crop_h"] = 0;
        rootObject["calibration"] = calibrationObject;

        rootObject["gamePoints"] = gamePointsArray;
        rootObject["userPoints"] = QJsonArray();

        rootObject["pointNotes"] = QJsonObject();

        QFile outFile(outDir.filePath(mapId + ".json"));
        if (!outFile.open(QIODevice::WriteOnly)) {
            qWarning() << "  -> FAILED to open output file for writing. Skipping.";
            continue;
        }

        QJsonDocument doc(rootObject);
        outFile.write(doc.toJson(QJsonDocument::Indented));
        filesProcessed++;
    }

    qInfo() << "\nImport complete. Processed" << filesProcessed << "files.";
    return true;
}