/*
 * Author: Kobee Raveendran
 * CDA5106: Advanced Computer Architecture (Fall 2021)
 * Machine Problem 1 - Cache Simulator
 * University of Central Florida
 */

#include <string>
#include <unordered_map>

using namespace std;

string bin2hex(string address);
string hex2bin(string address);

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

            string hex_tag = bin2hex(tag);
            cout << "TAG AS HEX: " << hex_tag << endl;
        }
};

// convert binary string to hex
string bin2hex(string address)
{
    string new_address;

    // pad with zeros if length is not a multiple of 4
    if (address.length() % 4 != 0)
    {
        int new_length = (address.length() / 4 + 1) * 4;
        string padding(new_length - address.length(), '0');
        padding.append(address);
        new_address = padding;
    }
    else
    {
        new_address = address;
    }

    string hex;
    unordered_map<string, string> bin_map;

    bin_map["0000"] = '0';
    bin_map["0001"] = '1';
    bin_map["0010"] = '2';
    bin_map["0011"] = '3';
    bin_map["0100"] = '4';
    bin_map["0101"] = '5';
    bin_map["0110"] = '6';
    bin_map["0111"] = '7';
    bin_map["1000"] = '8';
    bin_map["1001"] = '9';
    bin_map["1010"] = 'a';
    bin_map["1011"] = 'b';
    bin_map["1100"] = 'c';
    bin_map["1101"] = 'd';
    bin_map["1110"] = 'e';
    bin_map["1111"] = 'f';

    for (int i = 0; i < new_address.length(); i += 4)
    {
        string tmp = new_address.substr(i, 4);
        hex.append(bin_map[tmp]);
    }

    return hex;
}

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
    unordered_map<char, string> hex_map;

    hex_map['0'] = "0000";
    hex_map['1'] = "0001";
    hex_map['2'] = "0010";
    hex_map['3'] = "0011";
    hex_map['4'] = "0100";
    hex_map['5'] = "0101";
    hex_map['6'] = "0110";
    hex_map['7'] = "0111";
    hex_map['8'] = "1000";
    hex_map['9'] = "1001";
    hex_map['a'] = "1010";
    hex_map['b'] = "1011";
    hex_map['c'] = "1100";
    hex_map['d'] = "1101";
    hex_map['e'] = "1110";
    hex_map['f'] = "1111";

    for (int i = 0; i < new_address.length(); i++)
    {
        binary.append(hex_map[new_address[i]]);
    }

    return binary;
}