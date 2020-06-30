//
// Data Prefetching Championship Simulator 2
// Seth Pugsley, seth.h.pugsley@intel.com
//

/*

  gcc -Wall -o nline.exe nline.c ../lib/dpc2sim.a

  zcat ../traces/gcc_trace2.dpc.gz | ./nline.exe | tee result

*/

/*

  This file does NOT implement any prefetcher, and is just an outline

 */

#include <stdio.h>
#include "../inc/prefetcher.h"

#define BUFFER_LEN 2048
#define LEAST_TIMES 20

long long int buffer[BUFFER_LEN];

int is_full;
int head;
int buffer_filled_len;
unsigned long long int previous_addr;
long long int delta;

void l2_prefetcher_initialize(int cpu_num)
{
  printf("No Prefetching\n");
  // you can inspect these knob values from your code to see which configuration you're runnig in
  printf("Knobs visible from prefetcher: %d %d %d\n", knob_scramble_loads, knob_small_llc, knob_low_bandwidth);

  is_full = 0;
  head = 0;
  buffer_filled_len = 0;
  previous_addr = 0;
}

void l2_prefetcher_operate(int cpu_num, unsigned long long int addr, unsigned long long int ip, int cache_hit)
{
  // uncomment this line to see all the information available to make prefetch decisions
  //printf("(0x%llx 0x%llx %d %d %d) ", addr, ip, cache_hit, get_l2_read_queue_occupancy(0), get_l2_mshr_occupancy(0));
  
  // Update buffer
  if (previous_addr==0)
  {
    previous_addr = addr;
    return;
  }
  else
  {
    delta = addr - previous_addr;
  }

  buffer[head] = delta;


  // Update length of buffer
  if (is_full)
  {
    buffer_filled_len = BUFFER_LEN;
  }
  else
  {
    buffer_filled_len = head;
  }

  if (buffer_filled_len==BUFFER_LEN)
  {
    is_full = 1;
  }

  // Find most common
  long long int buffer_copy[buffer_filled_len];
  int freq[buffer_filled_len];
  int i, j, count;

  for (i=0;i<buffer_filled_len;i++)
  {
    buffer_copy[i] = buffer[i];
    freq[i] = -1;
  }

  for(i=0; i<buffer_filled_len; i++)
  {
      count = 1;
      for(j=i+1; j<buffer_filled_len; j++)
      {
          /* If duplicate element is found */
          if(buffer_copy[i]==buffer_copy[j])
          {
              count++;

              /* Make sure not to count frequency of same element again */
              freq[j] = 0;
          }
      }

      /* If frequency of current element is not counted */
      if(freq[i] != 0)
      {
          freq[i] = count;
      }
  }

  // Find the largest freq elements
  int max_freq = freq[0];
  int max_freq_delta = 0;

  for (i=0;i<buffer_filled_len;i++)
  {
    if (freq[i]!=0)
    {
      if (max_freq<freq[i])
      {
        max_freq = freq[i];
        max_freq_delta = buffer_copy[i];
      }
    }
  }

  // Prefetch via largest freq elements
  if (max_freq>=LEAST_TIMES)
  {
    unsigned long long int pf_address = addr + max_freq_delta;

	  // only issue a prefetch if the prefetch address is in the same 4 KB page 
	  // as the current demand access address
	  if((pf_address>>12) != (addr>>12))
    {
      return;
    }

	  // check the MSHR occupancy to decide if we're going to prefetch to the L2 or LLC
	  if(get_l2_mshr_occupancy(0) < 8)
    {
      l2_prefetch_line(0, addr, pf_address, FILL_L2);
    }
	  else
    {
      l2_prefetch_line(0, addr, pf_address, FILL_LLC);
    }
  }

  previous_addr = addr;
  head = (head+1) % BUFFER_LEN;
}

void l2_cache_fill(int cpu_num, unsigned long long int addr, int set, int way, int prefetch, unsigned long long int evicted_addr)
{
  // uncomment this line to see the information available to you when there is a cache fill event
  //printf("0x%llx %d %d %d 0x%llx\n", addr, set, way, prefetch, evicted_addr);

  // 1. Compute delta.
  // 2. Push in stack.
  // 3. Find most common elements in stack.
  // 4. Prefetch.
}

void l2_prefetcher_heartbeat_stats(int cpu_num)
{
  printf("Prefetcher heartbeat stats\n");
}

void l2_prefetcher_warmup_stats(int cpu_num)
{
  printf("Prefetcher warmup complete stats\n\n");
}

void l2_prefetcher_final_stats(int cpu_num)
{
  printf("Prefetcher final stats\n");
}
