#include "analyzer.h"

#include <fstream>
#include <algorithm>
#include <cctype>

using namespace std;

static bool parseRow(const string& line, string& zone, int& hour) {
    if (line.empty()) return false;

    int commaCount = 0;
    for (char c : line)
        if (c == ',') commaCount++;
    if (commaCount < 5) return false;

    size_t p1 = line.find(',');
    size_t p2 = line.find(',', p1 + 1);
    size_t p3 = line.find(',', p2 + 1);
    size_t p4 = line.find(',', p3 + 1);

    if (p1 == string::npos || p2 == string::npos ||
        p3 == string::npos || p4 == string::npos)
        return false;

    zone = line.substr(p1 + 1, p2 - p1 - 1);
    if (zone.empty() || zone == "PickupZoneID") return false;

    string dateStr = line.substr(p3 + 1, p4 - p3 - 1);
    if (dateStr.size() < 13 || dateStr[10] != ' ') return false;

    try {
        hour = stoi(dateStr.substr(11, 2));
        if (hour < 0 || hour > 23) return false;
    } catch (...) {
        return false;
    }

    return true;
}


void TripAnalyzer::ingest() {
    ifstream file("SmallTrips.csv");
    if (!file.is_open()) return;

    string line, zone;
    int hour = 0;

    if (!getline(file, line)) return; 

    while (getline(file, line)) {
        if (parseRow(line, zone, hour)) {
            m_zoneCounts[zone]++;
            if (m_hourlyCounts[zone].empty())
                m_hourlyCounts[zone].resize(24, 0);
            m_hourlyCounts[zone][hour]++;
        }
    }
}


vector<ZoneCount> TripAnalyzer::topZones(int k) const {
    vector<ZoneCount> res;
    for (const auto& p : m_zoneCounts)
        res.push_back({p.first, p.second});

    sort(res.begin(), res.end(),
        [](const ZoneCount& a, const ZoneCount& b) {
            if (a.count != b.count) return a.count > b.count;
            return a.zone < b.zone;
        });

    if ((int)res.size() > k) res.resize(k);
    return res;
}

vector<SlotCount> TripAnalyzer::topBusySlots(int k) const {
    vector<SlotCount> res;

    for (const auto& p : m_hourlyCounts)
        for (int h = 0; h < 24; h++)
            if (p.second[h] > 0)
                res.push_back({p.first, h, p.second[h]});

    sort(res.begin(), res.end(),
        [](const SlotCount& a, const SlotCount& b) {
            if (a.count != b.count) return a.count > b.count;
            if (a.zone != b.zone) return a.zone < b.zone;
            return a.hour < b.hour;
        });

    if ((int)res.size() > k) res.resize(k);
    return res;
}
