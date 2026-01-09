#include "analyzer.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cctype>


static bool parseRow(const std::string& line, std::string& zone, int& hour) {
    if (line.empty()) return false;

    int commaCount = 0;
    for (char c : line) if (c == ',') commaCount++;
    if (commaCount < 5) return false;

    std::size_t p1 = line.find(',');
    std::size_t p2 = line.find(',', p1 + 1);
    if (p1 == std::string::npos || p2 == std::string::npos) return false;

    zone = line.substr(p1 + 1, p2 - (p1 + 1));
    if (zone == "PickupZoneID" || zone.empty()) return false;

    std::size_t p3 = line.find(',', p2 + 1);
    std::size_t p4 = line.find(',', p3 + 1);
    if (p3 == std::string::npos || p4 == std::string::npos) return false;

    std::string dateStr = line.substr(p3 + 1, p4 - (p3 + 1));
    if (dateStr.size() < 13 || dateStr[10] != ' ') return false;

    try {
        std::string hourStr = dateStr.substr(11, 2);
        hour = std::stoi(hourStr);
        if (hour < 0 || hour > 23) return false;
    } catch (...) {
        return false;
    }
    return true;
}

void TripAnalyzer::ingestFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return;

    std::string line, zone;
    int hour = 0;
    if (!std::getline(file, line)) return; 

    while (std::getline(file, line)) {
        if (parseRow(line, zone, hour)) {
            m_zoneCounts[zone]++;
            if (m_hourlyCounts[zone].empty())
                m_hourlyCounts[zone].resize(24, 0);
            m_hourlyCounts[zone][hour]++;
        }
    }
}

std::vector<ZoneCount> TripAnalyzer::topZones(int k) const {
    std::vector<ZoneCount> results;
    for (const auto& p : m_zoneCounts)
        results.push_back({p.first, p.second});

    std::sort(results.begin(), results.end(), [](const ZoneCount& a, const ZoneCount& b) {
        if (a.count != b.count) return a.count > b.count;
        return a.zone < b.zone;
    });

    if ((int)results.size() > k) results.resize(k);
    return results;
}

std::vector<SlotCount> TripAnalyzer::topBusySlots(int k) const {
    std::vector<SlotCount> results;
    for (const auto& p : m_hourlyCounts) {
        for (int h = 0; h < 24; h++) {
            if (p.second[h] > 0)
                results.push_back({p.first, h, p.second[h]});
        }
    }

    std::sort(results.begin(), results.end(), [](const SlotCount& a, const SlotCount& b) {
        if (a.count != b.count) return a.count > b.count;
        if (a.zone != b.zone) return a.zone < b.zone;
        return a.hour < b.hour;
    });

    if ((int)results.size() > k) results.resize(k);
    return results;
}
