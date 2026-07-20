#include <iostream>
#include <string>
#include <unordered_map>
#include <set>

using namespace std;

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);
    
    unordered_map<string, set<int>> database;
    
    int n;
    cin >> n;
    
    for (int i = 0; i < n; i++) {
        string command;
        cin >> command;
        
        if (command == "insert") {
            string index;
            int value;
            cin >> index >> value;
            database[index].insert(value);
        }
        else if (command == "delete") {
            string index;
            int value;
            cin >> index >> value;
            auto it = database.find(index);
            if (it != database.end()) {
                it->second.erase(value);
                if (it->second.empty()) {
                    database.erase(it);
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