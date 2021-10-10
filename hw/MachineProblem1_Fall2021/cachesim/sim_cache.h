/*
 * Author: Kobee Raveendran
 * CDA5106: Advanced Computer Architecture (Fall 2021)
 * Machine Problem 1 - Cache Simulator
 * University of Central Florida
 */

#include <string>

using namespace std;

// an individual cache line (cell)
class Line
{
    public:
        int valid, dirty;
        string tag;

        Line()
        {
            valid = 0;
            dirty = 0;
        }

        void write_to_line(string tag)
        {
            tag = tag;
            valid = 1;
        }
};

// an individual cache set
/* 
 * contains the lines in the cache set, as specified by the cache associativity
 * also maintains a counter (for LRU replacement policy)
 */
class CacheSet
{
    public:
        vector<Line> set;
        int counter;

        CacheSet(int assoc)
        {
            set.resize(assoc);
            counter = 0;
        }
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

            for (int i = 0; i < num_sets; i++)
            {
                cache.push_back(CacheSet(assoc));
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
                vector<Line> cache_set;
                cache_set = cache[i].set;

                cout << "Set " << i << ":\t";

                for (int j = 0; j < cache_set.size(); j++)
                {
                    cout << "V: " << cache_set[j].valid << ' ';
                    cout << "D: " << cache_set[j].dirty << ' ';
                    cout << "tag: " << cache_set[j].tag << '\t';
                }

                cout << endl;
            }
        }

        void access(string address, string mode)
        {
            string tag = address.substr(0, tag_bits);
            string index = address.substr(tag_bits, index_bits);
            string offset = address.substr(tag_bits + index_bits, offset_bits);

            cout << tag << ' ' << index << ' ' << offset << endl;

            int set_index = stoi(index, nullptr, 2);
        }
};

// convert hex string to 32-bit binary string, padding with leading zeros if needed
string hex2bin(string address)
{
    string new_address = "";

    if (address.length() < 8)
    {
        int offset = 8 - address.length();

        for (int i = 0; i < offset; i++)
        {
            new_address.append("0");
        }

        new_address.append(address);
    }
    else
    {
        new_address = address;
    }

    string binary;

    for (int i = 0; i < new_address.length(); i++)
    {
        switch (new_address[i])
        {
        case '0':
            binary.append("0000");
            break;

        case '1':
            binary.append("0001");
            break;

        case '2':
            binary.append("0010");
            break;

        case '3':
            binary.append("0011");
            break;

        case '4':
            binary.append("0100");
            break;

        case '5':
            binary.append("0101");
            break;

        case '6':
            binary.append("0110");
            break;

        case '7':
            binary.append("0111");
            break;

        case '8':
            binary.append("1000");
            break;

        case '9':
            binary.append("1001");
            break;

        case 'a':
            binary.append("1010");
            break;

        case 'b':
            binary.append("1011");
            break;

        case 'c':
            binary.append("1100");
            break;

        case 'd':
            binary.append("1101");
            break;

        case 'e':
            binary.append("1110");
            break;

        case 'f':
            binary.append("1111");
        
        default:
            break;
        }
    }

    return binary;
}