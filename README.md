# SOL project - FileStorage server

**Examination project for the Operative Systems course at unipi, June-July 2022**

## Description

The project consists of making a client-server applications in an **UNIX** environment using C language and UNIX functionalities.
In particular, it is a client server file-server application that runs on the same machine, and uploads nad downloads files into the memory.

The server is a multi-threaded process that accepts connection from multiple clients on the same device (using AF_UNIX sockets), logging every operation.

The clients interface with the server using a communication API, sending one or more requests.
The type and number of requests depend on the command line argument used to start each client.

The Makefile will also include 3 test target to check if the server performs appropriately
Furthermore, a bash script to summarize and print the server log is necessary

## Installation & Run instrunction
The project can be cloned from this github repository or downloaded from it all the same.

It must run on an unix operative system.
To compile it, you can run 'make' in the project root folder from CLI, which will compile all the sources in object files and then link them into the ServerApp and the ClientApp executables.

ServerApp.exe will read a config file from CLI or the default config.txt found into the config folder (doing some mockup tests for now), and it will create an AF_UNIX socket in the project tmp/ folder.

ClientApp will read the CLI and will act on the options given. 
However, it currently cannot do much - at most it will connect to the server (the path must be relative to its own, by default using the command '-f tmp/mySocketDefault') and write a simple message before terminating. 
(you can choose the message with the option -W, which should normally write files into the server but the feature is not implemented in this version)

The makefile can also be used to clean the project folder from every .o, .exe and from the socket tmp file in case an error occurred and it has been left there.

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
0.1 - Client and Server can communicate very basically but it looks stable enough for a bit of connections.
	Hashmap currently being worked on as the data structure to use for the File Cache
