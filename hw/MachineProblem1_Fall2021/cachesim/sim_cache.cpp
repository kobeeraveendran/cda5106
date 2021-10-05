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
int main(int argc, char *argv[]) {
    
    int block_size = stoi(argv[1]);
    int l1_size = stoi(argv[2]);
    int l1_assoc = stoi(argv[3]);
    int l2_size = stoi(argv[4]);
    int l2_assoc = stoi(argv[5]);
    int replacement = stoi(argv[6]);
    int inclusion = stoi(argv[7]);
    string trace_path = argv[8];

    cout << "===== Simulator configuration =====" << endl;
    cout << "BLOCKSIZE:\t\t" << block_size << endl;
    cout << "L1_SIZE:\t\t" << l1_size << endl;
    cout << "L1_ASSOC:\t\t" << l1_assoc << endl;
    cout << "L2_SIZE:\t\t" << l2_size << endl;
    cout << "L2_ASSOC:\t\t" << l2_assoc << endl;
    cout << "REPLACEMENT POLICY:\t" << replacement << endl;
    cout << "INCLUSION PROPERTY:\t" << inclusion << endl;
    cout << "trace file:\t\t" << trace_path << endl;

    return 0;
}