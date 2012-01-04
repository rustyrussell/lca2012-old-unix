AS=matts-binutils/bin/pdp11-aout-as
CFLAGS=-Os
LDFLAGS=-s

all: cat-ported-forward gnu-cat-reduced

cat-ported-forward: unix-cat-rewritten-in-c.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

clean:
	rm -f cat-ported-forward gnu-cat-reduced

cat.o: cat.s
	$(AS) -o $@ cat.s

unix-cat-modified-for-gas.o: unix-cat-modified-for-gas.s
	$(AS) -o $@ unix-cat-modified-for-gas.s
