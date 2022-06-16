#include <stdio.h>
#include <string.h>
#include <error.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#ifndef UNIX_PATH_MAX
    #define UNIX_PATH_MAX 108
#endif

#define helpMsg \
"This program is an application designed to communicate with a file server through the communication API defined in this project, to upload and download files on it.\n\
The program acts on which command line arguments have been given, with the following optios:\n\n\
 -h\t\t\tOutput this message on how to use the application.\n\n\
 -f filename\t\tSpeficies the server AF_UNIX socket.\n\n\
 -w dirname[,n=0]\tSends all the files in 'dirname' to the server to write them.\n\t\t\tEach subdirectory is visited recursively, until n files have been written.\n\t\t\tIf n is 0 or not specified, files are sent until the server replies by being full.\n\n\
 -W file1[,file2,...]\tSends the file given (or a list of files separated by commas) to the server to write.\n\n\
 -D dirname\t\tDirectory in which to write files the server sends this program in case of a capacity miss.\n\t\t\tThis option must be used with '-w' or '-W'.\n\t\t\tIf no directory is specified, the returned files will be discarded.\n\n\
 -r file1[,file2,...]\tReads the file given (or a list of files separated by commas) from the server.\n\n\
 -R [n=0]\t\tRead up to n files from the server.\n\t\t\tIf n is 0 or not specified, read all the files on the server instead.\n\n\
 -d dirname\t\tDirectory in which to write files read from the server.\n\t\t\tThis option must be used with '-r' or '-R'\n\t\t\tIf no directory is specified, the file read won't be written on disk.\n\n\
 -t time\t\tTime, in milliseconds, to wait before sending the next request to the server.\n\n\
 -l file1[,file2,...]\tTries to acquire exclusive access to the file given (or to a list of files separated by commas).\n\n\
 -u file1[,file2,...]\tRelinquish exclusive access to the file given (or to a list of files separated by commas).\n\n\
 -c file1[,file2,...]\tRemoves the file given (or list of files separated by commas) from the server.\n\n\
 -p\t\t\tEnables standard output for monitoring the application.\n"

#define PRINT_SET 1
#define SOCKET_SET 2
#define WRITE_SET 4
#define WRITE_DIR 8
#define READ_SET 16
#define READ_DIR 32

int main (int argc, char *argv[]) {
    char opt;
    unsigned int flags = 0;
    int my_fdSocket;
    struct sockaddr_un ServerSocketAddress;

    char test[100] = "nothing, really...";
    
    // POSSIBLE IDEA : VECTOR OF REQUESTS


    while ((opt = getopt(argc, argv, "hpf:w:W:D:r:R:d:t:l:u:c:")) != -1) {
        switch (opt) {  
            case 'h':
                fprintf(stdout, helpMsg);
                exit(EXIT_SUCCESS);
            case 'p':
                if (flags & PRINT_SET)
                    fprintf(stderr, "Print already set\n");
                else
                    fprintf(stdout, "Setting print to stdout\n");
                flags = flags | PRINT_SET;
                break;
            case 'f':
                if (flags & SOCKET_SET) {
                    fprintf(stderr, "Socket must be defined once and only once!\n");
                    exit(EXIT_FAILURE);
                }
                if (strlen((char*)optarg) > UNIX_PATH_MAX) {
                    fprintf(stderr, "Socket name too long!\n");
                    exit(EXIT_FAILURE);
                }
                strncpy(ServerSocketAddress.sun_path, (char*) optarg, UNIX_PATH_MAX);
                ServerSocketAddress.sun_family = AF_UNIX;
                flags = flags | SOCKET_SET;
                break;
            case 'w':
                if (flags & WRITE_SET)
                    fprintf(stdout, "New write-from-directory!\t");
                fprintf(stdout, "Option write-from-directory with: %s\t NOT_IMPLEMENTED\n", optarg);
                flags = flags | WRITE_SET;
                break;
            case 'W':
                if (flags & WRITE_SET)
                    fprintf(stdout, "New write files!\t");
                strncpy(test, optarg, 100);
                flags = flags | WRITE_SET;
                break;
            case 'D':
                if (flags & WRITE_DIR)
                    fprintf(stdout, "New discard directory!\t");
                fprintf(stdout, "Option discard-to-directory with: %s\t NOT_IMPLEMENTED\n", optarg);
                flags = flags | WRITE_DIR;
                break;
            case 'r':
                if (flags & READ_SET)
                    fprintf(stdout, "New read files!\t");
                fprintf(stdout, "Option read files with: %s\t NOT_IMPLEMENTED\n", optarg);
                flags = flags | READ_SET;
                break;
            case 'R':
                if (flags & READ_SET)
                    fprintf(stdout, "New read files!\t");
                fprintf(stdout, "Option read N files with: %s\t NOT_IMPLEMENTED\n", optarg);
                flags = flags | READ_SET;
                break;
            case 'd':
                if (flags & READ_DIR)
                    fprintf(stdout, "New read-to-directory!\t");
                fprintf(stdout, "Option read-to-directory with: %s\t NOT_IMPLEMENTED\n", optarg);
                flags = flags | READ_DIR;
                break;
            case 't':
                fprintf(stdout, "Option set time interval with: %d\t NOT_IMPLEMENTED\n", atoi(optarg));
                break;
            case 'l':
                fprintf(stdout, "Option lock files with: %s\t NOT_IMPLEMENTED\n", optarg);
                break;
            case 'u':
                fprintf(stdout, "Option unlock files with: %s\t NOT_IMPLEMENTED\n", optarg);
                break;
            case 'c':
                fprintf(stdout, "Option close files with: %s\t NOT_IMPLEMENTED\n", optarg);
                break;
            default:
                fprintf(stderr, "I'm confused. Opt = %c ..?\n", opt);
        }
        fprintf(stdout, "\t\tflags = %x\n", flags);
        fflush(stdout);
    }

    if (!(flags & SOCKET_SET))
        fprintf(stderr, "Must give a socket path! (unless help is called but that terminates otherwise)\n");

    if ((flags & WRITE_DIR) && !(flags & WRITE_SET))
        fprintf(stderr, "To discard-to-directory writing options ('-w' or '-W') are necessary!\n");
        
    if ((flags & READ_DIR) && !(flags & READ_SET))
        fprintf(stderr, "To read-to-directory read options ('-r' or '-R') are necessary!\n");
    
    // debug trying connecting to the socket!
    // socketAddr set in -f option
    my_fdSocket = socket(AF_UNIX, SOCK_STREAM, 0);

    while ( connect(my_fdSocket, (struct sockaddr*) &ServerSocketAddress, sizeof(ServerSocketAddress)) == -1) {
        if (errno == ENOENT) {
            fprintf(stderr, "Server doesn't exists\n");
            sleep(1);
        } else {
            perror("Connecting to server");
            exit(EXIT_FAILURE);
        }
        errno = 0;
    }
    fprintf(stdout, "Connected!\n");

    write(my_fdSocket, test, 100);

    close(my_fdSocket);

    exit(EXIT_SUCCESS);
}
