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

