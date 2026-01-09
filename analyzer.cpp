#include "analyzer.h"
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <cctype>

static bool parseRow(const std::string& line, std::string& zone, int& hour) {

    if (line.empty()) return false;



    int commaCount = 0;

    for (char c : line) {

        if (c == ',') commaCount++;

    }

    if (commaCount < 5) return false;

    size_t p1 = line.find(',');

    if (p1 == std::string::npos) return false;

    size_t p2 = line.find(',', p1 + 1);

    if (p2 == std::string::npos) return false;

    zone = line.substr(p1 + 1, p2 - (p1 + 1));

    if (zone == "PickupZoneID" || zone.empty()) return false;

    size_t p3 = line.find(',', p2 + 1);

    if (p3 == std::string::npos) return false;

    size_t p4 = line.find(',', p3 + 1);

    if (p4 == std::string::npos) return false;

    std::string dateStr = line.substr(p3 + 1, p4 - (p3 + 1));



    if (dateStr.size() < 13) return false;

    if (dateStr[10] != ' ') return false;

    size_t spacePos = dateStr.find(' ');

    if (spacePos == std::string::npos || spacePos + 1 >= dateStr.size()) {

        return false;

    }

    try {

        std::string hourStr = dateStr.substr(spacePos + 1, 2);



        if (!isdigit(hourStr[0])) return false;

        hour = std::stoi(hourStr);

        if (hour < 0 || hour > 23) return false;

    } catch (...) {

        return false;

    }

    return true;

}

void TripAnalyzer::ingestFile(const std::string& csvPath) {

    std::ifstream file(csvPath);

    if (!file.is_open()) return;

    std::string line;

    std::string zone;

    int hour = 0;



    if (!std::getline(file, line)) return;

    while (std::getline(file, line)) {

        if (parseRow(line, zone, hour)) {

            m_zoneCounts[zone]++;

            if (m_hourlyCounts[zone].empty()) {

                m_hourlyCounts[zone].resize(24, 0);

            }

            m_hourlyCounts[zone][hour]++;

        }

    }

}

std::vector<ZoneCount> TripAnalyzer::topZones(int k) const {

    std::vector<ZoneCount> results;

    for (const auto& pair : m_zoneCounts) {

        results.push_back({pair.first, pair.second});

    }

    std::sort(results.begin(), results.end(),

        [](const ZoneCount& a, const ZoneCount& b) {

            if (a.count != b.count) return a.count > b.count;

            return a.zone < b.zone;

        });

    if ((int)results.size() > k) {

        results.resize(k);

    }

    return results;

}

std::vector<SlotCount> TripAnalyzer::topBusySlots(int k) const {

    std::vector<SlotCount> results;

    for (const auto& pair : m_hourlyCounts) {

        const std::string& currentZone = pair.first;

        const std::vector<long long>& hours = pair.second;

        for (int h = 0; h < 24; ++h) {

            if (hours[h] > 0) {

                results.push_back({currentZone, h, hours[h]});

            }

        }

    }

    std::sort(results.begin(), results.end(),

        [](const SlotCount& a, const SlotCount& b) {

            if (a.count != b.count) return a.count > b.count;

            if (a.zone != b.zone) return a.zone < b.zone;

            return a.hour < b.hour;

        });

    if ((int)results.size() > k) {

        results.resize(k);

    }

    return results;

}
 
