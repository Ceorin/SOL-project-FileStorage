CC = gcc
CFLAGS = -std=c99 -Wall -pedantic
CPPFLAGS = -pthread

# TODO? improve management of the project structure
CLIENTS = clients
API = communicationAPI

SERVER = server
SERVER_HD = server/headers

UTILITIES = utils
TEMP = tmp
LOG = log

SERVER_C_SRC := $(wildcard $(SERVER)/*.c)
SERVER_OBJ :=  $(SERVER_C_SRC:.c=.o)

.PHONY: clean cleanTmp cleanLog test1 test2 test3

all : serverApp.exe clientApp.exe

# SERVER dependencies
serverApp.exe : $(SERVER_OBJ)
	$(CC) -pthread $^ -o $@
	
	
$(SERVER_OBJ) :: CPPFLAGS += $(addprefix -iquote ,$(SERVER_HD))
$(SERVER_OBJ) :: $(wildcard $(SERVER)/%.c) $(wildcard $(SERVER_HD)/*.h)

# CLIENT dependencies
clientApp.exe : $(CLIENTS)/client.o $(API)/communicationAPI.o
	$(CC) -pthread $^ -o $@

$(CLIENTS)/client.o :: CPPFLAGS += $(addprefix -iquote ,$(API))	
$(CLIENTS)/client.o :: $(CLIENTS)/client.c $(API)/communicationAPI.h

# Why won't the implicit make the object here?
$(API)/communicationAPI.o : $(API)/communicationAPI.c $(API)/communicationAPI.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@
	
# TODO extra/utility folders dependencies?

# TODO better clean?
clean: cleanTmp cleanLog
	@echo "removing object files"
	rm -rf $(SERVER_OBJ)
	rm -rf $(wildcard $(CLIENTS)/*.o)
	rm -rf $(wildcard $(API)/*.o)
	@echo "removing executables"
	rm -rf $(wildcard *.exe)
	
cleanTmp: 
	@echo "removing temporary files"
	rm -rf $(wildcard $(TEMP)/*)
	
cleanLog:
	@echo "removing log files"
	rm -rf $(wildcard $(LOG)/*)
	
# TODO implement tests
test1:
	@echo "test1 not implemented"

test2:
	@echo "test2 not implemented"

test3:
	@echo "test3 not implemented"
