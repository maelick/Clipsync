# Options
DEBUG=yes
CC=g++
ifeq ($(DEBUG), yes)
	CFLAGS=-g
	LDFLAGS=-lPocoNet -lPocoCrypto -lPocoFoundation -lPocoUtil
else
	CFLAGS=-W -Wall
	LDFLAGS=-lPocoNet -lPocoCrypto -lPocoFoundation -lPocoUtil
endif

# Libs

# Executables
EXEC=clipsync.exe

# Rules
all: $(EXEC)
ifeq ($(DEBUG), yes)
	@echo "Debug Mode"
endif

# Dependencies
clipsync.exe: broadcaster.o config.o peer_handler.o clipboard_manager.o local_frontend.o

# Rules (cont'd)
%.exe: %.o
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CC) -o $@ -c $< $(CFLAGS)

.PHONY: clean mrproper

clean:
	@rm *.o

mrproper: clean
	@rm $(EXEC)
