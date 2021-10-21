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
#include <bitset>
#include <iomanip>
#include "sim_cache.h"

using namespace std;

class Line
{
    public:
        int valid, dirty, tag, lru_count = 0;

        Line()
        {
            valid = 0;
            dirty = 0;
        }

        Line(int v, int d, int t, int c)
        {
            valid = v;
            dirty = d;
            tag = t;
            lru_count = c;
        }
};

class Cache
{
    private:
        int block_size, size, assoc, replacement, inclusion;
        int num_sets, tag_bits, index_bits, offset_bits;
        int tag_mask, index_mask, offset_mask;
        vector<vector<Line>> cache;
        vector<int> lru_counter;

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

            offset_mask = pow(2, offset_bits) - 1;
            index_mask = pow(2, index_bits) - 1;

            cache.resize(num_sets);
            lru_counter.resize(num_sets);

            for (int i = 0; i < cache.size(); i++)
            {
                cache[i].resize(assoc);
            }
        }

        void print_details()
        {
            cout << endl << "CACHE DETAILS:" << endl;
            cout << "Number of sets: " << num_sets << endl;
            cout << "Index bits: " << index_bits << endl;
            cout << "Block offset bits: " << offset_bits << endl;
            cout << "Tag bits: " << tag_bits << endl;

            cout << endl << "CACHE VISUALIZATION:" << endl;

            for (int i = 0; i < cache.size(); i++)
            {
                cout << "Set " << i << ":\tLRU count: " << lru_counter[i] << "\t";

                for (int j = 0; j < cache[i].size(); j++)
                {
                    cout << "V: " << cache[i][j].valid << ' ';
                    cout << "C: " << cache[i][j].lru_count << ' ';
                    stringstream ss;
                    ss << hex << cache[i][j].tag;
                    cout << "T: " << setw(6) << ss.str() << '\t';
                }

                cout << endl;
            }

            int count_sum = 0;

            for (int i = 0; i < lru_counter.size(); i++)
            {
                count_sum += lru_counter[i];
            }

            cout << endl;
        }

        void access(string address, string mode)
        {
            int32_t bit_address = stoi(address, nullptr, 16);
            int offset, index, tag;

            offset = bit_address & offset_mask;
            bit_address >>= offset_bits;
            index = bit_address & index_mask;
            bit_address >>= index_bits;
            tag = bit_address;

            // for both reads and writes
            int first_invalid_index = -1;

            // find available valid blocks with matching tag
            for (int i = 0; i < cache[index].size(); i++)
            {
                if (cache[index][i].valid)
                {
                    // if valid, compare the tags
                    if (cache[index][i].tag == tag)
                    {
                        // we have a hit; if reading, make the block clean if dirty
                        // or, if writing, mark the block as dirty until it's read later
                        cache[index][i].dirty = (mode == "r") ? 0 : 1;

                        // do replacement policy-related updates
                        if (replacement == 0)
                        {
                            // LRU
                            cache[index][i].lru_count = lru_counter[index]++;
                        }

                        // TODO: add other replacement policies
                    }
                }
                else if (first_invalid_index == -1)
                {
                    first_invalid_index = i;
                }
            }

            // if there was a miss and an invalid index, write to it and make it valid
            if (first_invalid_index != -1)
            {
                cache[index][first_invalid_index] = Line(1, 1, tag, lru_counter[index]++);
            }
            else
            {
                // if there were no invalid indices, evict the block as determined by the 
                // replacement policy
                if (replacement == 0)
                {
                    // find the LRU block
                    int min_index = 0;
                    int min_count = INT32_MAX;

                    for (int i = 0; i < cache[index].size(); i++)
                    {
                        if (cache[index][i].lru_count < min_count)
                        {
                            min_count = cache[index][i].lru_count;
                            min_index = i;
                        }
                    }

                    // replace with new block
                    cache[index][min_index] = Line(1, 1, tag, lru_counter[index]++);

                    // TODO: handle eviction to next level of mem hierarchy
                }
            }
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

    // sample run cmd:
    // ./sim_cache 16 1024 2 0 0 0 0 ../trace_files/gcc_trace.txt

    cout << "===== Simulator configuration =====" << endl;
    cout << "BLOCKSIZE:\t\t" << block_size << endl;
    cout << "L1_SIZE:\t\t" << l1_size << endl;
    cout << "L1_ASSOC:\t\t" << l1_assoc << endl;
    cout << "L2_SIZE:\t\t" << l2_size << endl;
    cout << "L2_ASSOC:\t\t" << l2_assoc << endl;
    cout << "REPLACEMENT POLICY:\t";

    switch (replacement)
    {
        case 0:
            cout << "LRU";
            break;

        case 1:
            cout << "PLRU";
            break;

        case 2:
            cout << "optimal";
            break;

        default:
            cout << "LRU";
            break;
    }

    cout << endl;
    cout << "INCLUSION PROPERTY:\t";

    if (inclusion)
    {
        cout << "inclusive";
    }
    else
    {
        cout << "non-inclusive";
    }

    cout << endl;
    cout << "trace file:\t\t" << trace_path << endl;

    Cache l1(block_size, l1_size, l1_assoc, replacement, inclusion);
    l1.print_details();

    if (l2_size > 0)
    {
        Cache l2(block_size, l2_size, l2_assoc, replacement, inclusion);
        l2.print_details();
    }

    // read address sequence from file, line by line
    fstream trace_file;

    trace_file.open(trace_path, ios::in);

    if (trace_file.is_open())
    {
        int count = 0;
        string file_line;
        string mode;
        string address;

        while (trace_file >> mode >> address)
        {

            // in case the file/line starts with some garbage chars, skip them
            if (!isalpha(mode[0]))
            {
                mode = mode[mode.length() - 1];
            }

            // string bin_address = hex2bin(address);

            // cout << endl << endl << "mode: " << mode << endl;
            // cout << "hex address: " << address << endl;
            // cout << "bin address: ";

            // l1.access(bin_address, mode);
            l1.access(address, mode);
            count++;
        }

        l1.print_details();

        cout << "addresses processed: " << count << endl;
    }

    trace_file.close();

    return 0;
}