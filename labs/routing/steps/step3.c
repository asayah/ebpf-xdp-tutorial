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
	__uint(type, BPF_MAP_TYPE_LRU_HASH);
	__uint(max_entries, MAX_MAP_ENTRIES);
	__type(key, __u32);   // source IPv4 address
	__type(value, __u32); // packet count
} xdp_stats_map SEC(".maps");


SEC("xdp/route")
int precess_xdp(struct xdp_md *ctx)
{
    /*
    We need a key to store data in the map. 
    */
    __u32 ip = 0;
    
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

        /*
        We make a decision based on the count (value), to determine to which target we are going to route. 
        */
        char be = BACKEND_A;
        /*
                if (*value % 2)
            be = BACKEND_B;
          
        */


        __u32 *pkt_count = bpf_map_lookup_elem(&xdp_stats_map, &ip);
        if (!pkt_count) {
            // No entry in the map for this IP address yet, so set the initial value to 1.
            __u32 init_pkt_count = 1;
            bpf_map_update_elem(&xdp_stats_map, &ip, &init_pkt_count, BPF_ANY);
        } else {
            // Entry already exists for this IP address,
            // so increment it atomically using an LLVM built-in.
            __sync_fetch_and_add(pkt_count, 1);
        }

        bpf_printk("Incrementing to %lu\n", *pkt_count);
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
