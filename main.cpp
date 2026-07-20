#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <vector>
#include <set>
#include <map>
#include <unordered_map>
#include <algorithm>

using namespace std;

// File-based B+ Tree implementation
class BPlusTree {
private:
    struct Node {
        bool isLeaf;
        vector<string> keys;
        vector<int> values; // For leaf nodes
        vector<int> children; // For internal nodes (file offsets)
        int nextLeaf; // For leaf nodes
        int count;
        
        Node(bool leaf = false) : isLeaf(leaf), nextLeaf(-1), count(0) {}
    };
    
    const string FILENAME = "bptree.dat";
    fstream file;
    int rootOffset;
    int order;
    
    void writeNode(int offset, const Node& node) {
        file.seekp(offset);
        file.write(reinterpret_cast<const char*>(&node.isLeaf), sizeof(bool));
        file.write(reinterpret_cast<const char*>(&node.count), sizeof(int));
        
        for (int i = 0; i < node.count; i++) {
            int keyLen = node.keys[i].size();
            file.write(reinterpret_cast<const char*>(&keyLen), sizeof(int));
            file.write(node.keys[i].c_str(), keyLen);
        }
        
        if (node.isLeaf) {
            file.write(reinterpret_cast<const char*>(&node.nextLeaf), sizeof(int));
            for (int i = 0; i < node.count; i++) {
                file.write(reinterpret_cast<const char*>(&node.values[i]), sizeof(int));
            }
        } else {
            for (int i = 0; i <= node.count; i++) {
                file.write(reinterpret_cast<const char*>(&node.children[i]), sizeof(int));
            }
        }
    }
    
    void readNode(int offset, Node& node) {
        file.seekg(offset);
        file.read(reinterpret_cast<char*>(&node.isLeaf), sizeof(bool));
        file.read(reinterpret_cast<char*>(&node.count), sizeof(int));
        
        node.keys.resize(node.count);
        for (int i = 0; i < node.count; i++) {
            int keyLen;
            file.read(reinterpret_cast<char*>(&keyLen), sizeof(int));
            char* buffer = new char[keyLen + 1];
            file.read(buffer, keyLen);
            buffer[keyLen] = '\0';
            node.keys[i] = string(buffer);
            delete[] buffer;
        }
        
        if (node.isLeaf) {
            file.read(reinterpret_cast<char*>(&node.nextLeaf), sizeof(int));
            node.values.resize(node.count);
            for (int i = 0; i < node.count; i++) {
                file.read(reinterpret_cast<char*>(&node.values[i]), sizeof(int));
            }
            node.children.clear();
        } else {
            node.children.resize(node.count + 1);
            for (int i = 0; i <= node.count; i++) {
                file.read(reinterpret_cast<char*>(&node.children[i]), sizeof(int));
            }
            node.values.clear();
        }
    }
    
    int allocateNode(const Node& node) {
        file.seekp(0, ios::end);
        int offset = file.tellp();
        writeNode(offset, node);
        return offset;
    }
    
public:
    BPlusTree(int ord = 100) : order(ord), rootOffset(-1) {
        // Check if file exists
        ifstream test(FILENAME);
        if (test.good()) {
            test.close();
            file.open(FILENAME, ios::in | ios::out | ios::binary);
            file.read(reinterpret_cast<char*>(&rootOffset), sizeof(int));
        } else {
            file.open(FILENAME, ios::in | ios::out | ios::binary | ios::trunc);
            rootOffset = -1;
            file.write(reinterpret_cast<const char*>(&rootOffset), sizeof(int));
        }
    }
    
    ~BPlusTree() {
        if (file.is_open()) {
            file.seekp(0);
            file.write(reinterpret_cast<const char*>(&rootOffset), sizeof(int));
            file.close();
        }
    }
    
    void insert(const string& key, int value) {
        if (rootOffset == -1) {
            Node leaf(true);
            leaf.keys.push_back(key);
            leaf.values.push_back(value);
            leaf.count = 1;
            rootOffset = allocateNode(leaf);
            return;
        }
        
        // Find leaf node
        Node node;
        vector<int> path;
        int current = rootOffset;
        
        while (true) {
            path.push_back(current);
            readNode(current, node);
            if (node.isLeaf) break;
            
            int pos = 0;
            while (pos < node.count && key > node.keys[pos]) pos++;
            current = node.children[pos];
        }
        
        // Insert into leaf
        int pos = 0;
        while (pos < node.count && key > node.keys[pos]) pos++;
        
        // Check if value already exists for this key
        if (pos < node.count && key == node.keys[pos]) {
            // Key exists, check if value already present
            auto it = find(node.values.begin(), node.values.end(), value);
            if (it != node.values.end()) return; // Value already exists
            
            // Insert value maintaining sorted order
            node.values.insert(node.values.begin() + pos, value);
            node.count++;
        } else {
            // Insert new key-value pair
            node.keys.insert(node.keys.begin() + pos, key);
            node.values.insert(node.values.begin() + pos, value);
            node.count++;
        }
        
        writeNode(current, node);
        
        // Split if needed
        if (node.count > order) {
            splitLeaf(path, node, current);
        }
    }
    
    void splitLeaf(const vector<int>& path, Node& node, int nodeOffset) {
        // Create new leaf
        Node newLeaf(true);
        int mid = node.count / 2;
        
        // Move后半部分 to new leaf
        for (int i = mid; i < node.count; i++) {
            newLeaf.keys.push_back(node.keys[i]);
            newLeaf.values.push_back(node.values[i]);
        }
        
        newLeaf.count = node.count - mid;
        node.count = mid;
        
        // Update links
        newLeaf.nextLeaf = node.nextLeaf;
        node.nextLeaf = allocateNode(newLeaf);
        
        // Update parent
        writeNode(nodeOffset, node);
        
        // Propagate split
        string splitKey = newLeaf.keys[0];
        insertIntoParent(path, splitKey, nodeOffset, node.nextLeaf);
    }
    
    void insertIntoParent(const vector<int>& path, const string& key, int leftChild, int rightChild) {
        if (path.size() == 1) {
            // Create new root
            Node newRoot(false);
            newRoot.keys.push_back(key);
            newRoot.children.push_back(leftChild);
            newRoot.children.push_back(rightChild);
            newRoot.count = 1;
            rootOffset = allocateNode(newRoot);
            return;
        }
        
        int parentOffset = path[path.size() - 2];
        Node parent;
        readNode(parentOffset, parent);
        
        // Find position to insert
        int pos = 0;
        while (pos < parent.count && key > parent.keys[pos]) pos++;
        
        parent.keys.insert(parent.keys.begin() + pos, key);
        parent.children.insert(parent.children.begin() + pos + 1, rightChild);
        parent.count++;
        
        writeNode(parentOffset, parent);
        
        // Split parent if needed
        if (parent.count > order) {
            splitInternal(path, parent, parentOffset);
        }
    }
    
    void splitInternal(const vector<int>& path, Node& node, int nodeOffset) {
        Node newInternal(false);
        int mid = node.count / 2;
        string promoteKey = node.keys[mid];
        
        // Move后半部分 to new internal node
        for (int i = mid + 1; i < node.count; i++) {
            newInternal.keys.push_back(node.keys[i]);
        }
        for (int i = mid + 1; i <= node.count; i++) {
            newInternal.children.push_back(node.children[i]);
        }
        
        newInternal.count = node.count - mid - 1;
        node.count = mid;
        
        // Write nodes
        int newOffset = allocateNode(newInternal);
        writeNode(nodeOffset, node);
        
        // Remove path to nodeOffset
        vector<int> parentPath = path;
        parentPath.pop_back();
        insertIntoParent(parentPath, promoteKey, nodeOffset, newOffset);
    }
    
    void remove(const string& key, int value) {
        if (rootOffset == -1) return;
        
        // Find leaf node
        Node node;
        vector<int> path;
        int current = rootOffset;
        
        while (true) {
            path.push_back(current);
            readNode(current, node);
            if (node.isLeaf) break;
            
            int pos = 0;
            while (pos < node.count && key > node.keys[pos]) pos++;
            current = node.children[pos];
        }
        
        // Find key in leaf
        int pos = 0;
        while (pos < node.count && key > node.keys[pos]) pos++;
        
        if (pos == node.count || node.keys[pos] != key) {
            return; // Key not found
        }
        
        // Find value in values
        auto it = find(node.values.begin(), node.values.end(), value);
        if (it == node.values.end()) {
            return; // Value not found
        }
        
        int valuePos = distance(node.values.begin(), it);
        
        // Remove value
        node.values.erase(node.values.begin() + valuePos);
        if (node.values.empty()) {
            // Remove key if no values left
            node.keys.erase(node.keys.begin() + pos);
            node.count--;
        } else {
            // Still has other values, just update count
            node.count = node.values.size();
        }
        
        writeNode(current, node);
    }
    
    vector<int> find(const string& key) {
        vector<int> result;
        if (rootOffset == -1) return result;
        
        // Find leaf node
        Node node;
        int current = rootOffset;
        
        while (true) {
            readNode(current, node);
            if (node.isLeaf) break;
            
            int pos = 0;
            while (pos < node.count && key > node.keys[pos]) pos++;
            current = node.children[pos];
        }
        
        // Find key in leaf
        int pos = 0;
        while (pos < node.count && key > node.keys[pos]) pos++;
        
        if (pos < node.count && node.keys[pos] == key) {
            return node.values;
        }
        
        return result;
    }
};

// Simple in-memory cache for performance
unordered_map<string, set<int>> cache;

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);
    
    // Use B+ Tree for file storage
    BPlusTree bptree;
    
    int n;
    cin >> n;
    
    for (int i = 0; i < n; i++) {
        string command;
        cin >> command;
        
        if (command == "insert") {
            string index;
            int value;
            cin >> index >> value;
            cache[index].insert(value);
            bptree.insert(index, value);
        }
        else if (command == "delete") {
            string index;
            int value;
            cin >> index >> value;
            auto it = cache.find(index);
            if (it != cache.end()) {
                it->second.erase(value);
                if (it->second.empty()) {
                    cache.erase(it);
                }
            }
            bptree.remove(index, value);
        }
        else if (command == "find") {
            string index;
            cin >> index;
            auto it = cache.find(index);
            if (it == cache.end() || it->second.empty()) {
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