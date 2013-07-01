Yuriy Koziy
yuk30@pitt.edu
CS1550
Project 1: Parallel Sorting

Description:

	This project implements a sorting program which sorts a
	given file in divide and conquer fashion, by spawning
	multiple processes which sort a shorter range of input.

Files Included:

	README.txt
	Makefile
    	myRecordDef.h - header file which defines struct of each record
	qSort.c - implements quick sort.
	bSort.c - implements bubble sort.
	sSort.c - implements shell sort.
	soSort.c -- calls the unix sort.
	mysortapp.c - the main program.
	smnode.c - splitter and merger node.

Use Instruction:

	The program should be compiled by typing "make", Makefile
	has other usable functions such as "make clean" which cleans all the
	.o files and finally "make clean2" removes all the executables which 
	were created. Make sure that binary input files are in the same directory 
	before executing this program. 
	Program is executed by typing "./mysortapp -d depth -f fileName -a attribute"
	Where depth is a depth of binary tree and fileName name of file to sort and
	attribute specifies on which attribute to sort the file.
	
	Possible ranges for the parameters:
		-ddepth (1-6) -attribute (0-3) 0-ssn, 1-first name, 2-last name, 3-income

Known bugs/issues:
	NONE
