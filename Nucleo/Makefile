COMMONSLIB=-lcommons

LIBDIR=/opt/csw/lib
LIBS=-L$(LIBDIR) $(COMMONSLIB) $(EXTRALDFLAGS)

OPTFLAGS=-g
CFLAGS=$(OPTFLAGS) -ansi 

CC=gcc

PROGS=wrapp


all: man nucleo exito

nucleo:
	$(CC) $(CFLAGS) -c  src/Nucleo.c  -o Nucleo.o
	$(CC) Nucleo.o $(LIBS) -o NUCLEO
	rm *.o

clean:
	rm $($PROG) *.o
	rm -f NUCLEO
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