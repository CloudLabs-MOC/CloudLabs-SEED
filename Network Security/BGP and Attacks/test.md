##                                                             BGP Exploration and Attack Lab
                                                            
Copyright © 2021 by Wenliang Du.

This work is licensed under a Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License. If you remix, transform, or build upon the material, this copyright notice must be left intact, or reproduced in a way that is reasonable to the medium in which the work is being re-published.

## 1 Overview

Border Gateway Protocol (BGP) is the standard exterior gateway protocol designed to exchange routing and
reachability information among autonomous systems (AS) on the Internet. It is the “glue” of the Internet,
and is an essential piece of the Internet infrastructure. It is also a primary attack target, because if attackers
can compromise BGP, they can disconnect the Internet and redirect traffics.

The goal of this lab is to help students understand how BGP “glues” the Internet together, and how the
Internet is actually connected. We have built an Internet emulator, and will use this emulator as the basis
for the lab activities. Due to the complexity of BGP, the explanation of how BGP works is provided in a
separate document. It is essential for the lab activities, so students should read the document before working
on the lab. This lab covers the following topics:

• How the BGP protocol works

• Autonomous systems

• BGP configuration, BGP Large Communities

• Routing

• Internet Exchange (IX)

• BGP attack, network prefix hijacking

## Supporting materials. 
BGP is quite complicated, especially its practice side. To help students work on
this lab, I have written a chapter on BGP, which will be included in the 3rd edition of my book. I will make
this chapter a sample chapter, so it is free for everybody to download. The link to this chapter can be found
from the web page of this lab. Without reading this chapter or covering it in the lecture, it will be quite hard
for students to work on this lab.

## Note for instructors. 
Tasks 1 to 4 are designed to help students understand the technical details of BGP,
it is for the instructors who do cover BGP in-depth in their classes. Only Task 5, BGP attacks, is related
to security, so if instructors only want students to focus on the security aspect of BGP, they can skip Tasks
1 - 4, and assign Task 5 directly to students, as this task does not depend on the earlier tasks. High-level
knowledge on BGP should be sufficient for Task 5.

Lab environment. You can perform the lab exercise on the SEED VM provided by the Cloudlabs.

##                      The Lab Setup and the SEED Internet Emulator

This lab will be performed inside the SEED Internet Emulator (simply called the emulator in this document).
We provide a pre-built emulator in two different forms: Python code and container files. The container files
SEED Labs – BGP Exploration and Attack 2
are generated from the Python code, but students need to install the SEED Emulator source code from
the GitHub to run the Python code. The container files can be directly used without the emulator source
code. Instructors who would like to customize the emulator can modify the Python code, generate their own
container files, and then provide the files to students, replacing the ones included in the lab setup file.

## Download the emulator files 

Download the emulator files. Files needed for this lab are included in Labsetup.zip, which can be fetched by running the following commands.

```
sudo wget https://github.com/CloudLabs-MOC/CloudLabs-SEED/blob/main/Network%20Security/BGP%20and%20Attacks/Lab%20files/Labsetup.zip
```
```
sudo unzip Labsetup.zip
```
## Start the emulation

We will directly use the container files in the output folder. Go to this folder,
and run the following docker commands to build and start the containers. We recommend that you run the
emulator inside the provided SEED Ubuntu 20.04 VM, but doing it in a generic Ubuntu 20.04 operating
system should not have any problem, as long as the docker software is installed. Readers can find the docker
manual from  this link https://github.com/seed-labs/seed-labs/blob/master/manuals/docker/SEEDManual-Container.md

$ docker-compose build

$ docker-compose up

// Aliases for the Compose commands above (only available in the SEED VM)

$ dcbuild # Alias for: docker-compose build

$ dcup # Alias for: docker-compose up

$ dcdown # Alias for: docker-compose down

## The Network Map

Each computer (hosts or routers) running inside the emulator is a docker container. Users can access these
computers using docker commands, such as getting a shell inside a container. The emulator also comes
with a web application, which visualizes all the hosts, routers, and networks. After the emulator starts, the
map can be accessed from this URL: http://localhost:8080/map.html. See Figure 1. Users can
interact with this map, such as getting a terminal from any of the container, disabling BGP sessions (see
Figure 2). Users can also set filters to visualize network traffic. The syntax of the filter is the same as that in
tcpdump; actually, the filter is directly fed into the tcpdump program running on all nodes.

## Modifying the BGP Configuration File

We need to modify the BGP configuration file in several tasks. We can do that by directly modifying the
configuration file inside a container. Anther way is to copy the file into the host VM, do the editing from
the host VM, and then copy it back. Let us see an example (assuming that we want to modify the BGP
configuration file of AS-180):

// Find out the IP of the AS-180’s BGP router container

$ dockps | grep 180

6bf0bcda8d06 as180h-host_1-10.180.0.72

437874056b15 as180h-webservice_0-10.180.0.71

29676d5034ce as180r-router0-10.180.0.254 ➙This is AS-180’s BGP router

// Copy the configuration file from the container to the host machine

$ docker cp 2967:/etc/bird/bird.conf ./as180_bird.conf

![image](https://user-images.githubusercontent.com/131140229/236833894-86b1396b-6c2c-4346-9df3-ff18d16cd728.png)

... edit the file ...

// Copy the file back to the container

$ docker cp ./as180_bird.conf 2967:/etc/bird/bird.conf

// Reload the configuration on the container

$ docker exec 2967 birdc configure ➙Run "birdc configure"

BIRD 2.0.7 ready.

Reading configuration from /etc/bird/bird.conf

Reconfigured

## Convention used in the Emulator
To make it easy to identify the roles of each node in the emulator, we have created a set of conventions when
assigning various numbers to nodes. These conventions are only for the emulator, and they do not hold in
the real world.

• Autonomous System Number (ASN) assignment:

– ASN 2 - 9: for large transit ASes (e.g., national backbone).

– ASN 10 - 19: for smaller transit ASes.

– ASN 100 - 149: for Internet Exchanges (IX).

– ASN 150 - 199: for stub ASes.

• Network prefixes and IP addresses:

![image](https://user-images.githubusercontent.com/131140229/236834270-ab9e70e5-179a-455c-a48c-f5ace2b76757.png)

– For an autonomous system N, its first internal network’s prefix is 10.N.0.0/24, the second
internal network is 10.N.1.0/24, and so on.

– In each network, the address 200 to 255 are for routers. For hosts (non-router), their IP
address start from 71. For example, in AS-155, 10.155.0.255 is a BGP router, while
10.155.0.71 is a host.

## Task 1: Stub Autonomous System

An autonomous system (AS) is a collection of connected Internet Protocol (IP) routing prefixes under the
control of one or more network operators on behalf of a single administrative entity or domain. It is a basic
unit in BGP. A stub AS the type of AS that does not provide transit service to others. Most of end users are
stub ASes, including universities, organization, and most companies. Another type of AS is called transit
AS. They provide transit services for other ASes, and they are Internet service providers.

In this task, we focus on stub ASes, see how it peers with others. For this type of ASes, we will gain a
glimpse of how BGP works. Students should read Sections 1- 7 before working on this task

## Task 1.a: Understanding AS-155’s BGP Configuration

AS-155 is a stub AS, which has one network (10.155.0.0/24) and one BGP router (10.155.0.254).
Please get a terminal on the container of this router, study its BGP configuration in /etc/bird/bird.
conf, and then do the following tasks.

## Task 1.a.1:

From the BGP configuration file, identify who AS-155 peers with. You can ignore the
filtering part of the configuration for now. Here is one of the BGP entries in the configuration file. See
Section 6 of the provided BGP tutorial for the explanation of each entry

protocol bgp u_as2 {

ipv4 {

table t_bgp;

import filter {

SEED Labs – BGP Exploration and Attack 5

bgp_large_community.add(PROVIDER_COMM);

bgp_local_pref = 10;

accept;

};

export where bgp_large_community ˜ [LOCAL_COMM, CUSTOMER_COMM];

next hop self;

};

local 10.102.0.155 as 155;

neighbor 10.102.0.2 as 2;

}

## Task 1.a.2: 

AS-155 peers with several ASes, so if AS-155 loses the connection with one of them,
it can still connect to the Internet. Please design an experiment to demonstrate this. You can enable/disable BGP sessions either from the graph (see Figure 2) or using the birdc command (see the
following examples). In your experiment, please show how the routing table changes when a particular BGP session is disabled/enabled (you can use the "ip route" to see the content of a routing
table).

root@0c97d3ade85a / # birdc show protocols

![image](https://user-images.githubusercontent.com/131140229/236835627-e68486ab-3e6e-4805-845d-97b631b44985.png)

root@0c97d3ade85a / # birdc disable u_as2 ➙Disable peering with AS-2

![image](https://user-images.githubusercontent.com/131140229/236835767-96478b00-5711-46a7-965d-c6fe42d9b86d.png)

root@0c97d3ade85a / # birdc show protocols

![image](https://user-images.githubusercontent.com/131140229/236835885-6a4f2e73-1fea-455d-bf36-d1691366d883.png)


## Task 1.b: Observing BGP UPDATE Messages

The objective of this task is to understand the BGP UPDATE messages. Run tcpdump on AS-150’s
BGP router, use it to monitor the BGP traffic. This command will save the captured BGP packets into
/tmp/bgp.pcap

# tcpdump -i any -w /tmp/bgp.pcap "tcp port 179"

Your job is to do something on AS-155’s BGP router to trigger at least one BGP route withdrawal and
one BGP route advertisement messages. These UPDATE messages should be captured by the tcpdump
command on AS-150 and stored inside bgp.pcap. Copy this file to the host computer using the "docker
cp" command (from the host), and then load it into Wireshark. Pick a route advertisement message and a
route withdraw message, provide explanation on these two messages. Screenshots should be provided in the
lab report

 ## Task 1.c: Experimenting with Large Communities
 
When a BGP router sends routes to its peers, they do not send all the routes they know. What routes are
sent depends on many factors, such as the region of the peers, the business relationship between the peers,
and policies. To help BGP routers make such decisions, additional information needs to be attached to each
route, as the predefined set of route attributes cannot capture such information. The BGP Large Communities
are created to serve this goal. The objective of this task is to learn how it is used in the emulator to reflect
the business relationship among peers.

Let us assume that due to some technical issue, the connection between AS-4 and AS-156 is broken.
We can emulate this by disabling the peering between AS-4 and AS-156. Since AS-4 is the only service
provider for AS-156, this essentially disconnects AS-156 from the Internet. If we ping another host from
one of the hosts in AS-156, we can see the following results (please do not run ping from the BGP router;
only run it from a host):

// On 10.156.0.72

# ping 10.155.0.71

PING 10.155.0.71 (10.155.0.71) 56(84) bytes of data.

64 bytes from 10.155.0.71: icmp_seq=1 ttl=62 time=14.6 ms

64 bytes from 10.155.0.71: icmp_seq=2 ttl=62 time=0.363 ms

# ping 10.161.0.71

PING 10.161.0.71 (10.161.0.71) 56(84) bytes of data.

From 10.156.0.254 icmp_seq=1 Destination Net Unreachable

From 10.156.0.254 icmp_seq=2 Destination Net Unreachable

From 10.156.0.254 icmp_seq=3 Destination Net Unreachable

We can see that 10.155.0.71 is still reachable, because it belongs to AS-155, which is still peered
with AS-156. However, 10.161.0.71 (belonging to AS-161) cannot be reached, because nobody will
route the packet for AS-156. The question is, AS-156 still peers with AS-155, which is connected to the
Internet, so why is AS-156 not able to connect to the Internet? This is because whether an AS forwards
traffic for another AS depends on their business relationship.

While AS-156 and AS-4 are trying to solve the problem, AS-156 holds an emergency meeting with AS155, agreeing to pay AS-155, so its traffic can temporarily go through AS-155 to reach the Internet. This
requires some changes on AS-155’s BGP router. Please make such changes, so AS-155 can temporarily
provide a transit service to AS-156. Please read Section 9 before working on this task. After making the
changes, please make sure run the following command to reload the BIRD configuration.

# birdc configure

BIRD 2.0.7 ready.

Reading configuration from /etc/bird/bird.conf

Reconfigured

## Task 1.d: Configuring AS-180
AS-180 is already included in the emulator. It connects to the IX-105 Internet exchange (Houston), but it
does not peer with anybody, so it is not connected to the Internet. In this task, students need to complete the
configuration of AS-180’s BGP router and all the related BGP routers, so the following goals are achieved:

• Peer AS-180 with AS-171, so they can directly reach each other.
• Peer AS-180 with the AS-2 and AS-3 transit autonomous systems, so they can reach other destination
via these transits.


Shell script. In this task, we need to modify several BIRD configuration files. Instead of going to each
container to make changes, we can copy all the BIRD configuration files from the containers to the host
VM, make changes, and then copy them back to the containers. We have included two shell scripts in the
task1 folder to facilitate the process:

• import bird conf.sh: get all the needed BIRD configuration files from the containers. If a
configuration file already exists in the current folder, the file will not be overwritten.

• export bird conf.sh: copy the BIRD configuration files to the containers and run "birdc
configure" to reload the configuration.

Debugging. If the result is not what you have expected, you may need to debug to find out what has gone
wrong. In particular, you want to know where your packets go. For example, if you run ping, but you do
not get a reply, you want to know where the problem is. You can use the filter option in the map client, and
visualize the traffic flow. The syntax of the filter is the same as that in tcpdump. We give a few examples
in the following

![image](https://user-images.githubusercontent.com/131140229/236837042-266f1a6b-fa62-48d3-aa42-278426f50250.png)


Lab report. In your lab report, please include the content that you add to the BIRD configuration files,
and provide proper explanation. Please also include screenshots (such as traceroute) to demonstrate that
your task is successful.

## Task 2: Transit Autonomous System
If two ASes want to connect, they can peer with each other at an Internet exchange point. The question is
how two ASes in two different locations can reach each other. It is hard for them to find a common location
to peer. To solve this problem, a special type of AS is needed.

This type of AS have BGP routers in many Internet Exchange Points, where they peer with many other
ASes. Once packets get into its networks, they will be pulled from one IX to another IX (typically via some
internal routers), and eventually be handed over to another AS. This type of AS provides the transit service
for other ASes. That is how the hosts in one AS can reach the hosts in another AS, even though they are not
peers with each other. This special of AS is called Transit AS.

In this task, we will first understand how a transit AS works and then we will configure a transit AS
in our Internet emulator. Students should read Section 10 of the tutorial before working on this task. We
pick AS-3 transit autonomous system in this task. This AS has four BGP routers, each at a different Internet
exchange (IX). We pick the one connected to the Miami Internet exchange (IX-103)

##  Task 2.a: Experimenting with IBGP
For the task, we first need to find some traffic that goes through AS-3. We will ping 10.164.0.71 from a
host in AS-162. Using the map client program, we can see that the packets go through the AS-3 transit AS.
If this is not consistent with your observation, do find some other traffics that go through AS-3.

We will now disable the IBGP sessions on AS-3’s BGP router at IX-103 either using the map client or
from the command line (see the following example).

# birdc
bird> show protocols

![image](https://user-images.githubusercontent.com/131140229/236837585-3be785c6-06e1-49e0-84a7-5cf7fe018db4.png)

bird> disable ibgp3

bird> show protocols ibgp3





