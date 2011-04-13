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
EXEC=p2pmain.exe poco_encrypt_test.exe

# Rules
all: $(EXEC)
ifeq ($(DEBUG), yes)
	@echo "Debug Mode"
endif

# Dependencies
p2pmain.exe: broadcaster.o p2p.o config.o p2p_handler.o

# Rules (cont'd)
%.exe: %.o
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CC) -o $@ -c $< $(CFLAGS)

.PHONY: clean mrproper

clean:
	@rm *.o *~

mrproper: clean
	@rm $(EXEC)
