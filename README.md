# broker
An implementation of the broker pattern for distributed network components implemented in C using TCP/IP sockets. 

# Introduction

This is my library of useful c routines. The purposes of this library are:

Write common and useful functions that I can reuse in other programs.
Make it so that I just need to link to the library in order to use the functionality provided.
Use this as reference material for future code lookups to help me remember how to do a certain thing
The intention is also to have it as portable as possible to work in Linux and Windows. This will mean that the library can build unchanged across these platforms. Where OS specific functionality is needed in the library, this should be disabled/enabled based on the platform we are builing on.

# Functionality coverage

I've broken down the library down into functional areas. This encompases a low-level API such as moving/managing memory.

Basic, low-level routines

Storing and managing data
Lists
Writing to files
Writing to the console
Managing access to memory
Safety checking your program
Debugging routines for tracing
Working with strings in general
Working with timing
Validation and testing routines
Encryption
Compression
IPC
Higher-level routines

Graphics
Database
Internet
Parsing
Command line parsing
Networking and protocols
Regex
Threading
Interpreters
Internationalization
Windowed applications/forms
Application design and patterns
