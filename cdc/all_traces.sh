gcc -Wall -o cdc.exe CDC_prefetcher.c ../lib/dpc2sim.a

for trace in `ls /dpc2sim/traces` 
do   
    echo $trace             
    zcat ../traces/$trace | ./cdc.exe | tail -n 3 | head -n 1
done