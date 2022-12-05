---
slug: routing
id: qkuzlcknflrr
type: challenge
title: Routing Traffic Using eBPF XDP
teaser: Routing Traffic Using eBPF XDP
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

XDP is used to accelerate the routing in multiple technologies, like Cilium for example, in the following example we will explore how to load balance traffic between two different endpoint target.

Let's first test the connectivity to our target services, run the following command:

```bash
IPV4_TARGET_A_ADDR=`docker inspect -f '{{range .NetworkSettings.Networks}}{{.IPAddress}}{{end}}' target-A`
docker exec client curl $IPV4_TARGET_A_ADDR
```

We expect the following response for `TARGET-A` service:

```
...
Server address: 172.17.0.2:80
Server name: target-a
Date: 02/Dec/2022:13:23:35 +0000
URI: /
Request ID: e8edaac3dfecdc7d01a9797374844aec
```

Note: `TARGET-B` should respond with `target-b` instead in the response body when called.


Let's write some code now to load balance the traffic between `TARGET-A` and `TARGET-B`, we will start with the code we wrote previously:

```bash
cp ./tutorial/labs/routing/steps/step1.c ./tutorial/dist/xdp.c
```

You will notice that it is exactly the same, we just added the addresses of the sources and target statically:

```
#define CLIENT 4 // -> 172.17.0.4
#define ROUTER 6 // -> 172.17.0.6
#define BACKEND_A 2 // -> 172.17.0.2
#define BACKEND_B 3 // -> 172.17.0.3
```

The second part is to add the load balancing:
```bash
cp ./tutorial/labs/routing/steps/step2.c ./tutorial/dist/xdp.c
```

For this we will check if the packet is coming from a specific source, in our case `CLIENT`, then we will forward the traffic to the `TARGET-A` or `TARGET-B` on 50% probability.

For the traffic coming back from the `TARGET` we need to send it back to the `CLIENT`.

Let's run the code now:

```
docker exec router make TARGET=./dist/
```

And now let's call the `ROUTER` from the `CLIENT` pod:

```bash
IPV4_ROUTER_ADDR=`docker inspect -f '{{range .NetworkSettings.Networks}}{{.IPAddress}}{{end}}' router`
docker exec client curl $IPV4_ROUTER_ADDR
```

You should see:

```

...
Server address: 172.17.0.2:80
Server name: target-a
Date: 02/Dec/2022:13:23:35 +0000
URI: /
Request ID: e8edaac3dfecdc7d01a9797374844aec

...
Server address: 172.17.0.2:80
Server name: target-b
Date: 02/Dec/2022:13:23:35 +0000
URI: /
Request ID: e8edaac3dfecdc7d01a9797374844aec

...
Server address: 172.17.0.2:80
Server name: target-b
Date: 02/Dec/2022:13:23:35 +0000
URI: /
Request ID: e8edaac3dfecdc7d01a9797374844aec

...
Server address: 172.17.0.2:80
Server name: target-a
Date: 02/Dec/2022:13:23:35 +0000
URI: /
Request ID: e8edaac3dfecdc7d01a9797374844aec
```

Or something similar, this means that when calling the `ROUTER` we are successfully load balancing between `TARGET-A` and `TARGET-B`.


Great!, we created an XDP load balancer, but this can be optimized, in a real world scenario, the IPs of the client should be passed dynamically for to the kernel space so that the XDP program can process them them, and for all the applications we wrote today we should create also a corresponding user space program to control the XDP code and provide user input.

here is an example of an eBPF program and it's user space program:
see: [User space program example](https://github.com/iovisor/bcc/blob/master/libbpf-tools/oomkill.c).


Creating a user space program can be really complicated, this is why the Bumblebee project was created, it is at tool to simplify and speed-up the creating of eBPF programs, in the following lab we will learn more about it.