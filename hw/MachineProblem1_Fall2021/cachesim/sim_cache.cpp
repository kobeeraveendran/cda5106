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

int l1_writebacks = 0, l2_writebacks = 0;

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
        int cache_level;
        vector<vector<Line>> cache;
        vector<int> lru_counter;

    public:
        
        Cache(int level, int b_size, int cache_size, int cache_assoc, int rep_pol, int inc_prop)
        {
            cache_level = level;
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
            // cout << endl << "CACHE DETAILS:" << endl;
            // cout << "Number of sets: " << num_sets << endl;
            // cout << "Index bits: " << index_bits << endl;
            // cout << "Block offset bits: " << offset_bits << endl;
            // cout << "Tag bits: " << tag_bits << endl;

            cout << "===== L" << cache_level << " contents =====" << endl;

            for (int i = 0; i < cache.size(); i++)
            {
                cout << "Set " << i << ":\t\t";

                for (int j = 0; j < cache[i].size(); j++)
                {
                    // cout << "V: " << cache[i][j].valid << ' ';
                    stringstream ss;
                    ss << hex << cache[i][j].tag;
                    string dirty;

                    if (cache[i][j].dirty)
                    {
                        dirty = " D";
                    }
                    else
                    {
                        dirty = "  ";
                    }

                    cout << setw(8) << ss.str() << dirty << '\t';
                }

                cout << endl;
            }

            int count_sum = 0;

            for (int i = 0; i < lru_counter.size(); i++)
            {
                count_sum += lru_counter[i];
            }
        }

        int access(string address, string mode)
        {
            int32_t bit_address = stoi(address, nullptr, 16);
            int offset, index, tag;

            offset = bit_address & offset_mask;
            bit_address >>= offset_bits;
            index = bit_address & index_mask;
            bit_address >>= index_bits;
            tag = bit_address;

            // for both reads and writes
            int invalid_index = -1;

            // find available valid blocks with matching tag
            for (int i = 0; i < cache[index].size(); i++)
            {
                if (cache[index][i].valid)
                {
                    // if valid, compare the tags
                    if (cache[index][i].tag == tag)
                    {
                        // we have a hit; if writing, mark the block as dirty
                        if (mode == "w")
                        {
                            cache[index][i].dirty = 1; //(mode == "r") ? 0 : 1;
                        }

                        // do replacement policy-related updates
                        if (replacement == 0)
                        {
                            // LRU
                            cache[index][i].lru_count = lru_counter[index]++;
                        }

                        // TODO: add other replacement policies

                        // we had a hit
                        return 0;
                    }
                }
                else if (invalid_index == -1)
                {
                    // maintain the earliest-seen invalid index in case it needs to be filled
                    invalid_index = i;
                }
            }

            // if there was a miss and an invalid index, write to it and make it valid
            if (invalid_index != -1)
            {
                // for write misses, valid = 1 and dirty = 1
                cache[index][invalid_index] = Line(1, 1, tag, lru_counter[index]++);

                // if reading instead, mem will be up-to-date, so dirty = 0
                if (mode == "r")
                {
                    cache[index][invalid_index].dirty = 0;
                }
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

                    if (cache[index][min_index].dirty)
                    {
                        // if dirty, we must first write-back to memory (or next level cache) before evicting
                        // TODO
                        if (cache_level == 1)
                        {
                            l1_writebacks++;
                        }
                        else if (cache_level == 2)
                        {
                            l2_writebacks++;
                        }
                    }

                    cache[index][min_index] = Line(1, 0, tag, lru_counter[index]++);

                    if (mode == "w")
                    {
                        // if we wrote instead of read, dirty bit must be set
                        cache[index][min_index].dirty = 1;
                    }

                    // TODO: handle eviction to next level of mem hierarchy
                }
            }

            // we had a miss
            return 1;
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

    Cache l1(1, block_size, l1_size, l1_assoc, replacement, inclusion);

    if (l2_size > 0)
    {
        Cache l2(2, block_size, l2_size, l2_assoc, replacement, inclusion);
    }

    // simulation results
    int l1_reads = 0, l1_readmisses = 0, l1_writes = 0, l1_writemisses = 0;
    int l2_reads = 0, l2_readmisses = 0, l2_writes = 0, l2_writemisses = 0;

    // read address sequence from file, line by line
    fstream trace_file;

    trace_file.open(trace_path, ios::in);

    if (trace_file.is_open())
    {
        int count = 0;
        string file_line;
        string mode;
        string address;
        int res;

        while (trace_file >> mode >> address)
        {

            // in case the file/line starts with some garbage chars, skip them
            if (!isalpha(mode[0]))
            {
                mode = mode[mode.length() - 1];
            }

            res = l1.access(address, mode);

            if (mode == "w")
            {
                l1_writemisses += res;
                l1_writes++;
            }
            else
            {
                l1_readmisses += res;
                l1_reads++;
            }

            count++;
        }

        l1.print_details();
    }

    trace_file.close();

    // TODO: refactor all this to track it within the cache class

    int total_mem_traffic = l1_readmisses + l1_writemisses + l1_writebacks + 
                             l2_readmisses + l2_writemisses + l2_writebacks;

    float l1_missrate = (float)(l1_readmisses + l1_writemisses) / (float)(l1_reads + l1_writes);
    float l2_missrate = 0;
    
    if (l2_size > 0)
    {
        l2_missrate = (float)(l2_readmisses + l2_writemisses) / (float)(l2_reads + l2_writes);
    }

    cout << "===== Simulation results (raw) =====" << endl;
    cout << "a. number of L1 reads:\t\t" << l1_reads << endl;
    cout << "b. number of L1 read misses:\t" << l1_readmisses << endl;
    cout << "c. number of L1 writes:\t\t" << l1_writes << endl;
    cout << "d. number of L1 write misses:\t" << l1_writemisses << endl;
    cout << "e. L1 miss rate:\t\t" << fixed << setprecision(6) << l1_missrate << endl;
    cout << "f. number of L1 writebacks:\t" << l1_writebacks << endl;
    cout << "g. number of L2 reads:\t\t" << l2_reads << endl;
    cout << "h. number of L2 read misses:\t" << l2_readmisses << endl;
    cout << "i. number of L2 writes:\t\t" << l2_writes << endl;
    cout << "j. number of L2 write misses:\t" << l2_writemisses << endl;
    if (l2_size == 0)
    {
        cout << "k. L2 miss rate:\t\t0" << endl;
    }
    else
    {
        cout << "k. L2 miss rate:\t\t" << fixed << setprecision(6) << l2_missrate << endl;
    }
    cout << "l. number of L2 writebacks:\t" << l2_writebacks << endl;
    cout << "m. total memory traffic:\t" << total_mem_traffic << endl;


    return 0;
}