#!/bin/bash

for cfile in `ls ../example_prefetchers`
do
    echo $cfile
    gcc -Wall -o ${cfile:0:-2}_sim.o ../example_prefetchers/$cfile ../lib/dpc2sim.a
    zcat ../traces/gcc_trace2.dpc.gz | ./${cfile:0:-2}_sim.o
    echo
done