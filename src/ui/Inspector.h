#pragma once

#include <QWidget>
#include <QTreeWidget>
#include <QTimer>
#include <map>

// Forward declarations
namespace replicant::raw { struct RawWeaponBody; }

class QLineEdit;

class Inspector : public QWidget
{
    Q_OBJECT

public:
    explicit Inspector(QWidget* parent = nullptr);

private slots:
    void refreshData();
    void onItemChanged(QTreeWidgetItem* item, int column);
    void updateWidgetState();
    void onSearchChanged(const QString& text);

private:
    void setupUi();
    void applyStyling();
    void setupConnections();

    void populateTree();
    void populateSaveDataNode(QTreeWidgetItem* parent);
    void populatePlayerParamNode(QTreeWidgetItem* parent);
    void populateWeaponSpecsNode(QTreeWidgetItem* parent);

    bool filterItem(QTreeWidgetItem* item, const QString& filterText);

    // Helper to create items
    QTreeWidgetItem* createEditableItem(QTreeWidgetItem* parent, const QString& name, const QVariant& value, const QString& internalId);
    QTreeWidgetItem* createReadOnlyItem(QTreeWidgetItem* parent, const QString& name, const QVariant& value);

    QLineEdit* m_searchBox;
    QTreeWidget* m_treeWidget;
    QTimer* m_autoRefreshTimer;
    QTimer* m_stateUpdateTimer;

    bool m_isUpdatingTree = false;
};