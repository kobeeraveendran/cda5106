import matplotlib.pyplot as plt
import time
import subprocess
import os

def graph(title, x_label, y_label, x, y, legend_names = None):

    plt.title(title)
    plt.xlabel(x_label)
    plt.ylabel(y_label)

    if not legend_names:
        plt.plot(x, y)

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

    os.system("make clean")