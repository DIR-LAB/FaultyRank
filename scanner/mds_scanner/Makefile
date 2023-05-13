CPP = gcc
LINKERFLAG = -lm

all:
	$(CPP) mds_scanner.c -o mds_scanner $(LINKERFLAG)
	./ mds_scanner

.PHONY: clean
clean:
	rm -f mds_scanner