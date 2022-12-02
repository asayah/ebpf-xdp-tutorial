
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
   
   /*
   In the following section we are parsing iph, to get the the source ip of the incoming packet. 
   */
    struct iphdr *iph = data + sizeof(struct ethhdr);
    if (data + sizeof(struct ethhdr) + sizeof(struct iphdr) > data_end)
        // If not a valid iph, we abort. 
        return XDP_ABORTED;



    return XDP_PASS;
}

char _license[] SEC("license") = "GPL";
