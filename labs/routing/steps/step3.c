#include "xdp.h"

#define IP_ADDRESS(x) (unsigned int)(172 + (17 << 8) + (0 << 16) + (x << 24))
#define CLIENT 4 // -> 172.17.0.4
#define ROUTER 6 // -> 172.17.0.6
#define BACKEND_A 2 // -> 172.17.0.2
#define BACKEND_B 3 // -> 172.17.0.3


/*
Let's define a map to store the number of of times the router was used to load balance, and we will use this information to build a strict round robin.
*/


struct {
        __uint(type, BPF_MAP_TYPE_HASH);
        __type(key, __u32);
        __type(value, long);
        __uint(max_entries, 1);
} rxcnt SEC(".maps");



SEC("xdp/route")
int precess_xdp(struct xdp_md *ctx)
{
    /*
    We need a key to store data in the map. 
    */
    __u32 key = 0;
    long *value;
    
    void *data = (void *)(long)ctx->data;
    void *data_end = (void *)(long)ctx->data_end;

    struct ethhdr *eth = data;
    if (data + sizeof(struct ethhdr) > data_end)
        return XDP_ABORTED;

    if (bpf_ntohs(eth->h_proto) != ETH_P_IP)
        return XDP_PASS;

    struct iphdr *iph = data + sizeof(struct ethhdr);
    if (data + sizeof(struct ethhdr) + sizeof(struct iphdr) > data_end)
        return XDP_ABORTED;

    if (iph->protocol != IPPROTO_TCP)
        return XDP_PASS;

    bpf_printk("Got TCP packet from %x", iph->saddr);

    if (iph->saddr == IP_ADDRESS(CLIENT))
    {
        /*
        This is how we retrieve a value from the map.
        We are incrementing the value each time we are going to load balance. 
        */
        value = bpf_map_lookup_elem(&rxcnt, &key);
        if (value)
                *value += 1;
        /*
        We make a decision based on the count (value), to determine to which target we are going to route. 
        */
        char be = BACKEND_A;

        /*
        if (value % 2)
            be = BACKEND_B;
        */    
        iph->daddr = IP_ADDRESS(be);
        eth->h_dest[5] = be;
    }
    else

    {
        iph->daddr = IP_ADDRESS(CLIENT);
        eth->h_dest[5] = CLIENT;
    }
    iph->saddr = IP_ADDRESS(ROUTER);
    eth->h_source[5] = ROUTER;
    iph->check = iph_csum(iph);

    return XDP_TX;
}

char _license[] SEC("license") = "GPL";
