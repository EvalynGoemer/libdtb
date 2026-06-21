CC := clang
CFLAGS := -std=c99 -D_POSIX_C_SOURCE=200809L -O2 -Wall -Wextra -Wcast-qual
CXXFLAGS := -std=c++98 -Wall -Wextra -Wcast-qual -xc++
DTB := rp6-dpad-bottom.dtb

all: decomp standalone find-console

headers:
	./comp2header.pl

decomp: headers
	$(CC) $(CFLAGS) -o dtb-decomp examples/decomp.c

standalone: headers
	$(CC) $(CFLAGS) -o examples/standalone examples/standalone.c lib/libdtb0.c lib/llist.c lib/libdtb.c

find-console: headers
	$(CC) $(CFLAGS) -o examples/find-console examples/find_console.c

cpp: headers
	clang++ $(CXXFLAGS) -o dtb-decomp examples/decomp.c

clean:
	rm -f dtb-decomp libdtb_singleheader.h libdtb0_singleheader.h
	rm -f examples/standalone examples/find-console
