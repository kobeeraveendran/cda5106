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