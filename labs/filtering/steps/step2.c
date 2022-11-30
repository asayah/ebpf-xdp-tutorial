#include <linux/bpf.h>
#include <linux/in.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <bpf_helpers.h>
#include <bpf_endian.h>


/*
/* SEC is a helper macro to place programs, maps, license in
 * different sections in elf_bpf file.
 see: https://github.com/torvalds/linux/blob/v4.19/tools/testing/selftests/bpf/bpf_helpers.h#L5-L9
 */


SEC("xdp/filter")
