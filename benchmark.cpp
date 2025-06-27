#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <map>
#include <chrono>
#include <algorithm>
#include <iomanip>
#include <numeric>

using namespace std;
using namespace chrono;

struct DNAInfo {
    string species;
    string mutation;
};

// ... (fungsi load_csv dan findSingle tetap sama) ...
vector<pair<string, DNAInfo>> load_csv(const string &filename) {
    vector<pair<string, DNAInfo>> dataset;
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: cannot open file " << filename << endl;
        return dataset;
    }
    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        string dna, species, mutation;
        if (getline(ss, dna, ',') && getline(ss, species, ',') && getline(ss, mutation)) {
            mutation.erase(mutation.find_last_not_of(" \n\r\t")+1);
            dataset.emplace_back(dna, DNAInfo{species, mutation});
        }
    }
    return dataset;
}

void findSingle(const unordered_map<string, DNAInfo>& um,
                const map<string, DNAInfo>& mp,
                const string& key) {
    
    cout << "\n=== Find Result for key: '" << key << "' ===\n";
    cout << left << setw(12) << "Structure" << " | "
         << setw(20) << "Key" << " | "
         << setw(15) << "Species" << " | "
         << setw(15) << "Mutation" << " | "
         << "Time (ns)" << "\n";
    cout << string(12, '-') << "-|-" << string(20, '-') << "-|-"
         << string(15, '-') << "-|-" << string(15, '-') << "-|-"
         << string(10, '-') << "\n";

    auto t0 = high_resolution_clock::now();
    auto it_um = um.find(key);
    auto t1 = high_resolution_clock::now();
    long long t_um = duration_cast<nanoseconds>(t1 - t0).count();

    if (it_um != um.end()) {
        cout << left << setw(12) << "HashMap" << " | "
             << setw(20) << it_um->first << " | "
             << setw(15) << it_um->second.species << " | "
             << setw(15) << it_um->second.mutation << " | "
             << t_um << "\n";
    } else {
        cout << left << setw(12) << "HashMap" << " | "
             << setw(20) << "Not Found" << " | "
             << setw(15) << "-" << " | "
             << setw(15) << "-" << " | "
             << t_um << "\n";
    }

    auto t2 = high_resolution_clock::now();
    auto it_mp = mp.find(key);
    auto t3 = high_resolution_clock::now();
    long long t_mp = duration_cast<nanoseconds>(t3 - t2).count();
    
    if (it_mp != mp.end()) {
        cout << left << setw(12) << "B+ Tree" << " | "
             << setw(20) << it_mp->first << " | "
             << setw(15) << it_mp->second.species << " | "
             << setw(15) << it_mp->second.mutation << " | "
             << t_mp << "\n";
    } else {
        cout << left << setw(12) << "B+ Tree" << " | "
             << setw(20) << "Not Found" << " | "
             << setw(15) << "-" << " | "
             << setw(15) << "-" << " | "
             << t_mp << "\n";
        
        cout << string(85, '-') << "\n";
        cout << "B+ Tree can find nearby elements:\n";

        auto lb = mp.lower_bound(key);
        if (lb != mp.end()) {
            cout << left << setw(12) << "Lower Bound" << " | "
                 << setw(20) << lb->first << " | "
                 << setw(15) << lb->second.species << " | "
                 << setw(15) << lb->second.mutation << " | \n";
        } else {
            cout << left << setw(12) << "Lower Bound" << " | " << "Not Found (key is > all elements)\n";
        }

        auto ub = mp.upper_bound(key);
        if (ub != mp.end()) {
            cout << left << setw(12) << "Upper Bound" << " | "
                 << setw(20) << ub->first << " | "
                 << setw(15) << ub->second.species << " | "
                 << setw(15) << ub->second.mutation << " | \n";
        } else {
             cout << left << setw(12) << "Upper Bound" << " | " << "Not Found (key is >= all elements)\n";
        }
    }
    cout << "\n";
}


int main(int argc, char *argv[]) {
    if (argc < 2 || argc > 3) {
        cerr << "Usage: " << argv[0] << " <data.csv> [key_to_find]\n";
        return 1;
    }

    string filename = argv[1];
    auto full_data = load_csv(filename);
    if (full_data.empty()) {
        cerr << "No data loaded from " << filename << " or file is empty.\n";
        return 1;
    }
    
    const int n = full_data.size();
    const int num_runs = 100;
    cout << "=== Dataset Size: " << n << ", Benchmark Runs: " << num_runs << " ===\n";

    if (argc == 3) {
        string key_to_find = argv[2];
        unordered_map<string, DNAInfo> um_single(full_data.begin(), full_data.end());
        map<string, DNAInfo> mp_single(full_data.begin(), full_data.end());
        findSingle(um_single, mp_single, key_to_find);
    }
    
    unordered_map<string, DNAInfo> um;
    map<string, DNAInfo> mp;
    long long total_time;

    // -- 1. BENCHMARK CREATE (INSERT) --
    total_time = 0;
    for(int i = 0; i < num_runs; ++i) {
        um.clear();
        auto start = high_resolution_clock::now();
        for (const auto &e : full_data) um.insert(e);
        total_time += duration_cast<nanoseconds>(high_resolution_clock::now() - start).count();
    }
    double create_um = static_cast<double>(total_time) / num_runs;

    total_time = 0;
    for(int i = 0; i < num_runs; ++i) {
        mp.clear();
        auto start = high_resolution_clock::now();
        for (const auto &e : full_data) mp.insert(e);
        total_time += duration_cast<nanoseconds>(high_resolution_clock::now() - start).count();
    }
    double create_mp = static_cast<double>(total_time) / num_runs;


    // Isi map sekali untuk benchmark Find, Update, Delete
    um.clear(); mp.clear();
    for (const auto &e : full_data) {
        um.insert(e);
        mp.insert(e);
    }

    // -- 2. BENCHMARK FIND (LOOKUP ONLY) --
    total_time = 0;
    for(int i = 0; i < num_runs; ++i) {
        volatile size_t find_count_um = 0;
        auto start = high_resolution_clock::now();
        for (const auto &e : full_data) {
            if (um.count(e.first)) {
                find_count_um++;
            }
        }
        total_time += duration_cast<nanoseconds>(high_resolution_clock::now() - start).count();
    }
    double find_um = static_cast<double>(total_time) / num_runs;

    total_time = 0;
    for(int i = 0; i < num_runs; ++i) {
        volatile size_t find_count_mp = 0;
        auto start = high_resolution_clock::now();
        for (const auto &e : full_data) {
            if (mp.count(e.first)) {
                find_count_mp++;
            }
        }
        total_time += duration_cast<nanoseconds>(high_resolution_clock::now() - start).count();
    }
    double find_mp = static_cast<double>(total_time) / num_runs;

    // -- 3. BENCHMARK UPDATE --
    auto start_update = high_resolution_clock::now();
    for (auto &e : um) { e.second.species += "_upd"; }
    double update_um = duration_cast<nanoseconds>(high_resolution_clock::now() - start_update).count();

    start_update = high_resolution_clock::now();
    for (auto &e : mp) { e.second.species += "_upd"; }
    double update_mp = duration_cast<nanoseconds>(high_resolution_clock::now() - start_update).count();
    
    // -- 4. BENCHMARK DELETE --
    vector<string> keys_to_delete;
    keys_to_delete.reserve(n);
    for(const auto& e : full_data) { keys_to_delete.push_back(e.first); }

    auto start_delete = high_resolution_clock::now();
    for (const auto &key : keys_to_delete) um.erase(key);
    double delete_um = duration_cast<nanoseconds>(high_resolution_clock::now() - start_delete).count();

    // Map untuk delete perlu diisi ulang karena sudah di-update
    mp.clear();
    for (const auto &e : full_data) mp.insert(e);
    start_delete = high_resolution_clock::now();
    for (const auto &key : keys_to_delete) mp.erase(key);
    double delete_mp = duration_cast<nanoseconds>(high_resolution_clock::now() - start_delete).count();

    // -- Tampilan Hasil Benchmark --
    cout << "\n=== Full Dataset Benchmark Results ===\n";
    cout << "Avg. time per element (ns)\n";
    cout << left << setw(15) << "Operation"
         << "| " << right << setw(18) << "HashMap (ns)"
         << " | " << setw(18) << "B+ Tree (ns)" << "\n";
    cout << string(15, '-') << "+--------------------+------------------\n";
    cout << fixed << setprecision(2);
    cout << left << setw(15) << "Create"
         << "| " << right << setw(18) << create_um / n
         << " | " << setw(18) << create_mp / n << "\n";
    cout << left << setw(15) << "Find"
         << "| " << right << setw(18) << find_um / n
         << " | " << setw(18) << find_mp / n << "\n";
    cout << left << setw(15) << "Update"
         << "| " << right << setw(18) << update_um / n
         << " | " << setw(18) << update_mp / n << "\n";
    cout << left << setw(15) << "Delete"
         << "| " << right << setw(18) << delete_um / n
         << " | " << setw(18) << delete_mp / n << "\n\n";
    
    return 0;
}