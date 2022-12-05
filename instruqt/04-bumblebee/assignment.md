---
slug: bumblebee
id: yai8zzus6unx
type: challenge
title: Bumblebee
teaser: Prepare our eBPF env
tabs:
- title: Terminal
  type: terminal
  hostname: host
- title: Terminal 2
  type: terminal
  hostname: host
- title: Terminal 3
  type: terminal
  hostname: host
difficulty: basic
timelimit: 3600
---

![BumbleBee logo](https://raw.githubusercontent.com/solo-io/workshops/master/ebpf/images/steps/bumblebee/bumblebee.svg)

[BumbleBee](https://github.com/solo-io/bumblebee) helps to build, run and distribute eBPF programs using OCI images. It allows you to focus on writing eBPF code, while taking care of the user space components - automatically exposing your data as metrics or logs.

Install the `bee` CLI:

```bash
curl -sL https://run.solo.io/bee/install | BUMBLEBEE_VERSION=v0.0.10 sh
export PATH=$HOME/.bumblebee/bin:$PATH
```

Now let's create a skeleton for our eBPF program:

```
bee init
```

The first option you will be confronted with is the language with which you will develop your probe. Currently only `C` is supported, but support for `Rust` is planned as well.

```
? What language do you wish to use for the filter:
  ▸ C
```

Now that we have selected the language to use, we will be prompted to select the type of program you want to create.
As eBPF enables you to write programs that can hook into essentially any kernel functionality, there are several "types" of programs you can create.
`bee` currently has two starting points: network or file-system based programs.
Network programs will be focused on hooking into various functions in the kernel networking stack while file-system programs hook into file operations, such as `open()` calls.
For this tutorial, let's select "Network".

```
? What type of program to initialize:
  ▸ Network
    File system
```

Next you will be asked for the type of global map you would like to use. Maps are the instrument through which `eBPF` user space, and kernel space programs are able to communicate with each other. More detailed information on these maps, as well as the different types of maps which are available can be found in the `eBPF maps` section of the `BPF` [linux documentation](https://man7.org/linux/man-pages/man2/bpf.2.html).
Let's use HashMap.

```
? What type of map should we initialize:
    RingBuffer
  ▸ HashMap
```

After deciding on a map type, you will be asked to decide on an output format.
This step is the first that gets into the detail and magic of `bee`.
Normally developing `eBPF` applications requires writing user space and kernel space code.
However, with `bee` you only need to develop the kernel space code, and then `bee` can automatically handle and output the data from your eBPF maps.
Additionally, `bee` can emit metrics from the data being received by your eBPF maps.
Depending on your use-case, you can simply output the data in your map as text, which corresponds to the `print` output type.
However, if you would like to generate metrics from the data, you can select a metric type.
Currently, `counter` and `gauge` type metrics are supported.
We will be choosing `print` for now, which again will only output map data as text and not emit any metrics.

```
? What type of output would you like from your map:
  ▸ print
    counter
    gauge
```

Finally we will decide on our program file location.

```
✔ BPF Program File Location: probe.c
```

The output file `probe.c` should now have the following content:

```C
#include "vmlinux.h"
#include "bpf/bpf_helpers.h"
#include "bpf/bpf_core_read.h"
#include "bpf/bpf_tracing.h"
#include "solo_types.h"

// 1. Change the license if necessary
char __license[] SEC("license") = "Dual MIT/GPL";

struct dimensions_t {
	// 2. Add dimensions to your value. This struct will be used as the key in the hash map of your data.
	// These will be treated as labels on your metrics.
	// In this example we will have single field which contains the PID of the process
	u32 pid;
} __attribute__((packed));

// This is the definition for the global map which both our
// bpf program and user space program can access.
// More info and map types can be found here: https://www.man7.org/linux/man-pages/man2/bpf.2.html
struct {
	__uint(max_entries, 1 << 24);
	__uint(type, BPF_MAP_TYPE_HASH);
	__type(key, struct dimensions_t);
	__type(value, u64);
} values SEC(".maps.print");

SEC("kprobe/tcp_v4_connect")
int BPF_KPROBE(tcp_v4_connect, struct sock *sk)
{
	// initialize our struct which will be the key in the hash map
	struct dimensions_t key;
	// initialize variable used to track PID of process calling tcp_v4_connect
	u32 pid;
	// define variable used to track the count of function calls, and a pointer to it for plumbing
	u64 counter;
	u64 *counterp;

	// get the pid for the current process which has entered the tcp_v4_connect function
	pid = bpf_get_current_pid_tgid();
	key.pid = pid;

	// check if we have an existing value for this key
	counterp = bpf_map_lookup_elem(&values, &key);
	if (!counterp) {
		// debug log to help see how the program works
		bpf_printk("no entry found for pid: %u}", key.pid);
		// no entry found, so this is the first occurrence, set value to 1
		counter = 1;
	}
	else {
		bpf_printk("found existing value '%llu' for pid: %u", *counterp, key.pid);
		// we found an entry, so let's increment the existing value for this PID
		counter = *counterp + 1;
	}
	// update our map with the new value of the counter
	bpf_map_update_elem(&values, &key, &counter, 0);


	return 0;
}
```

It looks really similar to the BPF program we used with Libbpf, right?

Let's now see how the same `tcpconnect` BPF programm we used with Libbpf would look like with BumbleBee.

Run the following command to see the differences:

```
diff data/steps/bumblebee/tcpconnect.c data/steps/bumblebee/libbpf-tcpconnect.bpf.c
```

Here is the expected output:

```
28c28
< } events_hash SEC(".maps.counter");
---
> } events_hash SEC(".maps");
```

Amazing, no?

So, you just need to add the `.counter` suffix to let BumbleBee know how to interpret it on the user-space!

Let's copy the updated version:

```bash
cp data/steps/bumblebee/tcpconnect.c ./tcpconnect.c
```

Use the bee tool to compile your program and store it as an OCI image:

```bash
bee build tcpconnect.c localhost:5000/solo/tcpconnect:v1
```

Push the OCI image in the local Docker registry:
<!--bash
while ! curl localhost:5000; do
	sleep 1
done
-->
```bash
bee push localhost:5000/solo/tcpconnect:v1
```

It's nice, no?

You can push and pull the BumbleBee programs to/from any OCI compliant registry (like the private Docker registry running in this machine).

Finally, you can run your program:

```
bee run localhost:5000/solo/tcpconnect:v1
```

![BumbleBee tcpconnect](https://raw.githubusercontent.com/solo-io/workshops/master/ebpf/images/steps/bumblebee/tcpconnect.png)

You can execute any curl command in the second terminal to see new entries displayed in the BumbleBee UI.

Now, run the following command in a different tab while the program is running:

```
curl http://localhost:9091/metrics
```

You should have an output similar to the following:

```
# HELP ebpf_solo_io_events_hash
# TYPE ebpf_solo_io_events_hash counter
ebpf_solo_io_events_hash{daddr="10.101.0.2",saddr="10.101.0.1"} 45
ebpf_solo_io_events_hash{daddr="10.101.0.4",saddr="10.101.0.1"} 49
ebpf_solo_io_events_hash{daddr="10.101.1.2",saddr="10.101.1.1"} 47
...
# HELP ebpf_solo_io_events_ring
# TYPE ebpf_solo_io_events_ring counter
ebpf_solo_io_events_ring{daddr="10.101.0.2",saddr="10.101.0.1"} 50
ebpf_solo_io_events_ring{daddr="10.101.0.4",saddr="10.101.0.1"} 50
ebpf_solo_io_events_ring{daddr="10.101.1.2",saddr="10.101.1.1"} 50
...

# HELP promhttp_metric_handler_requests_total Total number of scrapes by HTTP status code.
# TYPE promhttp_metric_handler_requests_total counter
promhttp_metric_handler_requests_total{code="200"} 0
promhttp_metric_handler_requests_total{code="500"} 0
promhttp_metric_handler_requests_total{code="503"} 0
```

As you can see, BumbleBee also provides metrics in the Prometheus format!

In the next labs, we're going to run this eBPF program on Kubernetes as a DaemonSet to capture the network traffic on all the nodes.

Then, we're going to scrape the metrics in Prometheus.

And finally, we're going to run a small web application which will query both Prometheus and the Kubernetes API server to build a network graph.
