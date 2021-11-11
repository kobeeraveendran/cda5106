#!/bin/bash
make clean
make

smith_bits=(3 1 4)
smith_traces=("gcc_trace.txt" "jpeg_trace.txt" "perl_trace.txt")

# smith validation runs
for i in {0..2};
do
    FILE_IND=$((i+1))
    ./sim smith ${smith_bits[$i]} ${smith_traces[$i]} > outputs/smith_${FILE_IND}.txt
    DIFF=$(diff -iw outputs/smith_${FILE_IND}.txt ../validation_runs/val_smith_${FILE_IND}.txt)
    if [[ -n $DIFF ]]
    then
        echo "Smith Validation ${FILE_IND}: FAIL"
    else
        echo "Smith Validation ${FILE_IND}: PASS"
    fi
done

# remove generated .o files, sim binaries
rm -f *.o sim