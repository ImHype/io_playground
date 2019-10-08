.PHONY: block-server
.PHONY: nonblock-server
.PHONY: threads
.PHONY: block-multithreading
.PHONY: select-loop-server
.PHONY: poll-loop-server
.PHONY: epoll-loop-server
.PHONY: block-multithreading-io

cat:
	@$(CC) ./src/files/cat.c -o cat.o; ./cat.o;

fifo:
	@rm -rf a.fifo; mkfifo a.fifo; $(CC) ./src/files/cat-fifo.c -I ./include -o fifo.o; ./fifo.o


%.o:./src/server/%.c
	$(CC) $< ./src/toolkit/hash_map.c ./src/toolkit/loop-poll.c ./src/toolkit/loop-select.c -std=c99 -lpthread -I ./include -o $@; ./$@;

block-server:
	@$(MAKE) block-normal.o;

nonblock-server:
	@$(MAKE) nonblock-loop.o;

threads:
	@$(MAKE) block-multithreading.o;

block-multithreading:
	@$(MAKE) block-multithreading.o;

block-multithreading-io:
	@$(MAKE) block-multithreading-io.o;

select-loop-server:
	@$(MAKE) select-loop-server.o;

poll-loop-server:
	@$(MAKE) poll-loop-server.o;

epoll-loop-server:
	@$(CC) ./src/server/epoll-loop-server.c ./src/toolkit/hash_map.c ./src/toolkit/loop-kqueue.c  -std=c99 -lpthread -I ./include -o ./epoll.o; ./epoll.o;

clear:
	rm -rf *.o