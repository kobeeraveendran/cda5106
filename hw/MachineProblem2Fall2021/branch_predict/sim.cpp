#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <cmath>

using namespace std;

class Smith
{
    private:
        int mispreds = 0;
        int count, max_count;

    public:

        Smith(){}

        Smith(int num_bits)
        {
            count = pow(2, num_bits - 1);
            max_count = count * 2 - 1;
        }

        int predict(int outcome)
        {
            int prediction;

            if (count > max_count / 2)
            {
                // over the boundary; predict taken
                prediction = 1;
            }
            else
            {
                // under the boundary; predict not-taken
                prediction = 0;
            }

            // update counter accordingly

            // if we actually took the branch, increment
            if (outcome == 1)
            {
                count = min(count + 1, max_count);
            }
            else
            {
                count = max(count - 1, 0);
            }

            return prediction;
        }
};

int main(int argc, char *argv[])
{
    string pred_type = argv[1];
    string trace_path;
    Smith predictor;

    if (pred_type == "smith")
    {
        int counter_bits = stoi(argv[2]);
        trace_path = argv[3];
        predictor = Smith(counter_bits);
    }

    else if (pred_type == "bimodal")
    {
        int pc_bits = stoi(argv[2]);
        trace_path = argv[3];
    }

    else if (pred_type == "gshare")
    {
        int pc_bits = stoi(argv[2]);
        int gbh_bits = stoi(argv[3]);
        trace_path = argv[4];
    }

    cout << "COMMAND" << endl;
    
    for (int i = 0; i < argc - 1; i++)
    {
        cout << argv[i] << " ";
    }

    cout << argv[argc - 1] << endl;

    cout << "OUTPUT" << endl;

    // read from trace file
    fstream trace_file;

    trace_file.open(trace_path, ios::in);

    if (trace_file.is_open())
    {
        string file_line, address, outcome;

        while (trace_file >> address >> outcome)
        {
            predictor.predict(stoi(outcome));
        }
    }


    return 0;
}