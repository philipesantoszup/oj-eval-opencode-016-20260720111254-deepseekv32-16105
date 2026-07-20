#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <set>
#include <vector>
#include <algorithm>

using namespace std;

const string DB_FILE = "bptree.dat";

// Simple file-backed store
class FileStore {
private:
    unordered_map<string, set<int>> cache;
    
public:
    FileStore() {
        load();
    }
    
    ~FileStore() {
        save();
    }
    
    void insert(const string& key, int value) {
        cache[key].insert(value);
    }
    
    void remove(const string& key, int value) {
        auto it = cache.find(key);
        if (it != cache.end()) {
            it->second.erase(value);
            if (it->second.empty()) {
                cache.erase(it);
            }
        }
    }
    
    vector<int> find(const string& key) {
        auto it = cache.find(key);
        if (it == cache.end()) {
            return {};
        }
        return vector<int>(it->second.begin(), it->second.end());
    }
    
private:
    void load() {
        ifstream file(DB_FILE, ios::binary);
        if (!file) return;
        
        cache.clear();
        while (file.peek() != EOF) {
            size_t keyLen;
            file.read(reinterpret_cast<char*>(&keyLen), sizeof(size_t));
            
            string key(keyLen, '\0');
            file.read(&key[0], keyLen);
            
            size_t count;
            file.read(reinterpret_cast<char*>(&count), sizeof(size_t));
            
            set<int> values;
            for (size_t i = 0; i < count; i++) {
                int val;
                file.read(reinterpret_cast<char*>(&val), sizeof(int));
                values.insert(val);
            }
            
            cache[key] = values;
        }
    }
    
    void save() {
        ofstream file(DB_FILE, ios::binary | ios::trunc);
        if (!file) return;
        
        for (const auto& [key, values] : cache) {
            size_t keyLen = key.size();
            file.write(reinterpret_cast<const char*>(&keyLen), sizeof(size_t));
            file.write(key.c_str(), keyLen);
            
            size_t count = values.size();
            file.write(reinterpret_cast<const char*>(&count), sizeof(size_t));
            for (int val : values) {
                file.write(reinterpret_cast<const char*>(&val), sizeof(int));
            }
        }
    }
};

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);
    
    FileStore store;
    
    int n;
    cin >> n;
    
    for (int i = 0; i < n; i++) {
        string command;
        cin >> command;
        
        if (command == "insert") {
            string index;
            int value;
            cin >> index >> value;
            store.insert(index, value);
        }
        else if (command == "delete") {
            string index;
            int value;
            cin >> index >> value;
            store.remove(index, value);
        }
        else if (command == "find") {
            string index;
            cin >> index;
            vector<int> result = store.find(index);
            if (result.empty()) {
                cout << "null\n";
            } else {
                for (size_t j = 0; j < result.size(); j++) {
                    if (j > 0) cout << " ";
                    cout << result[j];
                }
                cout << "\n";
            }
        }
    }
    
    return 0;
}