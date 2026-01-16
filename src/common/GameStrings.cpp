#include "GameStrings.h"
#include <map>
#include <LunarTear++.h>

std::unordered_map<int, std::string> g_gameStringCache;

std::string GetGameString(int stringId) {
	auto it = g_gameStringCache.find(stringId);
	if (it != g_gameStringCache.end()) {
		return it->second;
	}
	std::string str = LunarTear::Get().Game().GetLocalizedString(stringId);
	g_gameStringCache[stringId] = str;
	return str;
}


QString GetGameQString(int stringId) {
	return QString::fromStdString(GetGameString(stringId));
}