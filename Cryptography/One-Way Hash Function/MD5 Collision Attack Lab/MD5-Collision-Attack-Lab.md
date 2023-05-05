# MD5 Collision Attack Lab

```
Copyright © 2018 by Wenliang Du.
This work is licensed under a Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International
License. If you remix, transform, or build upon the material, this copyright notice must be left intact, or
reproduced in a way that is reasonable to the medium in which the work is being re-published.
```
## 1 Introduction

A secure one-way hash function needs to satisfy two properties: the one-way property and the collision-
resistance property. The one-way property ensures that given a hash valueh, it is computationally infeasible
to find an inputM, such thathash(M) = h. The collision-resistance property ensures that it is compu-
tationally infeasible to find two different inputsM 1 andM 2 , such thathash(M 1 ) = hash(M 2 ).
Several widely-used one-way hash functions have trouble maintaining the collision-resistance prop-
erty. At the rump session of CRYPTO 2004, Xiaoyun Wang and co-authors demonstrated a collision attack
against MD5 [1]. In February 2017, CWI Amsterdam and Google Research announced theSHAtteredat-
tack, which breaks the collision-resistance property of SHA-1 [3]. While many students do not have trouble
understanding the importance of the one-way property, they cannot easily grasp why the collision-resistance
property is necessary, and what impact these attacks can cause.
The learning objective of this lab is for students to really understand the impact of collision attacks, and
see in first hand what damages can be caused if a widely-used one-way hash function’s collision-resistance
property is broken. To achieve this goal, students need to launch actual collision attacks against the MD
hash function. Using the attacks, students should be able to create two different programs that share the
same MD5 hash but have completely different behaviors. This lab covers a number of topics described in
the following:

- One-way hash function, MD
- The collision-resistance property
- Collision attacks

Readings. Detailed coverage of the one-way hash function can be found in the following:

- Chapter 22 of the SEED Book,Computer & Internet Security: A Hands-on Approach, 2nd Edition,
    by Wenliang Du. See details athttps://www.handsonsecurity.net.

Lab Environment. This lab has been tested on our pre-built Ubuntu 20.04 VM, which can be downloaded
from the SEED website. The lab uses a tool called “Fast MD5 Collision Generation”, which was written
by Marc Stevens. The name of the binary is calledmd5collgenin our VM, and it is installed inside the
/usr/binfolder. If it is not there, you can download it from the lab’s website (insideLabsetup.zip).
If you are interested in installing the tool to your own machine, you can download the source code directly
fromhttps://www.win.tue.nl/hashclash/.

Acknowledgment This lab was developed with the help of Vishtasp Jokhi, a graduate student in the De-
partment of Electrical Engineering and Computer Science at Syracuse University.


## 2 Lab Tasks

### 2.1 Task 1: Generating Two Different Files with the Same MD5 Hash

In this task, we will generate two different files with the same MD5 hash values. The beginning parts of these
two files need to be the same, i.e., they share the same prefix. We can achieve this using themd5collgen
program, which allows us to provide a prefix file with any arbitrary content. The way how the program works
is illustrated in Figure 1. The following command generates two output files,out1.binandout2.bin,
for a given a prefix fileprefix.txt:

$ md5collgen -p prefix.txt -o out1.bin out2.bin

```
MD5 Collision
Generation
Prefix
```
```
Prefix + (Padding) + P
```
```
Prefix + (Padding) + Q
```
```
Figure 1: MD5 collision generation from a prefix
```
We can check whether the output files are distinct or not using thediffcommand. We can also use the
md5sumcommand to check the MD5 hash of each output file. See the following commands.

$ diff out1.bin out2.bin
$ md5sum out1.bin
$ md5sum out2.bin

Sinceout1.binandout2.binare binary, we cannot view them using a text-viewer program, such
ascatormore; we need to use a binary editor to view (and edit) them. We have already installed a hex
editor software calledblessin our VM. Please use such an editor to view these two output files, and
describe your observations. In addition, you should answer the following questions:

- Question 1.If the length of your prefix file is not multiple of 64, what is going to happen?
- Question 2.Create a prefix file with exactly 64 bytes, and run the collision tool again, and see what
    happens.
- Question 3. Are the data (128 bytes) generated bymd5collgencompletely different for the two
    output files? Please identify all the bytes that are different.

### 2.2 Task 2: Understanding MD5’s Property

In this task, we will try to understand some of the properties of the MD5 algorithm. These properties are
important for us to conduct further tasks in this lab. MD5 is a quite complicated algorithm, but from very
high level, it is not so complicated. As Figure 2 shows, MD5 divides the input data into blocks of 64 bytes,
and then computes the hash iteratively on these blocks. The core of the MD5 algorithm is a compression
function, which takes two inputs, a 64-byte data block and the outcome of the previous iteration. The
compression function produces a 128-bit IHV, which stands for “Intermediate Hash Value”; this output is
then fed into the next iteration. If the current iteration is the last one, the IHV will be the final hash value.
The IHV input for the first iteration (IHV 0 ) is a fixed value.


```
Compression
Function
```
```
Compression
Function
```
```
Compression
```
**IHV 0 IHV 1 IHV 2 IHVn‐** (^1) Function **IHVn
M 1 M 2 Mn
Final Hash**
Figure 2: How the MD5 algorithm works
Based on how MD5 works, we can derive the following property of the MD5 algorithm: Given two
inputsMandN, ifMD5(M) = MD5(N), i.e., the MD5 hashes ofMandNare the same, then for any input
T,MD5(M ‖ T) = MD5(N ‖ T), where‖represents concatenation.
That is, if inputsMandNhave the same hash, adding the same suffixTto them will result in two outputs
that have the same hash value. This property holds not only for the MD5 hash algorithm, but also for many
other hash algorithms. Your job in this taskis to design an experiment to demonstrates that this property
holds for MD5.
You can use thecatcommand to concatenate two files (binary or text files) into one. The following
command concatenates the contents offile2to the contents offile1, and places the result infile3.
$ cat file1 file2 > file

### 2.3 Task 3: Generating Two Executable Files with the Same MD5 Hash

In this task, you are given the following C program. Your job is to create two different versions of this
program, such that the contents of theirxyzarrays are different, but the hash values of the executables are
the same.

#include <stdio.h>

unsigned char xyz[200] = {
/* The actual contents of this array are up to you */
};

int main()
{
int i;
for (i=0; i<200; i++){
printf("%x", xyz[i]);
}
printf("\n");
}

You may choose to work at the source code level, i.e., generating two versions of the above C program,
such that after compilation, their corresponding executable files have the same MD5 hash value. However,
it may be easier to directly work on the binary level. You can put some arbitrary values in thexyzarray,
compile the above code to binary. Then you can use a hex editor tool to modify the content of thexyzarray
directly in the binary file.
Finding where the contents of the array are stored in the binary is not easy. However, if we fill the array
with some fixed values, we can easily find them in the binary. For example, the following code fills the array
with0x41, which is the ASCII value for letterA. It will not be difficult to locate 200A’s in the binary.


unsigned char xyz[200] = {
0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41,
0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41,
0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41,
... (omitted) ...
0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41,
}

Guidelines. From inside the array, we can find two locations, from where we can divide the executable
file into three parts: a prefix, a 128-byte region, and a suffix. The length of the prefix needs to be multiple
of 64 bytes. See Figure 3 for an illustration of how the file is divided.

### AAAAAA...AAAAAAAAAAA

### Prefix (multiple of  64  bytes) Suffix (the rest of the program)

### A  128 ‐byte region

```
Figure 3: Break the executable file into three pieces.
```
We can runmd5collgenon the prefix to generate two outputs that have the same MD5 hash value.
Let us usePandQto represent the second part (each having 128 bytes) of these outputs (i.e., the part after
the prefix). Therefore, we have the following:

MD5 (prefix ‖ P) = MD5 (prefix‖ Q)

Based on the property of MD5, we know that if we append the same suffix to the above two outputs, the
resultant data will also have the same hash value. Basically, the following is true for any suffix:

MD5 (prefix ‖ P ‖ suffix) = MD5 (prefix‖ Q ‖ suffix)

Therefore, we just need to usePandQto replace 128 bytes of the array (between the two dividing
points), and we will be able to create two binary programs that have the same hash value. Their outcomes
are different, because they each print out their own arrays, which have different contents.

Tools. You can useblessto view the binary executable file and find the location for the array. For
dividing a binary file, there are some tools that we can use to divide a file from a particular location. The
headandtailcommands are such useful tools. You can look at their manuals to learn how to use them.
We give three examples in the following:

$ head -c 3200 a.out > prefix
$ tail -c 100 a.out > suffix
$ tail -c +3300 a.out > suffix

The first command above saves the first 3200 bytes ofa.outtoprefix. The second command saves
the last 100 bytes ofa.outtosuffix. The third command saves the data from the 3300 th byte to the


end of the filea.outtosuffix. With these two commands, we can divide a binary file into pieces from
any location. If we need to glue some pieces together, we can use thecatcommand.
If you useblessto copy-and-paste a block of data from one binary file to another file, the menu item
"Edit -> Select Range"is quite handy, because you can select a block of data using a starting point
and a range, instead of manually counting how many bytes are selected.

### 2.4 Task 4: Making the Two Programs Behave Differently

In the previous task, we have successfully created two programs that have the same MD5 hash, but their
behaviors are different. However, their differences are only in the data they print out; they still execute the
same sequence of instructions. In this task, we would like to achieve something more significant and more
meaningful.
Assume that you have created a software which does good things. You send the software to a trusted
authority to get certified. The authority conducts a comprehensive testing of your software, and concludes
that your software is indeed doing good things. The authority will present you with a certificate, stating that
your program is good. To prevent you from changing your program after getting the certificate, the MD
hash value of your program is also included in the certificate; the certificate is signed by the authority, so
you cannot change anything on the certificate or your program without rendering the signature invalid.
You would like to get your malicious software certified by the authority, but you have zero chance to
achieve that goal if you simply send your malicious software to the authority. However, you have noticed
that the authority uses MD5 to generate the hash value. You got an idea. You plan to prepare two different
programs. One program will always execute benign instructions and do good things, while the other program
will execute malicious instructions and cause damages. You find a way to get these two programs to share
the same MD5 hash value.
You then send the benign version to the authority for certification. Since this version does good things, it
will pass the certification, and you will get a certificate that contains the hash value of your benign program.
Because your malicious program has the same hash value, this certificate is also valid for your malicious
program. Therefore, you have successfully obtained a valid certificate for your malicious program. If other
people trusted the certificate issued by the authority, they will download your malicious program.
The objective of this taskis to launch the attack described above. Namely, you need to create two pro-
grams that share the same MD5 hash. However, one program will always execute benign instructions, while
the other program will execute malicious instructions. In your work, what benign/malicious instructions are
executed is not important; it is sufficient to demonstrate that the instructions executed by these two programs
are different.

Guidelines. Creating two completely different programs that produce the same MD5 hash value is quite
hard. The two hash-colliding programs produced bymd5collgenneed to share the same prefix; moreover,
as we can see from the previous task, if we need to add some meaningful suffix to the outputs produced by
md5collgen, the suffix added to both programs also needs to be the same. These are the limitations of
the MD5 collision generation program that we use. Although there are other more complicated and more
advanced tools that can lift some of the limitations, such as accepting two different prefixes [2], they demand
much more computing power, so they are out of the scope for this lab. We need to find a way to generate
two different programs within the limitations.
There are many ways to achieve the above goal. We provide one approach as a reference, but students
are encouraged to come up their own ideas. Instructors may consider rewarding students for their own
ideas. In our approach, we create two arraysXandY. We compare the contents of these two arrays; if they


are the same, the benign code is executed; otherwise, the malicious code is executed. See the following
pseudo-code:

Array X;
Array Y;

main()
{
if(X’s contents and Y’s contents are the same)
run benign code;
else
run malicious code;
return;
}

We can initialize the arraysXandYwith some values that can help us find their locations in the exe-
cutable binary file. Our job is to change the contents of these two arrays, so we can generate two different
versions that have the same MD5 hash. In one version, the contents of X and Y are the same, so the benign
code is executed; in the other version, the contents of X and Y are different, so the malicious code is exe-
cuted. We can achieve this goal using a technique similar to the one used in Task 3. Figure 4 illustrates what
the two versions of the program look like.

```
AAAAAA...AAAAAAAAAAA
```
```
Prefix (multiple of  64  bytes) Suffix (the rest of the program)
```
# P

# AAAAAA...AAAAAAAAAAA Q

# AAAAAA...AAAAAAAAAAA P

# AAAAAA...AAAAAAAAAAA P

```
Array X Array Y
```
```
P and Q are generated to
cause collision
```
```
Figure 4: An approach to generate two hash-colliding programs with different behaviors.
```
From Figure 4, we know that these two binary files have the same MD5 hash value, as long asPand
Qare generated accordingly. In the first version, we make the contents of arrays X and Y the same, while
in the second version, we make their contents different. Therefore, the only thing we need to change is the
contents of these two arrays, and there is no need to change the logic of the programs.

## 3 Submission

You need to submit a detailed lab report, with screenshots, to describe what you have done and what you
have observed. You also need to provide explanation to the observations that are interesting or surprising.
Please also list the important code snippets followed by explanation. Simply attaching code without any
explanation will not receive credits.


## References

[1] John Black, Martin Cochran, and Trevor Highland. A study of the md5 attacks: Insights and improve-
ments. InProceedings of the 13th International Conference on Fast Software Encryption, FSE’06, pages
262–277, Berlin, Heidelberg, 2006. Springer-Verlag.

[2] Marc Stevens. On collisions for md5. Master’s thesis, Eindhoven University of Technology, 6 2007.

[3] Marc Stevens, Elie Bursztein, Pierre Karpman, Ange Albertini, and Yarik Markov. The first collision
for full SHA-1. CWI Amsterdam and Google Research,https://shattered.io/, 2017.

