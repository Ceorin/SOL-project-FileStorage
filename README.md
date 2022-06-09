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

> FILE_NUM num <br> MEMORY num <br> WORKERS_NUM num <br> SOCKET socketName <br> LOG logName <br>

The memory given will be interpreted in *bytes*, adding K or M will respectively make the server interpret it as KB or MB.
Log files will be saved in the **logs** folder

**ATTENTION**
The config file cannot have more than 5 lines (except empty lines)
The values can be in any order but if any one line doesn't follow the pattern, it will raise an exception.

*If a config file is not provided, the default config file will be looked for in the config folder*
*If the config file used does not contain one of the five parameter, the value will be set to the default for that parameter*

## Version
0.00 - Pretty much nothing to work with so far
