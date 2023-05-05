# Environment Variable andSet-UIDProgram Lab

```
Copyright © 2006 - 2016 by Wenliang Du.
This work is licensed under a Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International
License. If you remix, transform, or build upon the material, this copyright notice must be left intact, or
reproduced in a way that is reasonable to the medium in which the work is being re-published.
```
## 1 Overview

The learning objective of this lab is for students to understand how environment variables affect program
and system behaviors. Environment variables are a set of dynamic named values that can affect the way
running processes will behave on a computer. They are used by most operating systems, since they were
introduced to Unix in 1979. Although environment variables affect program behaviors, how they achieve
that is not well understood by many programmers. As a result, if a program uses environment variables, but
the programmer does not know that they are used, the program may have vulnerabilities.
In this lab, students will understand how environment variables work, how they are propagated from
parent process to child, and how they affect system/program behaviors. We are particularly interested in how
environment variables affect the behavior ofSet-UIDprograms, which are usually privileged programs.
This lab covers the following topics:

- Environment variables
- Set-UIDprograms
- Securely invoke external programs
- Capability leaking
- Dynamic loader/linker

Readings and videos. Detailed coverage of theSet-UIDmechanism, environment variables, and their
related security problems can be found in the following:

- Chapters 1 and 2 of the SEED Book,Computer & Internet Security: A Hands-on Approach, 2nd
    Edition, by Wenliang Du. See details athttps://www.handsonsecurity.net.
- Section 2 of the SEED Lecture at Udemy,Computer Security: A Hands-on Approach, by Wenliang
    Du. See details athttps://www.handsonsecurity.net/video.html.

Lab environment. You can perform the lab exercise on the SEED VM provided by the Cloudlabs.

## 2 Lab Tasks

Files needed for this lab are included in Labsetup.zip, which can be fetched by running the following commands.

```
sudo wget https://github.com/CloudLabs-MOC/CloudLabs-SEED/blob/main/Software%20Security/Set-UID%20Programs/Lab%20files/Labsetup.zip
```

```
sudo unzip Labsetup.zip
```

### 2.1 Task 1: Manipulating Environment Variables

In this task, we study the commands that can be used to set and unset environment variables. We are using
Bash in the seed account. The default shell that a user uses is set in the/etc/passwdfile (the last field
of each entry). You can change this to another shell program using the commandchsh(please do not do it
for this lab). Please do the following tasks:

- Useprintenvorenvcommand to print out the environment variables. If you are interested in
    some particular environment variables, such asPWD, you can use"printenv PWD"or"env |
    grep PWD".
- Useexportandunsetto set or unset environment variables. It should be noted that these two
    commands are not separate programs; they are two of the Bash’s internal commands (you will not be
    able to find them outside of Bash).

### 2.2 Task 2: Passing Environment Variables from Parent Process to Child Process

In this task, we study how a child process gets its environment variables from its parent. In Unix,fork()
creates a new process by duplicating the calling process. The new process, referred to as the child, is an
exact duplicate of the calling process, referred to as the parent; however, several things are not inherited by
the child (please see the manual offork()by typing the following command:man fork). In this task,
we would like to know whether the parent’s environment variables are inherited by the child process or not.

Step 1. Please compile and run the following program, and describe your observation. The program can
be found in theLabsetupfolder; it can be compiled using"gcc myprintenv.c", which will generate
a binary calleda.out. Let’s run it and save the output into a file using"a.out > file".

Listing 1:myprintenv.c
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

extern char **environ;
void printenv()
{
int i = 0;
while (environ[i] != NULL) {
printf("%s\n", environ[i]);
i++;
}
}

void main()
{
pid_t childPid;
switch(childPid = fork()) {
case 0: /* child process*/
printenv(); ¿
exit(0);
default: /* parent process*/
//printenv(); ¡


exit(0);
}
}

Step 2. Now comment out theprintenv()statement in the child process case (Line¿), and uncomment
theprintenv()statement in the parent process case (Line¡). Compile and run the code again, and
describe your observation. Save the output in another file.

Step 3. Compare the difference of these two files using thediffcommand. Please draw your conclusion.

### 2.3 Task 3: Environment Variables andexecve()

In this task, we study how environment variables are affected when a new program is executed viaexecve().
The functionexecve()calls a system call to load a new command and execute it; this function never re-
turns. No new process is created; instead, the calling process’s text, data, bss, and stack are overwritten by
that of the program loaded. Essentially,execve()runs the new program inside the calling process. We
are interested in what happens to the environment variables; are they automatically inherited by the new
program?

Step 1. Please compile and run the following program, and describe your observation. This program
simply executes a program called/usr/bin/env, which prints out the environment variables of the
current process.

Listing 2:myenv.c
#include <unistd.h>

extern char **environ;
int main()
{
char *argv[2];

```
argv[0] = "/usr/bin/env";
argv[1] = NULL;
execve("/usr/bin/env", argv, NULL); ¿
```
return 0 ;
}

Step 2. Change the invocation ofexecve()in Line¿to the following; describe your observation.

execve("/usr/bin/env", argv, environ);

Step 3. Please draw your conclusion regarding how the new program gets its environment variables.


### 2.4 Task 4: Environment Variables andsystem()

In this task, we study how environment variables are affected when a new program is executed via the
system()function. This function is used to execute a command, but unlikeexecve(), which di-
rectly executes a command,system()actually executes"/bin/sh -c command", i.e., it executes
/bin/sh, and asks the shell to execute the command.
If you look at the implementation of thesystem()function, you will see that it usesexecl()to
execute/bin/sh;execl()callsexecve(), passing to it the environment variables array. Therefore,
usingsystem(), the environment variables of the calling process is passed to the new program/bin/sh.
Please compile and run the following program to verify this.

#include <stdio.h>
#include <stdlib.h>

int main()
{
system("/usr/bin/env");
return 0 ;
}

### 2.5 Task 5: Environment Variable andSet-UIDPrograms

Set-UIDis an important security mechanism in Unix operating systems. When aSet-UIDprogram
runs, it assumes the owner’s privileges. For example, if the program’s owner is root, when anyone runs
this program, the program gains the root’s privileges during its execution.Set-UIDallows us to do many
interesting things, but since it escalates the user’s privilege, it is quite risky. Although the behaviors of
Set-UIDprograms are decided by their program logic, not by users, users can indeed affect the behav-
iors via environment variables. To understand howSet-UIDprograms are affected, let us first figure out
whether environment variables are inherited by theSet-UIDprogram’s process from the user’s process.

Step 1. Write the following program that can print out all the environment variables in the current process.

#include <stdio.h>
#include <stdlib.h>

extern char **environ;
int main()
{
int i = 0;
while (environ[i] != NULL) {
printf("%s\n", environ[i]);
i++;
}
}

Step 2. Compile the above program, change its ownership toroot, and make it aSet-UIDprogram.

// Asssume the program’s name is foo
$ sudo chown root foo
$ sudo chmod 4755 foo


Step 3. In your shell (you need to be in a normal user account, not therootaccount), use theexport
command to set the following environment variables (they may have already exist):

- PATH
- LDLIBRARYPATH
- ANYNAME(this is an environment variable defined by you, so pick whatever name you want).

These environment variables are set in the user’s shell process. Now, run theSet-UIDprogram from
Step 2 in your shell. After you type the name of the program in your shell, the shell forks a child process,
and uses the child process to run the program. Please check whether all the environment variables you set
in the shell process (parent) get into theSet-UIDchild process. Describe your observation. If there are
surprises to you, describe them.

### 2.6 Task 6: The PATH Environment Variable andSet-UIDPrograms

Because of the shell program invoked, callingsystem()within aSet-UIDprogram is quite dangerous.
This is because the actual behavior of the shell program can be affected by environment variables, such as
PATH; these environment variables are provided by the user, who may be malicious. By changing these
variables, malicious users can control the behavior of theSet-UIDprogram. InBash, you can change
thePATHenvironment variable in the following way (this example adds the directory/home/seedto the
beginning of thePATHenvironment variable):

$ export PATH=/home/seed:$PATH

TheSet-UIDprogram below is supposed to execute the/bin/lscommand; however, the program-
mer only uses the relative path for thelscommand, rather than the absolute path:

int main()
{
system("ls");
return 0;
}

Please compile the above program, change its owner toroot, and make it aSet-UIDprogram. Can
you get thisSet-UIDprogram to run your own malicious code, instead of/bin/ls? If you can, is your
malicious code running with the root privilege? Describe and explain your observations.

Note: Thesystem(cmd)function executes the/bin/shprogram first, and then asks this shell pro-
gram to run thecmdcommand. In Ubuntu 20.04 (and several versions before),/bin/shis actually a
symbolic link pointing to/bin/dash. This shell program has a countermeasure that prevents itself from
being executed in aSet-UIDprocess. Basically, ifdashdetects that it is executed in aSet-UIDprocess,
it immediately changes the effective user ID to the process’s real user ID, essentially dropping the privilege.
Since our victim program is aSet-UIDprogram, the countermeasure in/bin/dashcan prevent our
attack. To see how our attack works without such a countermeasure, we will link/bin/shto another shell
that does not have such a countermeasure. We have installed a shell program calledzshin our Ubuntu
20.04 VM. We use the following commands to link/bin/shto/bin/zsh:

$ sudo ln -sf /bin/zsh /bin/sh


### 2.7 Task 7: TheLDPRELOADEnvironment Variable andSet-UIDPrograms

In this task, we study howSet-UIDprograms deal with some of the environment variables. Several en-
vironment variables, includingLDPRELOAD,LDLIBRARYPATH, and otherLD*influence the behavior
of dynamic loader/linker. A dynamic loader/linker is the part of an operating system (OS) that loads (from
persistent storage to RAM) and links the shared libraries needed by an executable at run time.
In Linux,ld.soorld-linux.so, are the dynamic loader/linker (each for different types of binary).
Among the environment variables that affect their behaviors,LDLIBRARYPATHandLDPRELOADare
the two that we are concerned in this lab. In Linux,LDLIBRARYPATHis a colon-separated set of di-
rectories where libraries should be searched for first, before the standard set of directories.LDPRELOAD
specifies a list of additional, user-specified, shared libraries to be loaded before all others. In this task, we
will only studyLDPRELOAD.

Step 1. First, we will see how these environment variables influence the behavior of dynamic loader/linker
when running a normal program. Please follow these steps:

1. Let us build a dynamic link library. Create the following program, and name itmylib.c. It basically
    overrides thesleep()function inlibc:
    #include <stdio.h>
    void sleep (int s)
    {
       /* If this is invoked by a privileged program,
          you can do damages here! */
       printf("I am not sleeping!\n");
    }
2. We can compile the above program using the following commands (in the-lcargument, the second
    character is`):
    $ gcc -fPIC -g -c mylib.c
    $ gcc -shared -o libmylib.so.1.0.1 mylib.o -lc
3. Now, set theLDPRELOADenvironment variable:
    $ export LD_PRELOAD=./libmylib.so.1.0.
4. Finally, compile the following programmyprog, and in the same directory as the above dynamic link
    librarylibmylib.so.1.0.1:
    /* myprog.c */
    #include <unistd.h>
    int main()
    {
       sleep(1);
       return 0;
    }

Step 2. After you have done the above, please runmyprogunder the following conditions, and observe
what happens.


- Makemyproga regular program, and run it as a normal user.
- MakemyprogaSet-UIDroot program, and run it as a normal user.
- MakemyprogaSet-UIDroot program, export theLDPRELOADenvironment variable again in
    the root account and run it.
- MakemyprogaSet-UIDuser1 program (i.e., the owner is user1, which is another user account),
    export theLDPRELOADenvironment variable again in a different user’s account (not-root user) and
    run it.

Step 3. You should be able to observe different behaviors in the scenarios described above, even though
you are running the same program. You need to figure out what causes the difference. Environment variables
play a role here. Please design an experiment to figure out the main causes, and explain why the behaviors
in Step 2 are different. (Hint: the child process may not inherit theLD*environment variables).

### 2.8 Task 8: Invoking External Programs Usingsystem()versusexecve()

Althoughsystem()andexecve()can both be used to run new programs,system()is quite danger-
ous if used in a privileged program, such asSet-UIDprograms. We have seen how the PATH environment
variable affect the behavior ofsystem(), because the variable affects how the shell works. execve()
does not have the problem, because it does not invoke shell. Invoking shell has another dangerous conse-
quence, and this time, it has nothing to do with environment variables. Let us look at the following scenario.
Bob works for an auditing agency, and he needs to investigate a company for a suspected fraud. For
the investigation purpose, Bob needs to be able to read all the files in the company’sUnixsystem; on the
other hand, to protect the integrity of the system, Bob should not be able to modify any file. To achieve this
goal, Vince, the superuser of the system, wrote a special set-root-uid program (see below), and then gave the
executable permission to Bob. This program requires Bob to type a file name at the command line, and then
it will run/bin/catto display the specified file. Since the program is running as a root, it can display any
file Bob specifies. However, since the program has no write operations, Vince is very sure that Bob cannot
use this special program to modify any file.

Listing 3:catall.c
int main(int argc, char *argv[])
{
char *v[3];
char *command;

```
if(argc < 2) {
printf("Please type a file name.\n");
return 1;
}
```
```
v[0] = "/bin/cat"; v[1] = argv[1]; v[2] = NULL;
command = malloc(strlen(v[0]) + strlen(v[1]) + 2);
sprintf(command, "%s %s", v[0], v[1]);
```
```
// Use only one of the followings.
system(command);
// execve(v[0], v, NULL);
```

return 0 ;
}

Step 1: Compile the above program, make it a root-ownedSet-UIDprogram. The program will use
system()to invoke the command. If you were Bob, can you compromise the integrity of the system? For
example, can you remove a file that is not writable to you?

Step 2: Comment out thesystem(command)statement, and uncomment theexecve()statement;
the program will useexecve()to invoke the command. Compile the program, and make it a root-owned
Set-UID. Do your attacks in Step 1 still work? Please describe and explain your observations.

### 2.9 Task 9: Capability Leaking

To follow the Principle of Least Privilege,Set-UIDprograms often permanently relinquish their root
privileges if such privileges are not needed anymore. Moreover, sometimes, the program needs to hand over
its control to the user; in this case, root privileges must be revoked. Thesetuid()system call can be
used to revoke the privileges. According to the manual, “setuid()sets the effective user ID of the calling
process. If the effective UID of the caller is root, the real UID and saved set-user-ID are also set”. Therefore,
if aSet-UIDprogram with effective UID 0 callssetuid(n), the process will become a normal process,
with all its UIDs being set ton.
When revoking the privilege, one of the common mistakes is capability leaking. The process may have
gained some privileged capabilities when it was still privileged; when the privilege is downgraded, if the
program does not clean up those capabilities, they may still be accessible by the non-privileged process.
In other words, although the effective user ID of the process becomes non-privileged, the process is still
privileged because it possesses privileged capabilities.
Compile the following program, change its owner to root, and make it aSet-UIDprogram. Run the
program as a normal user. Can you exploit the capability leaking vulnerability in this program? The goal is
to write to the/etc/zzzfile as a normal user.

Listing 4:capleak.c
void main()
{
int fd;
char *v[2];

```
/* Assume that /etc/zzz is an important system file,
* and it is owned by root with permission 0644.
* Before running this program, you should create
* the file /etc/zzz first. */
fd = open("/etc/zzz", O_RDWR | O_APPEND);
if (fd == -1) {
printf("Cannot open /etc/zzz\n");
exit(0);
}
```
```
// Print out the file descriptor value
printf("fd is %d\n", fd);
```

```
// Permanently disable the privilege by making the
// effective uid the same as the real uid
setuid(getuid());
```
// Execute /bin/sh
v[0] = "/bin/sh"; v[1] = 0;
execve(v[0], v, 0);
}

## 3 Submission

You need to submit a detailed lab report, with screenshots, to describe what you have done and what you
have observed. You also need to provide explanation to the observations that are interesting or surprising.
Please also list the important code snippets followed by explanation. Simply attaching code without any
explanation will not receive credits.


