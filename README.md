# SOL project - FileStorage server

Evaluation project for the Operative Systems course at unipi, June-July 2022.

## Description

The project consists of developing a file storage server that uses the main memory to store files in an UNIX environment.

The server is a multi-threaded process that accepts connection from multiple clients on the same device (using AF_UNIX sockets), making a log of every operation.

The clients interface with the server using a communication API, sending one or more requests.
The type and number of requests depend on the command line argument used to start each client.

The Makefile will also include 3 test target to check if the server performs appropriately
Furthermore, a bash script to summarize and print the server log is necessary

## Version
0 - No progress has been made so far
