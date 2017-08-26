# CSMsrv

## What's this

A car selling manager. hust 2016 C programming course assignment.

notice: due to course timetable changes in the next year,

I hereby claim that this project is assigned at 2017 summer holiday.

the zhwkre included here is a modified version of my zhwkre repo.

removed some functions and added some functions to fully satisfy the

criteria specified in instruction book.

## Makedepend

- imgui & cimgui -- for client GUI. git clone --recursive should work.
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