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
        string file_line;
        string mode;
        string address;

        for (int i = 0; i < 3; i++)
        {
            trace_file >> mode >> address;

            // in case the file/line starts with some garbage chars, skip them
            // if (!isalpha(mode[0]))
            // {
            //     mode = mode[mode.length() - 1];
            // }

            string bin_address = hex2bin(address);

            cout << endl << endl << "mode: " << mode << endl;
            cout << "hex address: " << address << endl;
            cout << "bin address: \n" << endl << endl;

            l1.access(bin_address, mode);
        }

        l1.print_details();
    }

    trace_file.close();

    return 0;
}