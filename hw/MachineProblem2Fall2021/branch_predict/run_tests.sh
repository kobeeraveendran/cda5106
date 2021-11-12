#!/bin/bash
make clean
make

smith_bits=(3 1 4)
smith_traces=("gcc_trace.txt" "jpeg_trace.txt" "perl_trace.txt")

gshare_pc_bits=(9 14 11)
gshare_bhr_bits=(3 8 5)
gshare_traces=("gcc_trace.txt" "gcc_trace.txt" "jpeg_trace.txt")

bimodal_bits=(6 12 4)
bimodal_traces=("gcc_trace.txt" "gcc_trace.txt" "jpeg_trace.txt")

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

for i in {0..2};
do
    FILE_IND=$((i+1))
    ./sim gshare ${gshare_pc_bits[$i]} ${gshare_bhr_bits[$i]} ${gshare_traces[$i]} > outputs/gshare_${FILE_IND}.txt
    DIFF=$(diff -iw outputs/gshare_${FILE_IND}.txt ../validation_runs/val_gshare_${FILE_IND}.txt)
    if [[ -n $DIFF ]]
    then
        echo "Gshare Validation ${FILE_IND}: FAIL"
    else
        echo "Gshare Validation ${FILE_IND}: PASS"
    fi
done

for i in {0..2};
do
    FILE_IND=$((i+1))
    ./sim bimodal ${bimodal_bits[$i]} ${bimodal_traces[$i]} > outputs/bimodal_${FILE_IND}.txt
    DIFF=$(diff -iw outputs/bimodal_${FILE_IND}.txt ../validation_runs/val_bimodal_${FILE_IND}.txt)
    if [[ -n $DIFF ]]
    then
        echo "Bimodal Validation ${FILE_IND}: FAIL"
    else
        echo "Bimodal Validation ${FILE_IND}: PASS"
    fi
done
    

# remove generated .o files, sim binaries
rm -f *.o sim