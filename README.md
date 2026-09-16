# Linux System Monitor - Multi-Threaded C Client/Server

A Linux system-monitoring service written in C. A TCP server accepts multiple clients concurrently using pthreads. Clients request system metrics and the server reads Linux /proc interfaces and returns the results.

## Features
- POSIX TCP sockets
- One pthread per client connection
- Length-prefixed TCP framing
- CPU/load, memory, and uptime monitoring
- Combined status command
- Makefile builds

## Architecture
Client -> TCP -> Server -> pthread -> monitor.c -> /proc

## Build

```bash
make
```

## Run

Terminal 1:
```bash
./server 127.0.0.1 9000
```

Terminal 2:
```bash
./client 127.0.0.1 9000
```

## Commands

```text
cpu
memory
uptime
load
status
help
quit
```

## Linux interfaces
- `/proc/loadavg` for load averages
- `/proc/meminfo` for memory information
- `/proc/uptime` for system uptime

The server uses direct Linux interfaces rather than spawning shell commands.

## TCP protocol

Requests and responses use:

```text
[4-byte uint32 length][payload]
```

The framing layer handles TCP as a byte stream rather than assuming one send equals one receive.

## SRE / Systems Concepts
- Linux system monitoring
- TCP networking
- Concurrent request handling
- Resource monitoring
- pthreads
- Make/GCC build workflow
- Separation of networking and monitoring logic
