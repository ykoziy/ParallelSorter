CC=gcc 
CFLAGS=-Wall -m32
LDFLAGS=-m32 -pthread
LDLIBS = -lm

all: bSort qSort sSort soSort smnode mysortapp clean

bSort: bSort.o
bSort.o: bSort.c myRecordDef.h

qSort: qSort.o
qSort.o: qSort.c myRecordDef.h

sSort: sSort.o
sSort.o: sSort.c myRecordDef.h

soSort: soSort.o
soSort.o: soSort.c myRecordDef.h

smnode: smnode.o
smnode.o: smnode.c myRecordDef.h

mysortapp: mysortapp.o
mysortapp.o: mysortapp.c

clean:	
	rm -rf *o
clean2:
	rm -f bSort qSort sSort soSort smnode mysortapp
