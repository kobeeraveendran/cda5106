/*
 * Author: Kobee Raveendran
 * CDA5106: Advanced Computer Architecture (Fall 2021)
 * Machine Problem 1 - Cache Simulator
 * University of Central Florida
 */

#include <string>
#include <unordered_map>
#include <iomanip>
#include <vector>
#include <algorithm>

using namespace std;

class PseudoLRU
{
    private:
        vector<int> tree_bits;
        int depth;
        int set_size;

    public:

        PseudoLRU(int assoc)
        {
            tree_bits.resize(assoc - 1);
            depth = ceil(log2(assoc));
            set_size = assoc;
        }

        void access(int index)
        {
            vector<int> index_bits;
            int index_copy = index;

            for (int i = 0; i < depth; i++)
            {
                index_bits.push_back(index_copy & 1);
                index_copy >>= 1;
            }

            reverse(index_bits.begin(), index_bits.end());

            // for indexing the binary tree
            int j = 0;

            for (int i = 0; i < depth; i++)
            {
                if (index_bits[i])
                {
                    // set node's value
                    tree_bits[j] = index_bits[i];
                    // go to index of the right child
                    j = 2 * j + 2;
                }
                else
                {
                    // set node's value
                    tree_bits[j] = index_bits[i];
                    // go to index of the left child
                    j = 2 * j + 1;
                }
            }

            // cout << "TREE BITS AFTER ACCESS TO INDEX " << index << ": ";

            // for (int i = 0; i < tree_bits.size(); i++)
            // {
            //     cout << tree_bits[i];
            // }

            // cout << endl;
        }

        int replace()
        {
            // for indexing the binary tree
            int j = 0;
            // for finding the LRU index in the cache set
            int low = 0, high = set_size - 1, mid;

            // cout << "INITIAL TREE BITS BEFORE REPLACEMENT: ";

            // for (int i = 0; i < tree_bits.size(); i++)
            // {
            //     cout << tree_bits[i];
            // }

            // cout << endl;

            for (int i = 0; i < depth; i++)
            {
                mid = (low + high) / 2;
                if (tree_bits[j])
                {
                    // switch the bit along the path
                    tree_bits[j] = 0;
                    // go to the index of the left child (opposite from access())
                    j = 2 * j + 1;
                    // search in left half of set
                    high = mid;
                    mid = high;
                }
                else
                {
                    // switch bit along the path
                    tree_bits[j] = 1;
                    // go to index of the right child
                    j = 2 * j + 2;
                    // search in right half of set
                    low = mid + 1;
                    mid = low;
                }
            }

            // cout << "TREE BITS AFTER REPLACEMENT: ";

            // for (int i = 0; i < tree_bits.size(); i++)
            // {
            //     cout << tree_bits[i];
            // }

            // cout << endl << "Index to be evicted: " << mid << endl;

            return mid;
        }
};

vector<int> preprocesses_trace(string filepath)
{
    vector<int> access_stream;
    fstream trace_file;

    trace_file.open(filepath, ios::in);

    if (trace_file.is_open())
    {
        string file_line, mode, address;

        while (trace_file >> mode >> address)
        {
            // in case there are extra chars in the front of the line
            if (!isalpha(mode[0]))
            {
                mode = mode[mode.length() - 1];
            }

            int32_t bit_address = stoi(address, nullptr, 16);
            access_stream.push_back(bit_address);
        }
    }

    trace_file.close();

    return access_stream;
}

// "predict the future" in the Belady optimal replacement algorithm by determining 
// which block in the line will be least urgently needed in the future
/*
 * query_address(int): the address we want to find t steps in the future
 * trace_index(int): our current progress through the access stream
 * trace(vector<int>): the array of accesses from the preprocessing stage
 */
int foresight(int query_address, int trace_index, vector<int> trace)
{

    // cout << "QUERY ADDRESS: " << query_address << endl;
    // determine how many timesteps in the future this block is needed again
    for (int i = trace_index; i < trace.size(); i++)
    {
        if (query_address == trace[i])
        {
            return i - trace_index;
        }
    }

    // otherwise, it was never used again, so it should be replaced
    stringstream ss;
    ss << hex << query_address;
    cout << "QUERY ADDR: " << ss.str() << " at trace index: " << trace_index << endl;
    return trace.size();
};