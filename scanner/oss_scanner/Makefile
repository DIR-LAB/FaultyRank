CPP = gcc
LINKERFLAG = -lm

all:
	$(CPP) oss_scanner.c -o oss_scanner $(LINKERFLAG)
	./ oss_scanner

.PHONY: clean
clean:
	rm -f oss_scanner
