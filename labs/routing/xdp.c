#include "xdp.h"

/*
In this exercise we are going to route traffic between two different targets, target-A and target-B, this is why we need the IPs.
*/
#define IP_ADDRESS(x) (unsigned int)(172 + (17 << 8) + (0 << 16) + (x << 24))
#define BACKEND_A 2 // -> 172.17.0.2
#define BACKEND_B 3 // -> 172.17.0.3
#define CLIENT 4 // -> 172.17.0.4
#define RESTRICTED_CLIENT 5 // -> 172.17.0.5
#define ROUTER 6 // -> 172.17.0.6

SEC("xdp/route")
int precess_xdp(struct xdp_md *ctx)
{
    void *data = (void *)(long)ctx->data;
    void *data_end = (void *)(long)ctx->data_end;

    bpf_printk("got something");

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

    /*
    In this section we will switch traffic between the two targets.
    we are using (bpf_ktime_get_ns() % 2) to do a 50% chance to select one target or another.
    */
    if (iph->saddr == IP_ADDRESS(CLIENT))
    {
        char be = BACKEND_A;
        if (bpf_ktime_get_ns() % 2)
            be = BACKEND_B;

        iph->daddr = IP_ADDRESS(be);
        eth->h_dest[5] = be;
    }
    else
    /*
    If the source address is not the client, it means it is traffic that needs to be routed back from the ROUTER
    to the CLIENT
    */
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
