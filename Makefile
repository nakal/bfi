
OPT_CFLAGS=-DBSD_OPTIMIZED $(CFLAGS)

COMPILER_FLAGS=-Wall -Wstrict-prototypes -pedantic $(OPT_CFLAGS)
LINKER_FLAGS=-L. $(LDFLAGS)

LIBRARY=libbfi.a
INTERPRETER=bfi

RANLIB=ranlib
AR=ar


all: make-libbfi compile-main
	$(CC) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(INTERPRETER) \
		main.o -lbfi

make-libbfi: $(LIBRARY)

$(LIBRARY): bfi.o
	$(AR) r $(LIBRARY) bfi.o
	$(RANLIB) $(LIBRARY)

bfi.o: bfi.c
	$(CC) -c $(COMPILER_FLAGS) -o bfi.o bfi.c

compile-main: main.o

main.o: main.c
	$(CC) -c $(COMPILER_FLAGS) -o main.o main.c

clean:
	rm -f *.o $(LIBRARY) $(INTERPRETER)

main.c: bfi.h

bfi.c: bfi.h

