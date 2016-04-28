COMMONSLIB=-lcommons

LIBDIR=/opt/csw/lib
LIBS=-L$(LIBDIR) $(COMMONSLIB) $(EXTRALDFLAGS)

OPTFLAGS=-g
CFLAGS=$(OPTFLAGS) -ansi 

CC=gcc

PROGS=wrapp


all: man umc exito

umc:
	$(CC) $(CFLAGS) -c  src/UMC.c  -o UMC.o
	$(CC) UMC.o $(LIBS) -o UMC
	rm *.o

clean:
	rm $($PROG) *.o
	rm -f UMC
	rm -f *.log
	rm -f *.gz

zip: 
	tar zcf tp.tar.gz src lib cfg

man:
	@echo "------------------------------------------------------------"
	@echo "------------------------------------------------------------"
	@echo "-----------------------WorstPractices-----------------------"
	@echo "------------------------------------------------------------"
	@echo "-------------------------elestac 1.0------------------------"
	@echo "------------------------------------------------------------"
	@echo "----------------------Archivo Makefile----------------------"
	@echo "------------------------------------------------------------"
	
exito:

	@echo "------------------------------------------------------------"
	@echo "-------------------Compilacion Generada---------------------"

#fin makefile