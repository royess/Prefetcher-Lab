//
// Data Prefetching Championship Simulator 2
// Seth Pugsley, seth.h.pugsley@intel.com
//

/*

  This file does NOT implement any prefetcher, and is just an outline

 */

#include <stdio.h>
#include "../inc/prefetcher.h"

#define GHB_LENGTH 256
#define PREFETCH_DEGREE 6

unsigned long long int global_history_buffer[GHB_LENGTH];
long long int delta_buffer[GHB_LENGTH];
long long int comparison_register[2];
long long int key_register[2];

int head; // The index in GHB we are dealing with

void l2_prefetcher_initialize(int cpu_num)
{
  printf("C/DC prefetching\n");
  // you can inspect these knob values from your code to see which configuration you're runnig in
  printf("Knobs visible from prefetcher: %d %d %d\n", knob_scramble_loads, knob_small_llc, knob_low_bandwidth);

  // Set counter to 0
  head = 0;

  // Set the value of all pointers to NULL, all addr to 0.
  int i = 0;

  for (i=0;i<GHB_LENGTH;i++)
  { 
    global_history_buffer[i] = 0;
  }
}

void l2_prefetcher_operate(int cpu_num, unsigned long long int addr, unsigned long long int ip, int cache_hit)
{
  if (cache_hit) return;

  global_history_buffer[head] = addr;

  // Collect addr with same tag in GHB
  unsigned long long int same_tag_list[GHB_LENGTH];
  int i, j;
  int k = 0;

  for (i=0;i<GHB_LENGTH;i++)
  {
    j = (head-i) % GHB_LENGTH;
    if (global_history_buffer[j]==0) break;
    if (global_history_buffer[j]/4096==addr/4096)
    {
      same_tag_list[k]=global_history_buffer[j];
      k++; // When loop ends, k will be length of same_tag_list.
    }
  }

  // Flags
  int comparison_register_is_full = 0;
  int key_register_is_full = 0;
  int if_constant_strides = 0;
  int hit = 0;

  long long int delta;
  int delta_buffer_index = 0;

  long long int prefetch_delta_list[PREFETCH_DEGREE];

  // This for loop is used for aquiring 'prefetch_delta_list'
  for (i=0;i<k-1;i++)
  {
    delta = same_tag_list[i] - same_tag_list[i+1];

    // Fill registers
    switch (i)
    {
      case 0:
        key_register[0] = delta;
        break;
      case 1:
        key_register[1] = delta;
        if (key_register[0]==key_register[1])
        {
          if_constant_strides = 1;
        }
        key_register_is_full = 1;
        break;
      case 2:
        comparison_register[0] = delta;
        break;
      case 3:
        comparison_register[1] = delta;
        comparison_register_is_full = 1;
        break;
      default:
        comparison_register[0] = comparison_register[1];
        comparison_register[1] = delta;
    }

    // Fill buffer
    delta_buffer[delta_buffer_index] = delta;
    delta_buffer_index++;

    // If detecting constant strides
    if (if_constant_strides)
    {
      for (j=0;j<PREFETCH_DEGREE;j++)
      {
        prefetch_delta_list[j] = delta;
      }
      hit = 1;
      break;
    }

    // If correlation hit
    if (comparison_register_is_full)
    {
      if(key_register[0]==comparison_register[0] && key_register[1]==comparison_register[1])
      {
        for (j=0;j<PREFETCH_DEGREE;j++)
        {
          prefetch_delta_list[j] = delta_buffer[(delta_buffer_index-3 - j % (delta_buffer_index-2))]; 
        }
        hit = 1;
        break;
      }
    }
  }

  // Prefetch
  unsigned long long int pf_address;

  if (hit)
  {
    //printf("HIT!")
    for (j=0;j<PREFETCH_DEGREE;j++)
    {
      pf_address = addr + prefetch_delta_list[j];

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
  }

  head = (head+1) % GHB_LENGTH;
}

void l2_cache_fill(int cpu_num, unsigned long long int addr, int set, int way, int prefetch, unsigned long long int evicted_addr)
{
  // uncomment this line to see the information available to you when there is a cache fill event
  //printf("0x%llx %d %d %d 0x%llx\n", addr, set, way, prefetch, evicted_addr);
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
