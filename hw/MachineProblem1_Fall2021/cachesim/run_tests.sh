#!/bin/bash
make clean
make

blockSize=16
l1Size=1024
l1Assoc=(2 1 2 2 2 1 2 1)
l2Size=(0 0 0 0 8192 8192 8192 8192)
l2Assoc=(0 0 0 0 4 4 4 4)
replacement=(0 0 1 2 0 0 0 0)
inclusion=(0 0 0 0 0 0 1 1)
traceFile=("gcc_trace.txt" "perl_trace.txt" "gcc_trace.txt" "vortex_trace.txt" \
           "gcc_trace.txt" "go_trace.txt" "gcc_trace.txt" "compress_trace.txt")

for i in {0..7};
do
    ./sim_cache ${blockSize} ${l1Size} ${l1Assoc[$i]} ${l2Size[$i]} ${l2Assoc[$i]} \
    ${replacement[$i]} ${inclusion[$i]} ${traceFile[$i]} > outputs/output${i}.txt
    DIFF=$(diff -iw outputs/output${i}.txt ../validation_runs/validation${i}.txt)
    if [[ -n $DIFF ]]
    then
        echo "Validation ${i}: FAIL"
    else
        echo "Validation ${i}: PASS"
    fi
done

# remove generated .o files, sim_cache binaries, and output.txt file
rm -f *.o sim_cache