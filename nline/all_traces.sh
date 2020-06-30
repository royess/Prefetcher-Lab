gcc -Wall -o nline.exe nline.c ../lib/dpc2sim.a

for trace in `ls /dpc2sim/traces` 
do   
    echo $trace             
    zcat ../traces/$trace | ./nline.exe | tail -n 3 | head -n 1
done