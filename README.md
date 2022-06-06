# SOL project - FileStorage server

**Evaluation project for the Operative Systems course at unipi, June-July 2022**

## Description

The project consists of developing a file storage server that uses the main memory to store files in an UNIX environment.

The server is a multi-threaded process that accepts connection from multiple clients on the same device (using AF_UNIX sockets), logging every operation.

The clients interface with the server using a communication API, sending one or more requests.
The type and number of requests depend on the command line argument used to start each client.

The Makefile will also include 3 test target to check if the server performs appropriately
Furthermore, a bash script to summarize and print the server log is necessary

## Config format
Config must be a txt file written in the following pattern

> FILE_NUM num
> MEMORY num
> WORKERS_NUM num
> SOCKET socketName
> LOG logName

The memory given will be interpreted in *bytes*, adding K or M will respectively make the server interpret it as KB or MB.
Log files will be saved in the **logs** folder

**ATTENTION**
The config file cannot have more than 5 lines.
The values can be in any order but if any one line doesn't follow the pattern, it will raise an exception.

*If a config file is not provided, or if one of the value is not provided in the configuration file used as an argument, the value will be replaced by a default one found in config/config.txt*

## Version
0 - No progress has been made so far
