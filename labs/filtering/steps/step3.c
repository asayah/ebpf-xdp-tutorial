#include <linux/bpf.h>
#include <linux/in.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <bpf_helpers.h>
#include <bpf_endian.h>

SEC("xdp/filter")

/*
The function definition takes the connection as an argument, and process run the code that we are attaching to the kernel through XDP.
This function returns an int that is used to determine the XDP behavior:

XDP_DROP – Drops the packet. 
XDP_PASS – Packet will be forwarded.
XDP_TX – Forwards the packet to the same network interface it came from, the packet may be modified.
XDP_REDIRECT – Redirect the packet via another NIC to the network.
XDP_ABORTED - In case of an error
*/


int precess_xdp(struct xdp_md *ctx)
{
    /*
    This first simple filter just drops all the traffic. 
    */
    bpf_printk("Every packet is dropped");

    return XDP_DROP;   
}

char _license[] SEC("license") = "GPL";
