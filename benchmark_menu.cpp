#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <map>
#include <chrono>

using namespace std;
using namespace chrono;

struct DNAInfo {
    string species;
    string mutation;
};

vector<pair<string, DNAInfo>> load_csv(const string &filename);
void findEntry(const unordered_map<string, DNAInfo>& um, const map<string, DNAInfo>& mp);
void createEntry(unordered_map<string, DNAInfo>& um, map<string, DNAInfo>& mp);
void readEntry(const unordered_map<string, DNAInfo>& um, const map<string, DNAInfo>& mp);
void updateEntry(unordered_map<string, DNAInfo>& um, map<string, DNAInfo>& mp);
void deleteEntry(unordered_map<string, DNAInfo>& um, map<string, DNAInfo>& mp);

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <data.csv>" << endl;
        return 1;
    }

    auto data = load_csv(argv[1]);
    if (data.empty()) {
        cerr << "No data loaded from CSV." << endl;
        return 1;
    }

    unordered_map<string, DNAInfo> um;
    map<string, DNAInfo> mp;

    auto t0 = high_resolution_clock::now();
    for (auto& e : data) um[e.first] = e.second;
    auto t1 = high_resolution_clock::now();
    for (auto& e : data) mp[e.first] = e.second;
    auto t2 = high_resolution_clock::now();

    cout << "Initial insertion time (µs): HashMap="
         << duration_cast<microseconds>(t1 - t0).count()
         << ", B+Tree=" << duration_cast<microseconds>(t2 - t1).count() << "\n\n";

    string choice;
    while (true) {
        cout << "(find create read upd del exit) \n->";
        if (!(cin >> choice)) break;
            if(choice == "find") findEntry(um, mp); 
            else if(choice == "create") createEntry(um, mp); 
            else if(choice == "read") readEntry(um, mp); 
            else if(choice == "upd") updateEntry(um, mp); 
            else if(choice == "del") deleteEntry(um, mp); 
            else{ 
                cout << "Exiting...\n"; 
                return 0;
            }
        cout << "\n";
    }
    return 0;
}


vector<pair<string, DNAInfo>> load_csv(const string &filename) {
    vector<pair<string, DNAInfo>> dataset;
    ifstream file(filename);
    if (!file) {
        cerr << "Error opening file: " << filename << endl;
        return dataset;
    }
    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        string dna, species, mutation;
        if (getline(ss, dna, ',') && getline(ss, species, ',') && getline(ss, mutation, ',')) {
            dataset.emplace_back(dna, DNAInfo{species, mutation});
        }
    }
    return dataset;
}

void findEntry(const unordered_map<string, DNAInfo>& um, const map<string, DNAInfo>& mp) {
    string key;
    cout << "Enter key to find: ";
    cin >> key;

    auto t0 = high_resolution_clock::now();
    auto it_um = um.find(key);
    auto t1 = high_resolution_clock::now();
    auto it_mp = mp.find(key);
    auto t2 = high_resolution_clock::now();

    long long t_um = duration_cast<microseconds>(t1 - t0).count();
    long long t_mp = duration_cast<microseconds>(t2 - t1).count();

    // HashMap result
    if (it_um != um.end()) {
        cout<<"tes";
        cout << "HashMap found: [" << it_um->first << "] -> ["
             << it_um->second.species << ", " << it_um->second.mutation << "]"
             << " (" << t_um << " µs)\n";
    } else {
        cout << "HashMap not found" << " (" << t_um << " µs)\n";
    }

    // B+ Tree result or nearest match
    if (it_mp != mp.end()) {
        cout << "\nB+ Tree found: [" << it_mp->first << "] -> ["
             << it_mp->second.species << ", " << it_mp->second.mutation << "]"
             << " (" << t_mp << " µs)\n";
    } else {
        auto it_lb = mp.lower_bound(key);
        if (it_lb == mp.end()) {
            it_lb = prev(mp.end());
        }
        cout << "\nB+ Tree no exact match. Nearest: [" << it_lb->first << "] -> ["
             << it_lb->second.species << ", " << it_lb->second.mutation << "]"
             << " (" << t_mp << " µs)\n\r";
    }
}

void createEntry(unordered_map<string, DNAInfo>& um, map<string, DNAInfo>& mp) {
    string key, species, mutation;
    cout << "Enter key, species, mutation: ";
    cin >> key >> species >> mutation;
    auto t0 = high_resolution_clock::now();
    um[key] = {species, mutation};
    auto t1 = high_resolution_clock::now();
    mp[key] = {species, mutation};
    auto t2 = high_resolution_clock::now();

    cout << "Create HashMap: " << duration_cast<microseconds>(t1 - t0).count() << " µs\n";
    cout << "Create B+ Tree: " << duration_cast<microseconds>(t2 - t1).count() << " µs\n";
}

void readEntry(const unordered_map<string, DNAInfo>& um, const map<string, DNAInfo>& mp) {
    string key;
    cout << "Enter key to read: ";
    cin >> key;
    auto t0 = high_resolution_clock::now();
    auto it_um = um.find(key);
    auto t1 = high_resolution_clock::now();
    auto it_mp = mp.find(key);
    auto t2 = high_resolution_clock::now();

    cout << "HashMap read: " << (it_um != um.end() ? "OK" : "Not found")
         << " (" << duration_cast<microseconds>(t1 - t0).count() << " µs)";
    if (it_um != um.end())
        cout << " -> [" << it_um->second.species << ", " << it_um->second.mutation << "]";
    cout << "\n";

    cout << "B+ Tree read: " << (it_mp != mp.end() ? "OK" : "Not found")
         << " (" << duration_cast<microseconds>(t2 - t1).count() << " µs)";
    if (it_mp != mp.end())
        cout << " -> [" << it_mp->second.species << ", " << it_mp->second.mutation << "]";
    cout << "\n";
}

void updateEntry(unordered_map<string, DNAInfo>& um, map<string, DNAInfo>& mp) {
    string key, species, mutation;
    cout << "Enter key, new species, new mutation: ";
    cin >> key >> species >> mutation;
    auto t0 = high_resolution_clock::now();
    if (um.find(key) != um.end()) um[key] = {species, mutation};
    auto t1 = high_resolution_clock::now();
    if (mp.find(key) != mp.end()) mp[key] = {species, mutation};
    auto t2 = high_resolution_clock::now();

    cout << "Update HashMap: " << duration_cast<microseconds>(t1 - t0).count() << " µs\n";
    cout << "Update B+ Tree: " << duration_cast<microseconds>(t2 - t1).count() << " µs\n";
}

void deleteEntry(unordered_map<string, DNAInfo>& um, map<string, DNAInfo>& mp) {
    string key;
    cout << "Enter key to delete: ";
    cin >> key;
    auto t0 = high_resolution_clock::now();
    um.erase(key);
    auto t1 = high_resolution_clock::now();
    mp.erase(key);
    auto t2 = high_resolution_clock::now();

    cout << "Delete HashMap: " << duration_cast<microseconds>(t1 - t0).count() << " µs\n";
    cout << "Delete B+ Tree: " << duration_cast<microseconds>(t2 - t1).count() << " µs\n";
}
