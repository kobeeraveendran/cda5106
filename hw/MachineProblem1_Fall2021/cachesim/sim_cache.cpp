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

void external_cache_access(int bit_address, string mode, int trace_index, int level);
vector<int> access_stream_l1;
vector<int> access_stream_l2;
int silent = 0;

fstream logfile;

// vector<int> extract_fields(int bit_address, int size, int block_size, int assoc)
// {
//     int index_bits = log2(size / (block_size * assoc));
//     int offset_bits = log2(block_size);
//     int tag_bits = 32 - index_bits - offset_bits;
//     int offset_mask = pow(2, offset_bits) - 1;
//     int index_mask = pow(2, index_bits) - 1;
//     int offset, index, tag;

//     vector<int> retval;

//     offset = bit_address & offset_mask;
//     bit_address >>= offset_bits;
//     index = bit_address & index_mask;
//     bit_address >>= index_bits;
//     tag = bit_address;

//     retval.push_back(tag);
//     retval.push_back(index);
//     retval.push_back(offset);

//     return retval;
// }

class Line
{
    public:
        int valid, dirty, tag, addr, lru_count = 0;

        Line()
        {
            valid = 0;
            dirty = 0;
        }

        Line(int v, int d, int t, int a)
        {
            valid = v;
            dirty = d;
            tag = t;
            addr = a;
        }

        Line(int v, int d, int t, int a, int c)
        {
            valid = v;
            dirty = d;
            tag = t;
            addr = a;
            lru_count = c;
        }
};

class Cache
{
    private:
        int cache_level;

        // simulation results
        int reads = 0, read_misses = 0, writes = 0, write_misses = 0, writebacks = 0;

        
        vector<int> lru_counter;
        vector<PseudoLRU> trees;
        vector<int> trace;

    public:
        int size, total_mem_traffic = 0;
        int block_size, assoc, replacement, inclusion;
        int num_sets, tag_bits, index_bits, offset_bits;
        int tag_mask, index_mask, offset_mask;
        
        vector<vector<Line>> cache;

        Cache(){}

        // instantiate an instance of a cache for a specified cache level
        /* 
         * level (int): cache level (either L1 or L2); specified manually
         * b_size (int): blocksize, a parameter specified on the commandline
         * cache_size (int): total size of the cache, also specified on the commandline
         * cache_assoc (int): associativity of the cache, specified on the commandline
         * rep_pol (int): the replacement policy to use (0: LRU, 1: pseudo-LRU, 2: optimal); specified on the commandline
         * inc_prop (int): which inclusion property to use (0: non-inclusive, 1: inclusive); specified on the commandline
         * access_stream (vector<int>): preprocessed access stream from the trace file (all hexstring addrs. converted to int)
         */
        Cache(int level, int b_size, int cache_size, int cache_assoc, int rep_pol, int inc_prop, vector<int> access_stream)
        {
            cache_level = level;
            block_size = b_size;
            size = cache_size;
            assoc = cache_assoc;
            replacement = rep_pol;
            inclusion = inc_prop;
            trace = access_stream;

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
        // trace_index(int): the current number of addresses in the addr. sequence (trace) we have progressed through
        void access(int32_t bit_address, string mode, int trace_index)
        {
            int offset, index, tag;

            int32_t address_copy = bit_address;

            offset = address_copy & offset_mask;
            address_copy >>= offset_bits;
            index = address_copy & index_mask;
            address_copy >>= index_bits;
            tag = address_copy;

            // vector<int> fields = extract_fields(bit_address, size, block_size, assoc);
            // tag = fields[0];
            // index = fields[1];
            // offset = fields[2];

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

                        else if (replacement == 1)
                        {
                            // PLRU
                            trees[index].access(i);
                        }

                        // no changes necessary for optimal (only consult oracle 
                        // for foresight on evictions)

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
                // update LRU
                if (replacement == 0)
                {
                    cache[index][invalid_index] = Line(1, 1, tag, bit_address, lru_counter[index]++);
                }

                // update Pseudo-LRU
                else if (replacement == 1)
                {
                    // since this is an access, update the tree bit array
                    trees[index].access(invalid_index);
                    cache[index][invalid_index] = Line(1, 1, tag, bit_address);
                }

                // no updates necessary for optimal, just write to the block
                else if (replacement == 2)
                {
                    cache[index][invalid_index] = Line(1, 1, tag, bit_address);
                }

                // for both non-inclusive and inclusive, issue a read to the next level 
                // cache (if not there already)
                if (cache_level == 1)
                {
                    external_cache_access(bit_address, "r", trace_index, 2);
                }

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
                int replacement_index;

                if (replacement == 0)
                {
                    // find the LRU block
                    int min_count = INT32_MAX;

                    for (int i = 0; i < cache[index].size(); i++)
                    {
                        if (cache[index][i].lru_count < min_count)
                        {
                            min_count = cache[index][i].lru_count;
                            replacement_index = i;
                        }
                    }                    
                }

                else if (replacement == 1)
                {
                    // find the LRU block according to PLRU
                    replacement_index = trees[index].replace();
                }

                else if (replacement == 2)
                {
                    // optimal - use foresight to determine the block to replace
                    // and replace the leftmost one in case multiple are not reused
                    vector<int> offsets;
                    int max_offset = -1;
                    int next_use;

                    for (int i = 0; i < cache[index].size(); i++)
                    {
                        // find the offset for this block
                        next_use = foresight(cache[index][i].addr >> offset_bits, trace_index, trace);
                        offsets.push_back(next_use);
                    }

                    // of all the offsets, find the maximum value; if any are equal 
                    // to the size of the trace, they won't be used again so use the 
                    // leftmost such block (in case there are ties)
                    for (int i = 0; i < offsets.size(); i++)
                    {
                        if (offsets[i] > max_offset)
                        {
                            max_offset = offsets[i];
                            replacement_index = i;
                        }
                    }
                }

                if (cache[index][replacement_index].dirty)
                {
                    // if dirty, we must first write-back to memory (or next level cache) before evicting
                    writebacks++;

                    if (cache_level == 1)
                    {
                        // write the victim block to L2 (if it exists) - step 1 of allocation
                        external_cache_access(cache[index][replacement_index].addr, "w", trace_index, 2);
                    }

                    // for inclusive outer caches only
                    if (cache_level == 2 && inclusion)
                    {
                        external_cache_access(cache[index][replacement_index].addr, "w", trace_index, 1);
                    }
                }

                if (replacement == 0)
                {
                    cache[index][replacement_index] = Line(1, 0, tag, bit_address, lru_counter[index]++);
                }
                else
                {
                    cache[index][replacement_index] = Line(1, 0, tag, bit_address);
                }

                // step 2 of allocating a block
                if (cache_level == 1)
                {
                    external_cache_access(bit_address, "r", trace_index, 2);
                }

                if (mode == "w")
                {
                    // if we wrote instead of read, dirty bit must be set
                    cache[index][replacement_index].dirty = 1;
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
            total_mem_traffic += read_misses + write_misses + writebacks;
            
            // for arrangements of output, see my python script
            if (size > 0 && silent != 0)
            {
                // logfile.open("graph_logs/output.csv", ios::app);

                if (silent == 1)
                {
                    // graph #1: L1 MR and assoc. vs size
                    cout << miss_rate << endl;
                }
                else if (silent >= 2 && silent <= 4)
                {
                    // graphs 2, 3, and 4 (deal with AAT)
                    // requires some postprocessing on the python side using 
                    // CACTI table values and these values
                    cout << reads << "," << read_misses << "," << writes << "," << write_misses << endl;
                }

                // logfile.close();
            }

            if (!silent)
            {
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
                        miss_rate = (float)(read_misses) / (float)(reads);
                        cout << fixed << setprecision(6) << miss_rate << endl;
                    }

                    cout << "l. number of L2 writebacks:\t" << writebacks << endl;
                }
            }
        }
};

Cache l1(0, 0, 0, 0, 0, 0, {0});
Cache l2(0, 0, 0, 0, 0, 0, {0});

// Wrapper method for cache access that enables accessing another cache within 
// one (i.e. writing back to L2 within L1)
/*
 * bit_address (int): the full hex address from the address sequence/evicted block, in decimal form
 * mode (string): either "r" (read) or "w" (write); only applicable when writing back to L2
 * trace_index (int): index of the address we're processing in the address sequence; only for optimal replacement pol
 * level (int): which cache level we are trying to access; 2 for L2 (acc. from L1), 1 for accessing L1 from L2
 */
void external_cache_access(int bit_address, string mode, int trace_index, int level)
{
    if (level == 2 && l2.size > 0)
    {
        l2.access(bit_address, mode, trace_index);
    }
    else if (level == 1)
    {
        int offset, index, tag;
        
        offset = bit_address & l1.offset_mask;
        bit_address >>= l1.offset_bits;
        index = bit_address & l1.index_mask;
        bit_address >>= l1.index_bits;
        tag = bit_address;

        // find the block in the L1 cache where it's supposed to be
        for (int i = 0; i < l1.cache[index].size(); i++)
        {
            if (l1.cache[index][i].valid)
            {
                // compare tags
                if (l1.cache[index][i].tag == tag)
                {
                    // allow this block to be replaced later on by marking it invalid
                    l1.cache[index][i].valid = 0;

                    // also, if it's dirty, write it to main memory
                    if (l1.cache[index][i].dirty)
                    {
                        l2.total_mem_traffic++;
                    }

                    return;
                }
            }
        }
    }

    return;
}

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
 * silent: int; whether to print full simulation results (0/don't specify), or print
 *         results needed for the report graphs (to be fed into my python script):
 *         (1) - graph #1 (L1 MR & assoc. vs. log2(SIZE))
 *         (2) - graph #2 (prints reads, read misses, writes, and write misses)
 *         (3) - graph #3 (same as 2)
 *         (4) - graph #4 (same as 2)
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
    
    if (argc > 9)
    {
        silent = stoi(argv[9]);
    }

    // sample run cmd:
    // ./sim_cache 16 1024 2 0 0 0 0 ../trace_files/gcc_trace.txt

    if (!silent)
    {
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
                cout << "Optimal";
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
        cout << "trace_file:\t\t" << trace_path << endl;
    }
    
    // preprocess the trace files for optimal replacement pol
    if (replacement == 2)
    {
        access_stream_l1 = preprocesses_trace(trace_path, log2(block_size));
        if (l2_size > 0)
        {
            access_stream_l2 = preprocesses_trace(trace_path, log2(block_size));
        }
    }

    l1 = Cache(1, block_size, l1_size, l1_assoc, replacement, inclusion, access_stream_l1);
    l2 = Cache(2, block_size, l2_size, l2_assoc, replacement, inclusion, access_stream_l2);

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

            l1.access(stoi(address, nullptr, 16), mode, count);
            count++;
        }

        if (!silent)
        {
            l1.print_details();
            if (l2_size > 0)
            {
                l2.print_details();
            }
        }
    }

    trace_file.close();

    if (!silent)
    {
        cout << "===== Simulation results (raw) =====" << endl;
    }

    l1.print_results();
    l2.print_results();
    
    if (!silent)
    {
        if (l2_size > 0)
        {
            cout << "m. total memory traffic:\t" << l2.total_mem_traffic << endl;
        }
        else
        {
            cout << "m. total memory traffic:\t" << l1.total_mem_traffic << endl;
        }
    }

    return 0;
}