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

Please download the Labsetup.zip file from the web page, and unzip
it. The files inside the output sub-folder are the actual emulation files (container files) that are generated
from the Python code mini-internet.py.

## Start the emulation

We will directly use the container files in the output folder. Go to this folder,
and run the following docker commands to build and start the containers. We recommend that you run the
emulator inside the provided SEED Ubuntu 20.04 VM, but doing it in a generic Ubuntu 20.04 operating
system should not have any problem, as long as the docker software is installed. Readers can find the docker
manual from  this link https://github.com/seed-labs/seed-labs/blob/master/manuals/docker/SEEDManual-Container.md



