#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <chrono>
#include <fstream>
#include <sstream>

using namespace std;
using Clock = chrono::high_resolution_clock;
using Micros = chrono::microseconds;

// Simple record struct
struct Record {
    string species;
    string mutation;
};


// HashMap wrapper
class HashMapDS {
public:
    void insertAll(const vector<pair<string, Record>>& data) {
        for (auto& kv : data)
            map_.emplace(kv.first, kv.second);
    }

    bool find(const string& key, Record& out, long& elapsed_us) const {
        auto start = Clock::now();
        auto it = map_.find(key);
        auto end = Clock::now();
        elapsed_us = chrono::duration_cast<Micros>(end - start).count();
        if (it != map_.end()) {
            out = it->second;
            return true;
        }
        return false;
    }

    bool create(const string& key, const Record& rec, long& elapsed_us) {
        auto start = Clock::now();
        auto p = map_.emplace(key, rec);
        auto end = Clock::now();
        elapsed_us = chrono::duration_cast<Micros>(end - start).count();
        return p.second;
    }

    bool remove(const string& key, long& elapsed_us) {
        auto start = Clock::now();
        auto cnt = map_.erase(key);
        auto end = Clock::now();
        elapsed_us = chrono::duration_cast<Micros>(end - start).count();
        return cnt > 0;
    }

    bool update(const string& key, const Record& rec, long& elapsed_us) {
        auto start = Clock::now();
        auto it = map_.find(key);
        if (it != map_.end()) it->second = rec;
        auto end = Clock::now();
        elapsed_us = chrono::duration_cast<Micros>(end - start).count();
        return it != map_.end();
    }

private:
    unordered_map<string, Record> map_;
};

// B+Tree wrapper using std::map
class BPlusTreeDS {
public:
    void insertAll(const vector<pair<string, Record>>& data) {
        for (auto& kv : data)
            tree_.emplace(kv.first, kv.second);
    }

    bool find(const string& key, Record& out, long& elapsed_us) const {
        auto start = Clock::now();
        auto it = tree_.find(key);
        auto end = Clock::now();
        elapsed_us = chrono::duration_cast<Micros>(end - start).count();
        if (it != tree_.end()) {
            out = it->second;
            return true;
        }
        return false;
    }

    bool findNearest(const string& key, string& nearestKey, Record& out, long& elapsed_us) const {
        auto start = Clock::now();
        if (tree_.empty()) {
            elapsed_us = 0;
            return false;
        }
        auto it = tree_.lower_bound(key);
        if (it == tree_.end()) it = prev(tree_.end());
        nearestKey = it->first;
        out = it->second;
        auto end = Clock::now();
        elapsed_us = chrono::duration_cast<Micros>(end - start).count();
        return true;
    }

    bool create(const string& key, const Record& rec, long& elapsed_us) {
        auto start = Clock::now();
        auto p = tree_.emplace(key, rec);
        auto end = Clock::now();
        elapsed_us = chrono::duration_cast<Micros>(end - start).count();
        return p.second;
    }

    bool remove(const string& key, long& elapsed_us) {
        auto start = Clock::now();
        auto cnt = tree_.erase(key);
        auto end = Clock::now();
        elapsed_us = chrono::duration_cast<Micros>(end - start).count();
        return cnt > 0;
    }

    bool update(const string& key, const Record& rec, long& elapsed_us) {
        auto start = Clock::now();
        auto it = tree_.find(key);
        if (it != tree_.end()) it->second = rec;
        auto end = Clock::now();
        elapsed_us = chrono::duration_cast<Micros>(end - start).count();
        return it != tree_.end();
    }

private:
    map<string, Record> tree_;
};

// Load CSV: each line "key,species,mutation"
bool loadCSV(const string& path, vector<pair<string, Record>>& out) {
    ifstream file(path);
    if (!file.is_open()) return false;
    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;
        if (line.rfind("key,", 0) == 0) continue; // skip header
        istringstream ss(line);
        string key, species, mutation;
        if (!getline(ss, key, ',')) continue;
        if (!getline(ss, species, ',')) continue;
        if (!getline(ss, mutation, ',')) continue;
        out.emplace_back(key, Record{species, mutation});
    }
    return true;
}

// Print a small benchmark table for initial insert
void printBenchmark(long hashTime, long bptTime) {
    const int w1 = 25, w2 = 12, w3 = 12;
    cout << "+" << string(w1, '-') << "+" << string(w2, '-') << "+" << string(w3, '-') << "+\n";
    cout << "| " << left << setw(w1 - 2) << "Operation"
         << "| " << right << setw(w2 - 2) << "HashMap (µs)"
         << " | " << setw(w3 - 2) << "B+Tree (µs)" << " |\n";
    cout << "+" << string(w1, '-') << "+" << string(w2, '-') << "+" << string(w3, '-') << "+\n";
    cout << "| " << left << setw(w1 - 2) << "Initial insert"
         << "| " << right << setw(w2 - 2) << hashTime
         << " | " << setw(w3 - 2) << bptTime << " |\n";
    cout << "+" << string(w1, '-') << "+" << string(w2, '-') << "+" << string(w3, '-') << "+\n";
}