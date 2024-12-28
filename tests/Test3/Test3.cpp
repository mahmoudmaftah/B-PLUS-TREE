#include "../../include/BPlusTree3.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>



using namespace std;

int main() {
    std::string filename = "key_value_pairs.txt";  // Replace with actual file path

    std::ifstream infile("key_value_pairs.txt");
    if (!infile) {
        std::cerr << "Failed to open key_value_pairs.txt" << std::endl;
        return 1;
    }
    
    int i = 0;
    BPlusTree<int, string> bpt(10); 
    int key; string value;
    while(infile >> key >> value){
        i += 1;
        bpt.insert(key, value);
    }
    infile.close();
    cout << "Loaded " << i << " key-value pairs." << endl;

    map<int, vector<string>> mp;
    infile.open(filename);
    while(infile >> key >> value){
        if(mp.find(key) == mp.end()){
            mp[key] = vector<string>{value};
        }else{
            mp[key].push_back(value);
        }
    }

    for(auto it = mp.begin(); it != mp.end(); it++){
        auto vals = bpt.searchAll(it->first);
        if(vals == nullptr){
            cout << "Key " << it->first << " not found." << endl;
            continue;
        }
        if(vals->size() != it->second.size()){
            cout << "Mismatch found for key: " << it->first << endl;
            return 1;
        }
        // // sort the values
        // sort(it->second.begin(), it->second.end());
        // // sort for the b+tree
        // sort(vals->begin(), vals->end());

        for(int i = 0; i < vals->size(); i++){
            if((*vals)[i] != it->second[i]){
                cout << "Mismatch found for key: " << it->first << endl;
                return 1;
            }
        }
    }

    cout << "All values for all keys are correct." << endl;



    return 0;
}
