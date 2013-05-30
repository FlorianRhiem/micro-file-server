server: server.c mime.data.c mime.c connection.c
	$(CC) -Wall -Wextra -std=c89 -pedantic $^ -o $@ #-static-libgcc -static -lc

all: thirdparty/lib/libminiupnpc.a
MINIUPNPC_URL=http://miniupnp.free.fr/files/download.php?file=miniupnpc-1.8.tar.gz

thirdparty:
	mkdir -p thirdparty
thirdparty/libminiupnpc: thirdparty/libminiupnpc.tar.gz
	rm -rf thirdparty/miniupnpc-*
	cd thirdparty; tar xzf libminiupnpc.tar.gz
	rm -rf $@
	mv thirdparty/miniupnpc-* $@
thirdparty/libminiupnpc.tar.gz: $(filter-out $(wildcard thirdparty), thirdparty)
	wget --no-verbose $(MINIUPNPC_URL) -O $@
thirdparty/lib/libminiupnpc.a: thirdparty/libminiupnpc
	make -C thirdparty/libminiupnpc
	INSTALLPREFIX=.. make -C thirdparty/libminiupnpc install
	rm -rf thirdparty/share
	rm -rf thirdparty/bin
	rm -rf thirdparty/lib/libminiupnpc.so*
clean:
	rm -rf thirdparty

.PHONY: clean all
