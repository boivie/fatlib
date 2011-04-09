CC = gcc
CFLAGS = -ansi -pedantic -Wall -g 
CFLAGS += -O0
#CLAGS += -fprofile-arcs -ftest-coverage
LINKFLAGS = -Wl
LTP_GENHTML = genhtml

all:    src/fatdump

src/fatdump: src/fatdump.o src/fat.o src/fat16.o src/fat32.o
	$(CC) $(CFLAGS) $(LINKFLAGS) -o src/fatdump src/fatdump.o src/fat.o src/fat16.o src/fat32.o

src/fatdump.o: src/fatdump.c include/fat.h fat_conf.h
	$(CC) $(CFLAGS) -c src/fatdump.c -o src/fatdump.o

src/fat.o: src/fat.c include/fat.h fat_conf.h
	$(CC) $(CFLAGS) -c src/fat.c -o src/fat.o

src/fat16.o: src/fat16.c include/fat.h fat_conf.h
	$(CC) $(CFLAGS) -c src/fat16.c -o src/fat16.o

src/fat32.o: src/fat32.c include/fat.h fat_conf.h
	$(CC) $(CFLAGS) -c src/fat32.c -o src/fat32.o

ccov:	ccov-html

fat.info:	all
	@find . -name \*.gcda -o -name \*.da -o -name \*.bbg? | xargs rm -f
	$(LCOV) --directory . --zerocounters
	./tests/run.sh
	$(LCOV) --directory . --capture --output-file fat.info

ccov-html:	fat.info
	@mkdir -p ./ccov_html
	@$(LTP_GENHTML) --legend --output-directory ccov_html/ --title "FAT Code Coverage" --show-details fat.info

clean:
	@-rm src/*.o *~ src/core src/fatdump *.gcda *.da *-bbg? src/*.map

