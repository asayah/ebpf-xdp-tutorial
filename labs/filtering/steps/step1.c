/*
The dependencies bellow, will allow us to create our XDP program.
Not all these dependencies are required right now, but we will need them in the next steps. 
*/

#include <linux/bpf.h>
#include <linux/in.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <bpf_helpers.h>
#include <bpf_endian.h>