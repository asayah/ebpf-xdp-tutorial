#include "xdp.h"

#define IP_ADDRESS(x) (unsigned int)(172 + (17 << 8) + (0 << 16) + (x << 24))
#define RESTRICTED_CLIENT 5 // -> 172.17.0.5

SEC("xdp/route")
int precess_xdp(struct xdp_md *ctx)
{
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

    bpf_printk("Got TCP packet from %x", iph->saddr);

    if (iph->saddr == IP_ADDRESS(RESTRICTED_CLIENT))
    {
        // If the IP of the incoming packet is restricted, drop the packet.
        return XDP_DROP;
    }
    // if the packet is coming from another source, forward it. 
    return XDP_PASS;
}

char _license[] SEC("license") = "GPL";
