/* 
 * Author: Kobee Raveendran
 * CDA5106 - Advanced Computer Architecture (Fall 2021)
 * Machine Problem 1 - Cache Simulator
 * University of Central Florida
 */

#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
#include <cmath>
#include <fstream>
#include "sim_cache.h"

using namespace std;

// an individual cache set
/* 
 * contains the lines in the cache set, as specified by the cache associativity
 * also maintains a counter (for LRU replacement policy)
 */
class CacheSet
{
    private:
        vector<int> data;
        int counter = 0;
};

// generic cache class definition
class Cache
{
    private:
        int block_size, size, assoc;
        int replacement, inclusion;
        int num_sets;
        int tag_bits, index_bits, offset_bits;
        vector<CacheSet> cache;

    public:
        Cache(int b_size, int cache_size, int cache_assoc, int rep_pol, int inc_prop)
        {
            block_size = b_size;
            size = cache_size;
            assoc = cache_assoc;
            replacement = rep_pol;
            inclusion = inc_prop;

            num_sets = size / (assoc * block_size);
            index_bits = log2(num_sets);
            offset_bits = log2(block_size);
            tag_bits = 32 - index_bits - offset_bits;
        }

        void print_details()
        {
            cout << endl << "CACHE DETAILS:" << endl;
            cout << "Number of sets: " << num_sets << endl;
            cout << "Index bits: " << index_bits << endl;
            cout << "Block offset bits: " << offset_bits << endl;
            cout << "Tag bits: " << tag_bits << endl;
        }

        void access(string address, char mode)
        {
            string tag = address.substr(0, tag_bits);
            string index = address.substr(tag_bits, tag_bits + index_bits);
            string offset = address.substr(tag_bits + index_bits, tag_bits + index_bits + offset_bits);

            cout << "Tag: " << tag << endl;
            cout << "Index: " << index << endl;
            cout << "Offset: " << offset << endl;
        }
};

// commandline args:
/* 
 * BLOCKSIZE: int
 * L1_SIZE: int
 * L1_ASSOC: int
 * L2_SIZE: int
 * L2_ASSOC: int
 * REPLACEMENT_POLICY: int (0: LRU, 1: PLRU, 2: optimal)
 * INCLUSION_PROPERTY: int (0: non-inclusive, 1: inclusive)
 * trace_file: string (trace file path with extension)
 */
int main(int argc, char *argv[])
{
    int block_size = stoi(argv[1]);
    int l1_size = stoi(argv[2]);
    int l1_assoc = stoi(argv[3]);
    int l2_size = stoi(argv[4]);
    int l2_assoc = stoi(argv[5]);
    int replacement = stoi(argv[6]);
    int inclusion = stoi(argv[7]);
    string trace_path = argv[8];

    // dummy run cmd:
    // ./sim_cache 32 256 1 256 1 0 0 ../traces/gcc_trace.txt

    cout << "===== Simulator configuration =====" << endl;
    cout << "BLOCKSIZE:\t\t" << block_size << endl;
    cout << "L1_SIZE:\t\t" << l1_size << endl;
    cout << "L1_ASSOC:\t\t" << l1_assoc << endl;
    cout << "L2_SIZE:\t\t" << l2_size << endl;
    cout << "L2_ASSOC:\t\t" << l2_assoc << endl;
    cout << "REPLACEMENT POLICY:\t" << replacement << endl;
    cout << "INCLUSION PROPERTY:\t" << inclusion << endl;
    cout << "trace file:\t\t" << trace_path << endl;

    Cache l1(block_size, l1_size, l1_assoc, replacement, inclusion);
    l1.print_details();

    // read address sequence from file
    string file_line;
    fstream trace_file(trace_path);
    string mode, address;

    for (int i = 0; i < 1; i++)
    {
        getline(trace_file, file_line);
        cout << "full line: " << file_line << endl;
        cout << "mode: " << file_line.at(0) << endl;
        cout << "address: " << file_line.substr(2, file_line.length()) << endl;;
    }

    // while (getline(trace_file, file_line))
    // {
    //     continue;
    // }

    trace_file.close();

    return 0;
}