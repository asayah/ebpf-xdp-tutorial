---
slug: filtering
id: 5gscmcna9wvi
type: challenge
title: Filtering Traffic using eBPF XDP
teaser: Filtering Traffic using eBPF XDP
tabs:
- title: Terminal
  type: terminal
  hostname: host
- title: Configuration
  type: code
  hostname: kubernetes
  path: /root/tutorial/dist/
difficulty: basic
timelimit: 3600
---

1. The demo environment
==========================
The demo environment consists of a set of docker containers, 2 clients from where we are going to use tools like ping and curl, and two demo backend containers that will just respond with 200 and the name of the service, and finally the most important container called `router`, that we will use to run the eBPF programs.

Take a look a the demo environment using the following command:
```
docker ps --format '{{.Names}}'
```
You should see the following result:

```
router
restricted-client
client
target-B
target-A
```

We can test now that the router pod is reachable through both ipv4 and ipv6, let's check first for ipv4, (172.17.0.6 is the ip of the router container)

```bash
IPV4_ROUTER_ADDR=`docker inspect -f '{{range .NetworkSettings.Networks}}{{.IPAddress}}{{end}}' router`
docker exec client ping $IPV4_ROUTER_ADDR
```

you should see the following result:

```
PING 172.17.0.6 (172.17.0.6) 56(84) bytes of data.
64 bytes from 172.17.0.6: icmp_seq=1 ttl=64 time=0.185 ms
64 bytes from 172.17.0.6: icmp_seq=2 ttl=64 time=0.171 ms
64 bytes from 172.17.0.6: icmp_seq=3 ttl=64 time=0.070 ms
64 bytes from 172.17.0.6: icmp_seq=4 ttl=64 time=0.067 ms
64 bytes from 172.17.0.6: icmp_seq=5 ttl=64 time=0.084 ms
```

You can stop the command after couple seconds using `CTRL+C`.

Now let's test the ping6, to test the IPV6 connectivity:

```bash
IPV6_ROUTER_ADDR=`docker inspect -f '{{range .NetworkSettings.Networks}}{{.GlobalIPv6Address}}{{end}}' router`
docker exec client ping6 $IPV6_ROUTER_ADDR
```

You should expect the following result:

```
PING fd00::242:ac11:6(fd00::242:ac11:6) 56 data bytes
64 bytes from fd00::242:ac11:6: icmp_seq=1 ttl=64 time=0.217 ms
64 bytes from fd00::242:ac11:6: icmp_seq=2 ttl=64 time=0.331 ms
64 bytes from fd00::242:ac11:6: icmp_seq=3 ttl=64 time=0.067 ms
64 bytes from fd00::242:ac11:6: icmp_seq=4 ttl=64 time=0.063 ms

```

At this stage, we can ping the router using it's IPV4 and IPV6 address.


2. Filtering Traffic using eBPF XDP
==========================

In the following section we will use XDP to filter traffic to a destination, XDP operating early in the network stack allow us to filter the traffic fast and early, this is useful when building security tool that filter and allow only a certain type of traffic or a specific protocol.


In the following example, we will create an XDP program that will filter out and drop all IPV6 traffic.
Note: at anytime, you can see your configuration under `Configuration` tab.

first step is to create `.c` file, let's call it `xdp.c`, run the following command:

```
touch ./tutorial/dist/xdp.c
```
Now, we have an `xdp.c` file so we an start writing some code, the first thing we will need is some dependencies, to understand the tcp/ip stack, let's add them to our file:

```
cp ./tutorial/labs/filtering/steps/step1.c ./tutorial/dist/xdp.c
```

Let's take a look a this file, open the `Configuration` tab.
Now that we added the the dependencies, let's add the eBPF SEC macro, it is a helper macro to place programs, maps, license in different sections in elf_bpf file, run the following command:

```
cp ./tutorial/labs/filtering/steps/step2.c ./tutorial/dist/xdp.c
```

Now we can create our first XDP program, an XDP kernel program is a function that takes the connection as argument and returns a code that control to the filter behavior:

XDP_DROP – Drops the packet.
XDP_PASS – Packet will be forwarded.
XDP_TX – Forwards the packet to the same network interface it came from, the packet may be modified.
XDP_REDIRECT – Redirect the packet via another NIC to the network.
XDP_ABORTED - In case of an error

Let's use XDP_DROP, to stop all the traffic, use the following command:

```
cp ./tutorial/labs/filtering/steps/step3.c ./tutorial/dist/xdp.c
```

We see now that we create a simple XDP program that filter out all the traffic, but what we really want it to filter out only the IPV6 traffic.

Let add some code to parse the stack and filter out only the IPV6 traffic:

```bash
cp ./tutorial/labs/filtering/steps/step4.c ./tutorial/dist/xdp.c
```

Great, it is the time now to test if this works, let's run the XDP program, for this, we are using a Makefile that build and attaches the code to the network interface:

```
docker exec router make TARGET=./dist/
```

You should see the following result, it means that the eBPF code is now attached:
```
clang -S \
    -target bpf \
    -D __BPF_TRACING__ \
    -Ilibbpf/src\
    -Wall \
    -Wno-unused-value \
    -Wno-pointer-sign \
    -Wno-compare-distinct-pointer-types \
    -Werror \
    -O2 -emit-llvm -c -o dist/xdp.ll dist/xdp.c
llc -march=bpf -filetype=obj -o dist/xdp.o dist/xdp.ll
bpftool net detach xdpgeneric dev eth0
rm -f /sys/fs/bpf/tutorial_xdp
bpftool prog load ./dist/xdp.o /sys/fs/bpf/tutorial_xdp
libbpf: elf: skipping unrecognized data section(4) .rodata.str1.1
bpftool net attach xdpgeneric pinned /sys/fs/bpf/tutorial_xdp dev eth0
```

Note: if you have an issue with your code, or just want to skip to the solution, you can run the following command:
```
docker exec router make TARGET=./labs/filtering/
```

Let's test now again the IPV6 and IPV:

```bash
IPV4_ROUTER_ADDR=`docker inspect -f '{{range .NetworkSettings.Networks}}{{.IPAddress}}{{end}}' router`
docker exec client ping $IPV4_ROUTER_ADDR
```

you should see the following result:

```
PING 172.17.0.6 (172.17.0.6) 56(84) bytes of data.
64 bytes from 172.17.0.6: icmp_seq=1 ttl=64 time=0.185 ms
64 bytes from 172.17.0.6: icmp_seq=2 ttl=64 time=0.171 ms
64 bytes from 172.17.0.6: icmp_seq=3 ttl=64 time=0.070 ms
64 bytes from 172.17.0.6: icmp_seq=4 ttl=64 time=0.067 ms
64 bytes from 172.17.0.6: icmp_seq=5 ttl=64 time=0.084 ms
```

You can stop the command after couple seconds using `CTRL+C`.

Now let's test the ping6, to test the IPV6 connectivity:

```bash
IPV6_ROUTER_ADDR=`docker inspect -f '{{range .NetworkSettings.Networks}}{{.GlobalIPv6Address}}{{end}}' router`
docker exec client ping6 $IPV6_ROUTER_ADDR
```

You should expect the following result:

```
PING fd00::242:ac11:6(fd00::242:ac11:6) 56 data bytes

```

but if we ping it using ping (IPV4), it will still work:
```bash
IPV4_ROUTER_ADDR=`docker inspect -f '{{range .NetworkSettings.Networks}}{{.IPAddress}}{{end}}' router`
docker exec client ping $IPV4_ROUTER_ADDR
```

you should see the following result:

```
PING 172.17.0.6 (172.17.0.6) 56(84) bytes of data.
64 bytes from 172.17.0.6: icmp_seq=1 ttl=64 time=0.185 ms
64 bytes from 172.17.0.6: icmp_seq=2 ttl=64 time=0.171 ms
64 bytes from 172.17.0.6: icmp_seq=3 ttl=64 time=0.070 ms
64 bytes from 172.17.0.6: icmp_seq=4 ttl=64 time=0.067 ms
64 bytes from 172.17.0.6: icmp_seq=5 ttl=64 time=0.084 ms
```

You can stop the command after couple seconds using `CTRL+C`.

Great! our filter is working, no result should be return back from ping6, this means that we couldn't connect to the router container using its IPV6 filter.


In the next lab we will learn how to block certain IPs using eBPF XDP.

Click on `Next` when ready to continue.