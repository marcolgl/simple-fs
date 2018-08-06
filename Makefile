CCOPTS= -Wall -g -std=gnu99 -Wstrict-prototypes
LIBS= 
CC=gcc
AR=ar


BINS= simplefs_test\
	disk_driver_test\
	simplefs_test_weird_cases

OBJS = #add here your object files

HEADERS=bitmap.h\
	disk_driver.h\
	simplefs.h\
	utility.h

%.o:	%.c $(HEADERS)
	$(CC) $(CCOPTS) -c -o $@  $<

.phony: clean all


all:	$(BINS) 

simplefs_test: simplefs_test.o simplefs.o disk_driver.o bitmap.o $(OBJS)
	$(CC) $(CCOPTS)  -o $@ $^ $(LIBS)
disk_driver_test: disk_driver_test.o simplefs.o disk_driver.o bitmap.o $(OBJS)
	$(CC) $(CCOPTS)  -o $@ $^ $(LIBS)
simplefs_test_weird_cases: simplefs_test_weird_cases.o simplefs.o disk_driver.o bitmap.o $(OBJS)
	$(CC) $(CCOPTS)  -o $@ $^ $(LIBS)


clean:
	rm -rf *.o *~  $(BINS)
