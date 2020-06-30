#!/usr/bin/python3

import re
import sys, os

def modify_source(buffer_len, least_times):
    with open('nline.c', 'r') as file:
        text = file.read()
    
    text = re.sub(r'#define BUFFER_LEN 1024\n', r'#define BUFFER_LEN {}\n'.format(buffer_len), text)
    text = re.sub(r'#define LEAST_TIMES 50\n', r'#define LEAST_TIMES {}\n'.format(least_times), text)

    new_name = 'nline_bl{}_lt{}.c'.format(buffer_len, least_times)

    with open(new_name, 'w') as file:
        file.write(text)

    return new_name

if __name__ == '__main__':
    for buffer_len in [512, 1024, 2048]:
        for least_times in [20, 50, 100]:
            new_name = modify_source(buffer_len, least_times)
            os.system(
                '''
                    gcc -Wall -o nline_{0}.exe {0}.c ../lib/dpc2sim.a;  
                    echo {0};               
                    zcat ../traces/gcc_trace2.dpc.gz | ./nline_{0}.exe | tail -n 3 | head -n 1
                '''.format(new_name[:-2])
                )
