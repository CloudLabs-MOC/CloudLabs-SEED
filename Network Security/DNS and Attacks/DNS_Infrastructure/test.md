# Local DNS Attack Lab 

Copyright © 2018 - 2020 by Wenliang Du.
This work is licensed under a Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International
License. If you remix, transform, or build upon the material, this copyright notice must be left intact, or
reproduced in a way that is reasonable to the medium in which the work is being re-published.

## 1 Lab Overview
DNS (Domain Name System) is the Internet’s phone book; it translates hostnames to IP addresses (and vice
versa). This translation is through DNS resolution, which happens behind the scene. DNS attacks manipulate this resolution process in various ways, with an intent to misdirect users to alternative destinations,
which are often malicious. The objective of this lab is to understand how such attacks work. Students will
first set up and configure a DNS server, and then they will try various DNS attacks on the target that is also
within the lab environment.
  
  The difficulties of attacking local victims versus remote DNS servers are quite different. Therefore, we
have developed two labs, one focusing on local DNS attacks, and the other on remote DNS attack. This lab
focuses on local attacks. This lab covers the following topics:

• DNS and how it works

• DNS server setup

• DNS cache poisoning attack

• Spoofing DNS responses

• Packet sniffing and spoofing

• The Scapy tool

### Readings and videos 
Detailed coverage of the DNS protocol and attacks can be found in the following:

• Chapter 18 of the SEED Book, Computer & Internet Security: A Hands-on Approach, 2nd Edition,
by Wenliang Du. See details at https://www.handsonsecurity.net.

• Section 7 of the SEED Lecture, Internet Security: A Hands-on Approach, by Wenliang Du. See details
at https://www.handsonsecurity.net/video.html.

### Lab environment
You can perform the lab exercise on the SEED VM provided by the Cloudlabs.

## 2 Lab Environment Setup Task

The main target for DNS cache poisoning attacks is local DNS server. Obviously, it is illegal to attack a real
server, so we need to set up our own DNS server to conduct the attack experiments. The lab environment
needs four separate machines: one for the victim, one for the local DNS server, and two for the attacker.
The lab environment setup is illustrated in Figure 1. This lab focuses on the local attack, so we put all these
machines on the same LAN.

![image](https://user-images.githubusercontent.com/112887172/236833692-228b9397-c814-41a6-ae64-8e8776e5f652.png)

### 2.1 Container Setup and Commands
Please download the Labsetup.zip file to your VM from the lab’s website, unzip it, enter the Labsetup
folder, and use the docker-compose.yml file to set up the lab environment. Detailed explanation of the
content in this file and all the involved Dockerfile can be found from the user manual, which is linked
to the website of this lab. If this is the first time you set up a SEED lab environment using containers, it is
very important that you read the user manual.

In the following, we list some of the commonly used commands related to Docker and Compose. Since
we are going to use these commands very frequently, we have created aliases for them in the .bashrc file
(in our provided SEEDUbuntu 20.04 VM).

     $ docker-compose build # Build the container image

     $ docker-compose up # Start the container

     $ docker-compose down # Shut down the container

     // Aliases for the Compose commands above

     $ dcbuild # Alias for: docker-compose build

     $ dcup # Alias for: docker-compose up

     $ dcdown # Alias for: docker-compose down
     

All the containers will be running in the background. To run commands on a container, we often need
to get a shell on that container. We first need to use the "docker ps" command to find out the ID of
the container, and then use "docker exec" to start a shell on that container. We have created aliases for
them in the .bashrc file.

      $ dockps // Alias for: docker ps --format "{{.ID}} {{.Names}}"
      
      $ docksh <id> // Alias for: docker exec -it <id> /bin/bash
      
      // The following example shows how to get a shell inside hostC
      
      $ dockps
      
      b1004832e275 hostA-10.9.0.5
      
      0af4ea7a3e2e hostB-10.9.0.6
      
      9652715c8e0a hostC-10.9.0.7
    
      $ docksh 96
      root@9652715c8e0a:/#
      
      // Note: If a docker command requires a container ID, you do not need to
      
      // type the entire ID string. Typing the first few characters will
      
      // be sufficient, as long as they are unique among all the containers.

If you encounter problems when setting up the lab environment, please read the “Common Problems”
section of the manual for potential solutions.

### 2.2 About the Attacker Container
In this lab, we can either use the VM or the attacker container as the attacker machine. If you look at
the Docker Compose file, you will see that the attacker container is configured differently from the other
containers.

• Shared folder. 
When we use the attacker container to launch attacks, we need to put the attacking
code inside the attacker container. Code editing is more convenient inside the VM than in containers,
because we can use our favorite editors. In order for the VM and container to share files, we have
created a shared folder between the VM and the container using the Docker volumes. If you look
at the Docker Compose file, you will find out that we have added the following entry to some of the
containers. It indicates mounting the ./volumes folder on the host machine (i.e., the VM) to the
/volumes folder inside the container. We will write our code in the ./volumes folder (on the
VM), so they can be used inside the containers.

      volumes:
      - ./volumes:/volumes
      
• Host mode.
In this lab, the attacker needs to be able to sniff packets, but running sniffer programs
inside a container has problems, because a container is effectively attached to a virtual switch, so it
can only see its own traffic, and it is never going to see the packets among other containers. To solve
this problem, we use the host mode for the attacker container. This allows the attacker container to
see all the traffics. The following entry used on the attacker container:
      network_mode: host

When a container is in the host mode, it sees all the host’s network interfaces, and it even has the
same IP addresses as the host. Basically, it is put in the same network namespace as the host VM.
However, the container is still a separate machine, because its other namespaces are still different
from the host.

### 2.3 Summary of the DNS Configuration
All the containers are already configured for this lab. We provide a summary here, so students are aware of
these configurations. Detailed explanation of the configuration can be found from the manual.

#### Local DNS Server 
We run the BIND 9 DNS server program on the local DNS server. BIND 9 gets its
configuration from a file called /etc/bind/named.conf. This file is the primary configuration file, and
it usually contains several "include" entries, i.e., the actual configurations are stored in those included
files. One of the included files is called /etc/bind/named.conf.options. This is where the actual
configuration is set.

• Simplification. DNS servers now randomize the source port number in their DNS queries; this makes
the attacks much more difficult. Unfortunately, many DNS servers still use predictable source port
number. For the sake of simplicity in this lab, we fix the source port number to 33333 in the configuration file.

• Turning off DNSSEC. DNSSEC is introduced to protect against spoofing attacks on DNS servers. To
show how attacks work without this protection mechanism, we have turned off the protection in the
configuration file.

• DNS cache. During the attack, we need to inspect the DNS cache on the local DNS server. The
following two commands are related to DNS cache. The first command dumps the content of the
cache to the file /var/cache/bind/dump.db, and the second command clears the cache.

      # rndc dumpdb -cache // Dump the cache to the specified file
      # rndc flush // Flush the DNS cache

      










