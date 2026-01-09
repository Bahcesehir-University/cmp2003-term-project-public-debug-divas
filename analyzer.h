#pragma once

#include <string>
#include <vector>
#include <unordered_map>

struct ZoneCount {
    std::string zone;
    long long count;
};

struct SlotCount {
    std::string zone;
    int hour;
    long long count;
};

class TripAnalyzer {
public:
    void ingest(std::istream& in);
    std::vector<ZoneCount> topZones(int k = 10) const;
    std::vector<SlotCount> topBusySlots(int k = 10) const;

private:
    std::unordered_map<std::string, long long> m_zoneCounts;
    std::unordered_map<std::string, std::vector<long long>> m_hourlyCounts;
};
