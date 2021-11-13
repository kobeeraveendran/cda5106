#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <cmath>
#include <iomanip>

using namespace std;

int silent = 0;

class Smith
{
    private:
        int mispreds = 0, total_preds = 0;
        int count, max_count;

    public:

        Smith(){}

        Smith(int num_bits)
        {
            count = pow(2, num_bits - 1);
            max_count = count * 2 - 1;
        }

        void predict(int outcome)
        {
            int prediction;

            total_preds++;

            if (count > max_count / 2)
            {
                // over the boundary; predict taken
                prediction = 1;
            }
            else
            {
                // under the boundary; predict not taken
                prediction = 0;
            }

            if (prediction != outcome)
            {
                mispreds++;
            }

            // update counter accordingly

            // if we actually took the branch, increment until saturation
            if (outcome == 1)
            {
                count = min(count + 1, max_count);
            }
            else
            {
                // if we didn't take the branch, decrement until saturation
                count = max(count - 1, 0);
            }
        }

        void print_results()
        {
            float mispred_rate = (float) mispreds / (float) total_preds * 100;
            mispred_rate = roundf(mispred_rate * 100) / 100;

            if (silent)
            {
                cout << mispred_rate << endl;
            }
            else
            {
                cout << "OUTPUT" << endl;
                cout << "number of predictions:\t\t" << total_preds << endl;
                cout << "number of mispredictions:\t" << mispreds << endl;
                cout << "misprediction rate:\t\t\t"  << fixed << setprecision(2) << mispred_rate << "%" << endl;
                cout << "FINAL COUNTER CONTENT:\t\t" << count << endl;
            }
        }
};

class Gshare
{
    private:
        int mispreds = 0, total_preds = 0;
        // these are the default counter params as specified in the doc
        int counter = 4, max_count = 7;
        int pc_bits, bhr_bits;
        vector<int> table;
        int bhr = 0;

    public:

        Gshare(){}

        Gshare(int pc, int bhr)
        {
            pc_bits = pc;
            bhr_bits = bhr;
            int table_size = pow(2, pc_bits);

            for (int i = 0; i < table_size; i++)
            {
                table.push_back(counter);
            }
        }

        void predict(int address, int outcome)
        {
            total_preds++;

            // discard lower 2 bits of the PC address
            address >>= 2;
            
            // compute the PC's contribution using its lower n bits
            int pc_index = address & ((int) pow(2, pc_bits) - 1);

            // compute the Gshare table index
            // note that if n=0, this always evaluates to the pc_index above
            int index = (bhr ^ pc_index);

            // make prediction based on table[index]'s counter
            int prediction;

            if (table[index] >= 4)
            {
                // predict taken
                prediction = 1;
            }

            else
            {
                //predict not taken
                prediction = 0;
            }

            if (prediction != outcome)
            {
                mispreds++;
            }

            // evaluate our prediction and update counters as needed
            if (outcome == 1)
            {
                table[index] = min(table[index] + 1, max_count);
            }

            else
            {
                table[index] = max(table[index] - 1, 0);
            }

            // update the branch history register if n > 0
            if (bhr_bits > 0)
            {
                bhr >>= 1;
                
                if (outcome)
                {
                    bhr |= (int) pow(2, bhr_bits - 1);
                }
            }
        }

        void print_results()
        {
            float mispred_rate = (float) mispreds / (float) total_preds * 100;
            mispred_rate = roundf(mispred_rate * 100) / 100;

            if (silent)
            {
                cout << mispred_rate << endl;
            }
            else
            {
                cout << "OUTPUT" << endl;
                cout << "number of predictions:\t\t" << total_preds << endl;
                cout << "number of mispredictions:\t" << mispreds << endl;
                cout << "misprediction rate:\t\t\t" << fixed << setprecision(2) << mispred_rate << "%" << endl;

                if (bhr_bits > 0)
                {
                    cout << "FINAL GSHARE CONTENTS" << endl;
                }

                else
                {
                    cout << "FINAL BIMODAL CONTENTS" << endl;
                }

                for (int i = 0; i < table.size(); i++)
                {
                    cout << i << "\t\t" << table[i] << endl;
                }
            }
        }
};

int main(int argc, char *argv[])
{
    string pred_type = argv[1];
    string trace_path;
    Smith predictor;
    Gshare gshare_predictor;

    if (pred_type == "smith")
    {
        int counter_bits = stoi(argv[2]);
        trace_path = argv[3];

        if (argc > 4)
        {
            silent = stoi(argv[4]);
        }

        predictor = Smith(counter_bits);
    }

    else if (pred_type == "bimodal")
    {
        int pc_bits = stoi(argv[2]);
        trace_path = argv[3];

        if (argc > 4)
        {
            silent = stoi(argv[4]);
        }

        gshare_predictor = Gshare(pc_bits, 0);
    }

    else if (pred_type == "gshare")
    {
        int pc_bits = stoi(argv[2]);
        int bhr_bits = stoi(argv[3]);
        trace_path = argv[4];

        if (argc > 5)
        {
            silent = stoi(argv[5]);
        }

        gshare_predictor = Gshare(pc_bits, bhr_bits);
    }

    if (!silent)
    {
        cout << "COMMAND" << endl;
        
        for (int i = 0; i < argc - 1; i++)
        {
            cout << argv[i] << " ";
        }

        cout << argv[argc - 1] << endl;
    }

    // read from trace file
    fstream trace_file;

    trace_file.open(trace_path, ios::in);

    if (trace_file.is_open())
    {
        string file_line, address, outcome;
        int outcome_flag;

        while (trace_file >> address >> outcome)
        {
            outcome_flag = (outcome == "t") ? 1 : 0;

            if (pred_type == "smith")
            {
                predictor.predict(outcome_flag);
            }

            else
            {
                // use the generic Gshare predictor
                gshare_predictor.predict(stoi(address, nullptr, 16), outcome_flag);
            }
        }

        if (pred_type == "smith")
        {
            predictor.print_results();
        }
        else
        {
            gshare_predictor.print_results();
        }
    }


    return 0;
}