# CSMsrv

> The important part is not how you use it, but how it implements the stuff you are using.

## What's this

A car selling manager. hust 2016 C programming course assignment project.

Notice: this project is assigned for 2017 summer holiday.

> Note: removed git submodule management. Now the first two makedepends are included in the repository.

## Makedepend

- `zhwkre` -- a simple c based library written by myself. `git clone` should work.
- `dear imgui` -- for client GUI. `git clone` should work.
- `glfw version 3` -- for the `dear imgui` backend.
- `Linux/Unix environment` -- for networking and concurrent of `zhwkre`.
- `gcc 7.1.1(20170528) or 7.1.1-3` -- for compiling zhwkre and base structrue of project.
- `g++ the same version with gcc`. -- for compiling imgui and cimgui.
- Notice: *DO NOT* guarantee it works on the newest `GCC(7.1.1-4,20170630?)`

## Building

``` shell
1. $ git clone <this_repo>
2. $ cd CSMsrv # enter the build directory
3. $ make server # for server side !OPTIONAL<1>
4. $ make client # for client side !OPTIONAL<1>
5. $ make # automake the two !OPTIONAL<2>
6. $ make clean # remove generated files(include the executables)
```

## Running

``` shell
1. $ ./srv 127.0.0.1:1992 # start up server, listen at localhost,port 1992
2. $ ./cli
3. $ # then enter 127.0.0.1:1992 at serveraddr, username and password.
```

## Current Status

- *BUILD PASSING*
- *TEST PASSING*

## Using for Future Projects

### *Deprecated*

Since 2017, the C course projects has cancelled.

### IMPORTANT

Make sure you have fully understood the codes in this project.

The critical points are as follows:

- Doubly linked lists.
- Hash maps.
- Networking using `socket` in \*nix environments.
- Concurrent and (a)synchronization theory and practise using `pthread`.
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

## LICENSE

Licensed under MIT.