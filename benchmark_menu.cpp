#include "benchmark_menu.hpp"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <data.csv>\n";
        return 1;
    }

    vector<pair<string, Record>> data;
    if (!loadCSV(argv[1], data)) {
        cerr << "Failed to open or parse CSV: " << argv[1] << "\n";
        return 1;
    }

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
        cout << endl;

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

    cout << "Goodbye Love!\n";
    return 0;
}
