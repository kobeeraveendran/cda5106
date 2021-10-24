/* 
 * Author: Kobee Raveendran
 * CDA5106 - Advanced Computer Architecture (Fall 2021)
 * Machine Problem 1 - Cache Simulator
 * University of Central Florida
 */

#include <iostream>
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
        int cache_level;

        // simulation results
        int reads = 0, read_misses = 0, writes = 0, write_misses = 0, writebacks = 0;

        vector<vector<Line>> cache;
        vector<int> lru_counter;
        vector<PseudoLRU> trees;

    public:
        int total_mem_traffic;
        
        // instantiant an instance of a cache for a specified cache level
        /* 
         * level (int): cache level (either L1 or L2); specified manually
         * b_size (int): blocksize, a parameter specified on the commandline
         * cache_size (int): total size of the cache, also specified on the commandline
         * cache_assoc (int): associativity of the cache, specified on the commandline
         * rep_pol (int): the replacement policy to use (0: LRU, 1: pseudo-LRU, 2: optimal); specified on the commandline
         * inc_prop (int): which inclusion property to use (0: non-inclusive, 1: inclusive); specified on the commandline
         */
        Cache(int level, int b_size, int cache_size, int cache_assoc, int rep_pol, int inc_prop)
        {
            cache_level = level;
            block_size = b_size;
            size = cache_size;
            assoc = cache_assoc;
            replacement = rep_pol;
            inclusion = inc_prop;

            if (size > 0)
            {
                num_sets = size / (assoc * block_size);
                index_bits = log2(num_sets);
                offset_bits = log2(block_size);
                tag_bits = 32 - index_bits - offset_bits;

                offset_mask = pow(2, offset_bits) - 1;
                index_mask = pow(2, index_bits) - 1;

                cache.resize(num_sets);

                if (replacement == 0)
                {
                    lru_counter.resize(num_sets);
                }
                else if (replacement == 1)
                {
                    for (int i = 0; i < num_sets; i++)
                    {
                        trees.push_back(PseudoLRU(assoc));
                    }
                }

                for (int i = 0; i < cache.size(); i++)
                {
                    cache[i].resize(assoc);
                }
            }
        }

        // print the cache's contents in the following format:
        // Set n: <line 0 hex tag> <dirty?> ... <line m hex tag> <dirty?>
        // where n is the current set number and m is the associativity of the cache
        void print_details()
        {
            cout << "===== L" << cache_level << " contents =====" << endl;

            for (int i = 0; i < cache.size(); i++)
            {
                cout << "Set " << i << ":\t\t";

                for (int j = 0; j < cache[i].size(); j++)
                {
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
        }

        // attempt to read or write to this cache, handling misses and writebacks as necessary
        // address (string): the full hex address from the address sequence
        // mode (string): the access mode, either read (r) or write (w); also extracted from the address sequence
        void access(string address, string mode)
        {
            int32_t bit_address = stoi(address, nullptr, 16);
            int offset, index, tag;

            offset = bit_address & offset_mask;
            bit_address >>= offset_bits;
            index = bit_address & index_mask;
            bit_address >>= index_bits;
            tag = bit_address;

            if (mode == "w") writes++; else reads++;

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
                            cache[index][i].dirty = 1;
                        }

                        // do replacement policy-related updates
                        if (replacement == 0)
                        {
                            // LRU
                            cache[index][i].lru_count = lru_counter[index]++;
                        }

                        // TODO: add other replacement policies
                        else if (replacement == 1)
                        {
                            // PLRU
                            trees[index].access(i);
                        }

                        // we had a hit
                        return;
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
                        writebacks++;
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
            if (mode == "w") write_misses++; else read_misses++;

            return;
        }

        // print the results of the simulation for this cache (depending on cache level)
        void print_results()
        {
            float miss_rate = (float)(read_misses + write_misses) / (float)(reads + writes);
            total_mem_traffic = read_misses + write_misses + writebacks;
            
            if (cache_level == 1)
            {
                cout << "a. number of L1 reads:\t\t" << reads << endl;
                cout << "b. number of L1 read misses:\t" << read_misses << endl;
                cout << "c. number of L1 writes:\t\t" << writes << endl;
                cout << "d. number of L1 write misses:\t" << write_misses << endl;
                cout << "e. L1 miss rate:\t\t" << fixed << setprecision(6) << miss_rate << endl;
                cout << "f. number of L1 writebacks:\t" << writebacks << endl;
            }
            else
            {
                cout << "g. number of L2 reads:\t\t" << reads << endl;
                cout << "h. number of L2 read misses:\t" << read_misses << endl;
                cout << "i. number of L2 writes:\t\t" << writes << endl;
                cout << "j. number of L2 write misses:\t" << write_misses << endl;
                cout << "k. L2 miss rate:\t\t";

                if (size == 0)
                {
                    cout << "0" << endl;
                }
                else
                {
                    cout << fixed << setprecision(6) << miss_rate << endl;
                }

                cout << "l. number of L2 writebacks:\t" << writebacks << endl;
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
    cout << "BLOCKSIZE:\t\t\t" << block_size << endl;
    cout << "L1_SIZE:\t\t\t" << l1_size << endl;
    cout << "L1_ASSOC:\t\t\t" << l1_assoc << endl;
    cout << "L2_SIZE:\t\t\t" << l2_size << endl;
    cout << "L2_ASSOC:\t\t\t" << l2_assoc << endl;
    cout << "REPLACEMENT POLICY:\t";

    switch (replacement)
    {
        case 0:
            cout << "LRU";
            break;

        case 1:
            cout << "Pseudo-LRU";
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
    Cache l2(2, block_size, l2_size, l2_assoc, replacement, inclusion);

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

            l1.access(address, mode);

            count++;
        }

        l1.print_details();
        if (l2_size > 0)
        {
            l2.print_details();
        }
    }

    trace_file.close();

    cout << "===== Simulation results (raw) =====" << endl;
    l1.print_results();
    l2.print_results();    
    cout << "m. total memory traffic:\t" << l1.total_mem_traffic + l2.total_mem_traffic << endl;


    return 0;
}