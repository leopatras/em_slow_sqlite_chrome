CFLAGS=-fPIC -Wall -I. -O2 -DNDEBUG -std=c99 -DSQLITE_OMIT_LOAD_EXTENSION -DSQLITE_THREADSAFE=0
CC=emcc
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)
HDRS=$(wildcard *.h)

#startup option for gwasrv
export BROWSER=chrome

all: main.html


main.html: $(OBJS)
	$(CC) \
          -s MAIN_MODULE=1 \
          -O2 \
          -s EXPORT_ALL=1 \
          -s EXIT_RUNTIME=1 \
          -s ASSERTIONS=1 \
          -s FORCE_FILESYSTEM=1 \
          -s ALLOW_MEMORY_GROWTH=1 \
          -s ASYNCIFY \
	  -lidbfs.js \
          -o main.html $(OBJS)




%.o : %.c 
	$(CC) $(CFLAGS) -c $<

$(OBJS): $(HDRS)

gwa.run: all
	gwasrv main.html


clean:
	rm -f *.o main.html main.js main.wasm

.PHONY: gwa.run clean
