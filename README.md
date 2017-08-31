# CSMsrv

## What's this

A car selling manager. hust 2016 C programming course assignment project.

notice: due to course timetable changes in the next year,

I hereby claim that this project is assigned at 2017 summer holiday.

the zhwkre included here is a modified version of my zhwkre repo.

removed some functions and added some functions to fully satisfy the

criteria specified in instruction book.

## Makedepend

- imgui -- for client GUI. git clone --recursive should work.
- glfw version 3 -- for the imgui backend.
- Linux/Unix environment -- for networking and concurrent.
- gcc 7.1.1(20170528) or 7.1.1-3
- g++ the same version with gcc. for compiling imgui and cimgui.

- Notice: do not guarantee it works on the newest GCC(7.1.1-4,20170630?)

## Building

1. $ git clone --recursive <this_repo>
1. $ cd CSMsrv
1. $ make server # for server side
1. $ make client # for client side
1. $ make # automake the two
1. $ make clean # clean up(include the executables)

## Current Status

- Important: not fully written yet.

## Using for Future Projects

### IMPORTANT

Make sure you have fully understood the codes in this project.

The critical points are as follows:

- Doubly linked lists.
- Hash maps.
- Networking using socket in \*nix environments.
- Concurrent and (a)synchronization theory and practise using pthread.
- Recursive strategy on (un)serializing the lists.

### Method

Notice that the server side program is written for a universal usage.

So all you need to do is alter the structure definition in conf.h

and redraw the client side GUI.

### WARNING

This implementation can fit in all course project in theory.

But due to the unusual structure this implementation used,

directly copy this project can easily be discovered.

Use at your *OWN RISK*.
