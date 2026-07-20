#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstring>
#include <unordered_map>

using namespace std;

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);
    
    unordered_map<string, vector<int>> database;
    
    int n;
    cin >> n;
    
    for (int i = 0; i < n; i++) {
        string command;
        cin >> command;
        
        if (command == "insert") {
            string index;
            int value;
            cin >> index >> value;
            auto& vec = database[index];
            // Insert maintaining sorted order using binary search
            auto it = lower_bound(vec.begin(), vec.end(), value);
            if (it == vec.end() || *it != value) {
                vec.insert(it, value);
            }
        }
        else if (command == "delete") {
            string index;
            int value;
            cin >> index >> value;
            auto it = database.find(index);
            if (it != database.end()) {
                auto& vec = it->second;
                auto val_it = lower_bound(vec.begin(), vec.end(), value);
                if (val_it != vec.end() && *val_it == value) {
                    vec.erase(val_it);
                    if (vec.empty()) {
                        database.erase(it);
                    }
                }
            }
        }
        else if (command == "find") {
            string index;
            cin >> index;
            auto it = database.find(index);
            if (it == database.end() || it->second.empty()) {
                cout << "null\n";
            } else {
                bool first = true;
                for (int val : it->second) {
                    if (!first) cout << " ";
                    cout << val;
                    first = false;
                }
                cout << "\n";
            }
        }
    }
    
    return 0;
}