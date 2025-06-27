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

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <data.csv>\n";
        return 1;
    }

    // 1) Load data from CSV
    vector<pair<string, Record>> data;
    if (!loadCSV(argv[1], data)) {
        cerr << "Failed to open or parse CSV: " << argv[1] << "\n";
        return 1;
    }

    // 2) Build structures & benchmark insert
    HashMapDS hm;
    BPlusTreeDS bpt;
    auto t0 = Clock::now();
    hm.insertAll(data);
    auto t1 = Clock::now();
    bpt.insertAll(data);
    auto t2 = Clock::now();

    long t_hm_init = chrono::duration_cast<Micros>(t1 - t0).count();
    long t_bpt_init = chrono::duration_cast<Micros>(t2 - t1).count();
    printBenchmark(t_hm_init, t_bpt_init);

    cout << "\nType commands: find/create/read/update/delete/exit\n";

    string line;
    while (true) {
        cout << "> ";
        if (!getline(cin, line) || line.empty()) continue;

        istringstream iss(line);
        string cmd;
        iss >> cmd;

        if (cmd == "exit") break;

        // common variables
        string key, species, mutation;
        Record recLocal, recHm, recBpt;
        long t_hm, t_bpt;

        if (cmd == "find" || cmd == "read") {
            iss >> key;
            if (key.empty()) {
                cout << "Usage: " << cmd << " <key>\n";
                continue;
            }
            // HashMap lookup
            bool okHm = hm.find(key, recHm, t_hm);
            if (okHm) {
                cout << "HashMap:   ✓ Found \"" << key << "\" (" << t_hm << "µs) -> "
                     << recHm.species << ", " << recHm.mutation << "\n";
            } else {
                cout << "HashMap:   ✗ \"" << key << "\" not found (" << t_hm << "µs)\n";
            }
            // B+Tree lookup
            bool okBpt = bpt.find(key, recBpt, t_bpt);
            if (okBpt) {
                cout << "B+Tree:    ✓ Found \"" << key << "\" (" << t_bpt << "µs) -> "
                     << recBpt.species << ", " << recBpt.mutation << "\n";
            } else {
                string nk; Record nr; long t_near;
                bpt.findNearest(key, nk, nr, t_near);
                cout << "B+Tree:    ✗ \"" << key << "\" not found (" << t_bpt << "µs)"
                     << "  Nearest: [" << nk << "] -> "
                     << nr.species << ", " << nr.mutation << "\n";
            }
        }
        else if (cmd == "create") {
            iss >> key >> species >> mutation;
            if (mutation.empty()) {
                cout << "Usage: create <key> <species> <mutation>\n";
                continue;
            }
            recLocal = {species, mutation};
            long t_h, t_b;
            hm.create(key, recLocal, t_h);
            bpt.create(key, recLocal, t_b);
            cout << "Created \"" << key << "\" in HashMap(" << t_h
                 << "µs) & B+Tree(" << t_b << "µs)\n";
        }
        else if (cmd == "update") {
            iss >> key >> species >> mutation;
            if (mutation.empty()) {
                cout << "Usage: update <key> <new_species> <new_mutation>\n";
                continue;
            }
            recLocal = {species, mutation};
            bool uh = hm.update(key, recLocal, t_hm);
            bool ub = bpt.update(key, recLocal, t_bpt);
            if (uh && ub) {
                cout << "Updated \"" << key << "\" in HashMap(" << t_hm
                     << "µs) & B+Tree(" << t_bpt << "µs)\n";
            } else {
                cout << "✗ \"" << key << "\" not found, update failed\n";
            }
        }
        else if (cmd == "delete") {
            iss >> key;
            if (key.empty()) {
                cout << "Usage: delete <key>\n";
                continue;
            }
            bool dh = hm.remove(key, t_hm);
            bool db = bpt.remove(key, t_bpt);
            if (dh && db) {
                cout << "Deleted \"" << key << "\" in HashMap(" << t_hm
                     << "µs) & B+Tree(" << t_bpt << "µs)\n";
            } else {
                cout << "✗ \"" << key << "\" not found, delete failed\n";
            }
        }
        else {
            cout << "Unknown command: " << cmd << "\n";
        }
    }

    cout << "Goodbye!\n";
    return 0;
}
