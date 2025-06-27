#include <iostream>
#include <unordered_map>
#include <map>
#include <vector>
#include <chrono>
#include <fstream>
#include <sstream>

using namespace std;
using namespace chrono;

struct DNAInfo
{
    string species;
    string mutation;
};

vector<pair<string, DNAInfo>> load_csv(const string &filename)
{
    vector<pair<string, DNAInfo>> dataset;
    ifstream file(filename);
    string line;
    while (getline(file, line))
    {
        stringstream ss(line);
        string dna, species, mutation;
        if (getline(ss, dna, ',') &&
            getline(ss, species, ',') &&
            getline(ss, mutation, ','))
        {
            dataset.push_back({dna, {species, mutation}});
        }
    }
    return dataset;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        cout << "Usage: " << argv[0] << " <data.csv> <query>" << endl;
        return 1;
    }

    string filename = argv[1];
    string query = argv[2];

    auto dataset = load_csv(filename);
    size_t n = dataset.size();

    unordered_map<string, DNAInfo> hash_map;
    map<string, DNAInfo> bpt;

    auto start_insert_hash = high_resolution_clock::now();
    for (auto &entry : dataset)
    {
        hash_map[entry.first] = entry.second;
    }
    auto end_insert_hash = high_resolution_clock::now();

    auto start_insert_bpt = high_resolution_clock::now();
    for (auto &entry : dataset)
    {
        bpt[entry.first] = entry.second;
    }
    auto end_insert_bpt = high_resolution_clock::now();

    auto start_search_hash = high_resolution_clock::now();
    auto it_hash = hash_map.find(query);
    auto end_search_hash = high_resolution_clock::now();

    auto start_search_bpt = high_resolution_clock::now();
    vector<pair<string, DNAInfo>> bpt_results;
    for (auto it = bpt.lower_bound(query);
         it != bpt.end(); ++it)
    {
        if (it->first.compare(0, query.size(), query) != 0)
            break;
        bpt_results.push_back(*it);
    }
    auto end_search_bpt = high_resolution_clock::now();
    int count_prefix = bpt_results.size();

    size_t est_hash_mem = hash_map.size() * (sizeof(string) + sizeof(DNAInfo));
    size_t est_bpt_mem = bpt.size() * (sizeof(string) + sizeof(DNAInfo));

    cout << "==== Data Size: " << n << " ====" << endl;

    auto insert_hash_us = duration_cast<microseconds>(end_insert_hash - start_insert_hash).count();
    auto insert_bpt_us = duration_cast<microseconds>(end_insert_bpt - start_insert_bpt).count();
    cout << "Insert Time (Hash Map) : " << insert_hash_us << " µs" << endl;
    cout << "Insert Time (B+ Tree)  : " << insert_bpt_us << " µs" << endl
         << endl;

    auto search_hash_ns = duration_cast<nanoseconds>(end_search_hash - start_search_hash).count();
    auto search_bpt_ns = duration_cast<nanoseconds>(end_search_bpt - start_search_bpt).count();
    cout << "Search Time (Hash Map) : " << search_hash_ns << " ns" << endl;
    cout << "Search Time (B+ Tree)  : " << search_bpt_ns << " ns" << endl
         << endl;

    cout << "Search Result:" << endl;

    if (it_hash != hash_map.end())
    {
        cout << "- Hash Map  : 1 record found -> "
             << it_hash->first << ": "
             << it_hash->second.species << ", "
             << it_hash->second.mutation << endl;
    }
    else
    {
        cout << "- Hash Map  : 0 records found" << endl;
    }

    if (count_prefix > 0)
    {
        cout << "- B+ Tree   : " << count_prefix << " record(s) found:" << endl;
        for (auto &entry : bpt_results)
        {
            cout << "    • " << entry.first << ": "
                 << entry.second.species << ", "
                 << entry.second.mutation << endl;
        }
    }
    else
    {
        cout << "- B+ Tree   : 0 records found" << endl;
    }
    cout << endl;

    cout << "Estimated Memory Usage:" << endl;
    cout << "  - Hash Map : ~" << est_hash_mem << " bytes" << endl;
    cout << "  - B+ Tree  : ~" << est_bpt_mem << " bytes" << endl;

    return 0;
}
