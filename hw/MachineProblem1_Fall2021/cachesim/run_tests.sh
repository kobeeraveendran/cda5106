#!/bin/bash
make clean
make

./sim_cache 16 1024 2 0 0 0 0 gcc_trace.txt > output.txt
DIFF=$(diff -iw output.txt ../validation_runs/validation0.txt)
if [[ -n $DIFF ]]
then
    echo "Validation 0: FAIL"
else
    echo "Validation 0: PASS"
fi

./sim_cache 16 1024 1 0 0 0 0 perl_trace.txt > output.txt
DIFF=$(diff -iw output.txt ../validation_runs/validation1.txt)
if [[ -n $DIFF ]]
then
    echo "Validation 1: FAIL"
else
    echo "Validation 1: PASS"
fi

./sim_cache 16 1024 2 0 0 1 0 gcc_trace.txt > output.txt
DIFF=$(diff -iw output.txt ../validation_runs/validation2.txt)
if [[ -n $DIFF ]]
then
    echo "Validation 2: FAIL"
else
    echo "Validation 2: PASS"
fi

./sim_cache 16 1024 2 0 0 2 0 vortex_trace.txt > output.txt
DIFF=$(diff -iw output.txt ../validation_runs/validation3.txt)
if [[ -n $DIFF ]]
then
    echo "Validation 3: FAIL"
else
    echo "Validation 3: PASS"
fi

./sim_cache 16 1024 2 8192 4 0 0 gcc_trace.txt > output.txt
DIFF=$(diff -iw output.txt ../validation_runs/validation4.txt)
if [[ -n $DIFF ]]
then
    echo "Validation 4: FAIL"
else
    echo "Validation 4: PASS"
fi

./sim_cache 16 1024 1 8192 4 0 0 go_trace.txt > output.txt
DIFF=$(diff -iw output.txt ../validation_runs/validation5.txt)
if [[ -n $DIFF ]]
then
    echo "Validation 5: FAIL"
else
    echo "Validation 5: PASS"
fi

./sim_cache 16 1024 2 8192 4 0 1 gcc_trace.txt > output.txt
DIFF=$(diff -iw output.txt ../validation_runs/validation6.txt)
if [[ -n $DIFF ]]
then
    echo "Validation 6: FAIL"
else
    echo "Validation 6: PASS"
fi

# remove generated .o files, sim_cache binaries, and output.txt file
rm -f *.o sim_cache