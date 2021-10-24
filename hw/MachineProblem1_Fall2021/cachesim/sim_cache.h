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

    public:

        PseudoLRU(int assoc)
        {
            tree_bits.resize(assoc);
            depth = ceil(log2(assoc));
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
        }
};