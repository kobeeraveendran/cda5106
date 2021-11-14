import matplotlib.pyplot as plt
import time
import subprocess
import os


def graph(title, x_label, y_label, x, y, legend_label = None):

    plt.title(title)
    plt.xlabel(x_label)
    plt.ylabel(y_label)

    if not legend_label:
        plt.plot(x, y, marker = 'o')
    else:
        plt.plot(x, y, marker = 'o', label = legend_label)
        plt.legend(title = 'n', loc = "upper right")

    if not legend_label:
        plt.savefig("graph_logs/{}.png".format(title.replace(', ', '_')))
        plt.clf()

if __name__ == "__main__":

    os.system("make clean && make")
    os.makedirs("graph_logs", exist_ok = True)

    # graph 1: n-bit smith counter
    x = [i for i in range(1, 7)]
    y = {
        "gcc_trace.txt": [], 
        "jpeg_trace.txt": [], 
        "perl_trace.txt": []
    }

    g1_count = 0

    for benchmark in y.keys():
        for b in x:
            mispred_rate = subprocess.run(
                ["./sim", "smith", str(b), benchmark, '1'], 
                capture_output = True
            ).stdout

            mispred_rate = float(mispred_rate)
            y[benchmark].append(mispred_rate)

            print("(graph 1 - Smith): b = {} | benchmark = {}\t | misprediction rate: {}".format(b, benchmark, mispred_rate))
            g1_count += 1

        print('-' * 85)

    print("GRAPH 1 (Smith) SIMULATIONS RUN: ", g1_count)

    for benchmark in y.keys():
        benchmark_name = benchmark.split('_')[0].upper()
        graph(benchmark_name + ", Smith", 'b', "Misprediction Rate (%)", x, y[benchmark])

    # graph 2: bimodal predictor
    x = [i for i in range(7, 13)]
    
    for key in y.keys():
        y[key] = []

    g2_count = 0

    for benchmark in y.keys():
        for m in x:
            mispred_rate = subprocess.run(
                ["./sim", "bimodal", str(m), benchmark, '1'], 
                capture_output = True
            ).stdout

            mispred_rate = float(mispred_rate)
            y[benchmark].append(mispred_rate)

            print("(graph 2 - bimodal): m = {}\t | benchmark = {}\t | misprediction rate: {}".format(m, benchmark, mispred_rate))
            g2_count += 1
        
        print('-' * 85)

    print("GRAPH 2 (bimodal) SIMULATIONS RUN: ", g2_count)

    for benchmark in y.keys():
        benchmark_name = benchmark.split('_')[0].upper()
        graph(benchmark_name + ", bimodal", 'm', "Misprediction Rate (%)", x, y[benchmark])

    # graph 3: Gshare predictor
    for key in y.keys():
        y[key] = {}

    g3_count = 0

    for benchmark in y.keys():
        for m in x:
            for n in range(2, m + 1, 2):
                y[benchmark].setdefault(n, [])

                mispred_rate = subprocess.run(
                    ["./sim", "gshare", str(m), str(n), benchmark, '1'], 
                    capture_output = True
                ).stdout

                mispred_rate = float(mispred_rate)
                y[benchmark][n].append(mispred_rate)

                print("(graph 3 - gshare): m = {}\t | n = {}\t | benchmark = {}\t | misprediction rate: {}".format(m, n, benchmark, mispred_rate))
                g3_count += 1

            print('-' * 110)

        print('-' * 110)

    print("GRAPH 3 (Gshare) SIMULATIONS RUN: ", g3_count)

    for benchmark in y.keys():
        benchmark_name = benchmark.split('_')[0].upper()

        for n, vals in y[benchmark].items():
            graph(benchmark_name + ", Gshare", 'm', "Misprediction Rate (%)", x[-len(vals):], vals, n)

        plt.savefig("graph_logs/{}_gshare.png".format(benchmark_name))
        plt.clf()

    os.system("make clean")