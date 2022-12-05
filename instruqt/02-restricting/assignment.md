---
slug: restricting
id: lpv6lncj6g1y
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

1. Restricting traffic using eBPF XDP
==========================

In the following section we will see how we can use eBPF XDP to restrict the traffic coming from a specific IP, this can be a useful was to prevent DDOS attacks for example, when detecting malicious attacks.

Note: at anytime, you can see your configuration under `Configuration` tab.


Let's first start by testing that the `ROUTER` respond to requests from the the `CLIENT` and the `RESTRICTED_CLIENT` containers, run the following code:

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

This means the `CLIENT`, can reach the `ROUTER`.
Let's try the same thing now using the `RESTRICTED_CLIENT`:

```bash
IPV4_ROUTER_ADDR=`docker inspect -f '{{range .NetworkSettings.Networks}}{{.IPAddress}}{{end}}' router`
docker exec restricted-client ping $IPV4_ROUTER_ADDR
```

You should see that the `RESTRICTED-CLIENT` can also reach the `ROUTER` container for now:

```
PING 172.17.0.6 (172.17.0.6) 56(84) bytes of data.
64 bytes from 172.17.0.6: icmp_seq=1 ttl=64 time=0.185 ms
64 bytes from 172.17.0.6: icmp_seq=2 ttl=64 time=0.171 ms
64 bytes from 172.17.0.6: icmp_seq=3 ttl=64 time=0.070 ms
64 bytes from 172.17.0.6: icmp_seq=4 ttl=64 time=0.067 ms
64 bytes from 172.17.0.6: icmp_seq=5 ttl=64 time=0.084 ms
```

We will start by using the same XDP code we wrote in the previous lab, run the following command:

```bash
cp ./tutorial/labs/restricting/steps/step1.c ./tutorial/dist/xdp.c
cp ./tutorial/labs/restricting/xdp.h ./tutorial/dist/xdp.h
```

Note that this time we moved all the dependencies needed to `xdp.h`.

Let's take a look at the `Configuration` tab, we see that this time we added the following in our `xdp.c` file:

```
#define CLIENT 4 // -> 172.17.0.4
#define RESTRICTED_CLIENT 5 // -> 172.17.0.5
```

We are defining statically the IPs of the clients, this information is important to restrict the traffic against these specific IPs.

Now, let's continue and add some code to parse the ip header, run the following command, then check the `Configuration` tab:

```bash
cp ./tutorial/labs/restricting/steps/step2.c ./tutorial/dist/xdp.c
```

At this step, we see now that we have the iph parse, so we can access the source and destination IP of the incoming packets.

Finally, let's write some code to drop all the packets coming from the `RESTRICTED_CLIENT`, run the following command:


```bash
cp ./tutorial/labs/restricting/steps/step2.c ./tutorial/dist/xdp.c
```

You notice now that we added a section to check if the traffic is coming from the `RESTRICTED_CLIENT`, let's test our configuration now.

Run the new XDP code:
```bash
docker exec router make TARGET=./dist/
```

Then:

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

This means the `CLIENT`, can still reach the `ROUTER`.
Let's try the same thing now using the `RESTRICTED_CLIENT`:

```bash
IPV4_ROUTER_ADDR=`docker inspect -f '{{range .NetworkSettings.Networks}}{{.IPAddress}}{{end}}' router`
docker exec restricted-client ping $IPV4_ROUTER_ADDR
```

You should see that the `RESTRICTED-CLIENT` is now restricted and can't reach the `ROUTER` container:

```
PING 172.17.0.6 (172.17.0.6) 56(84) bytes of data.
...
```

Excellent, we secured the `ROUTER` container and restrict the traffic to it.

In the Next lab, we will explore XDP forwarding and create a load balancer, client on `Next` when ready to continue.