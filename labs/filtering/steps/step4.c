#include <linux/bpf.h>
#include <linux/in.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <bpf_helpers.h>
#include <bpf_endian.h>

SEC("xdp/filter")

/*
This filter will process the traffic, and filter out all IPV6 traffic. 
*/
int precess_xdp(struct xdp_md *ctx)
{
    // This first section is to parse the data structure and determine if the packet received are a valid ethernet header (TCP/IP).  
    void *data = (void *)(long)ctx->data;
    void *data_end = (void *)(long)ctx->data_end;
    struct ethhdr *eth = data;
    if (data + sizeof(struct ethhdr) > data_end)
        // If the packet can't be parsed, then error out. 
        return XDP_ABORTED;

    //If it is not IPV6 then proceed 
    if (bpf_ntohs(eth->h_proto) != ETH_P_IPV6)
        return XDP_PASS;
        
    // if IPV6 drop the packet. 
    return XDP_DROP;   
}

char _license[] SEC("license") = "GPL";
