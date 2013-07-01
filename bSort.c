/*
 * Yuriy Koziy
 * CS1550
 * Program implements bubble sort
 * Command line options:
 * -b is start of range
 * -e is end of range
 * -a is an attribute number 0 to 3
 * -f is filename string
 * -n is node number
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/times.h>
#include <ctype.h>
#include <string.h>
#include <fcntl.h>
#include "myRecordDef.h"

typedef struct{
	int beg;
	int end;
    int attrib;
	char *file_name;
	int num;
} args;

/* read file into struct */
int read_file(MyRecord *records, char *file_name, int size, int start);

/* write struct into named_pipe */
int write_file(MyRecord *records, int size, int num);

/* get command line args into struct */
int process_c_args(int argc, char *argv[], args *list);

/* swap used in sorting algortihm */
void swap(MyRecord* a, MyRecord* b);

/* sort by SSN */
void bubblesort_ssn(MyRecord* records, int size);

/* sort by income */
void bubblesort_income(MyRecord* records, int size);

/* sort by first name */
void bubblesort_fname(MyRecord* records, int size);

/* sort by last name*/
void bubblesort_lname(MyRecord* records, int size);

/* array of function pointers for quick acces to different versions of sort*/
void (*func_ptr[4])(MyRecord* r, int s) = {bubblesort_ssn, bubblesort_fname, bubblesort_lname, bubblesort_income};

int main(int argc, char *argv[], char *envp[])
{
    //timing vars
    double t1, t2, cpu_time;
    struct tms tb1, tb2;
    double ticspersec;

    int size = 0;
    MyRecord *records = NULL;
    FILE* out_file;
    args list;

    if(process_c_args(argc, argv, &list))
    {
        return -1;
    }

    size = list.end - list.beg;
    records = (MyRecord*)malloc(sizeof(MyRecord) * size);

    if(read_file(records, list.file_name, size, list.beg))
    {
       return -1;
    }
   // -b 0 -e 100 -a 0 -f 100records -s 1
    ticspersec = (double) sysconf(_SC_CLK_TCK);
    t1 = (double)times(&tb1);

    (*func_ptr[list.attrib])(records, size); // do sorting

    t2 = (double)times(&tb2);
    cpu_time = (double)((tb2.tms_utime + tb2.tms_stime) - (tb1.tms_utime + tb1.tms_stime));

    if(write_file(records, size, list.num))
    {
        return -1;
    }

    free(records);

    //write timing to a file
    out_file = fopen("stat.txt", "a");
    if(fprintf(out_file, "BS took %lf sec, and used CPU for %lf sec. Range: %d-%d\n", (t2-t1)/ticspersec, cpu_time/ticspersec, list.beg, list.end) < 0)
    {
        printf("File writing failed to stat.txt!\n");
        return -1;
    }
    fclose(out_file);

    return 0;
}

void swap(MyRecord* a, MyRecord* b)
{
    MyRecord temp = *a;
    *a = *b;
    *b = temp;
}

void bubblesort_ssn(MyRecord* records, int size)
{
    int i = 0;
    int j = 0;
    for (i = (size - 1); i > 0; i--)
    {
        for (j = 1; j <= i; j++)
        {
            if (records[j-1].ssn > records[j].ssn)
            {
                swap(&records[j-1], &records[j]);
            }
        }
    }
}

void bubblesort_income(MyRecord* records, int size)
{
    int i = 0;
    int j = 0;
    for (i = (size - 1); i > 0; i--)
    {
        for (j = 1; j <= i; j++)
        {
            if (records[j-1].income > records[j].income)
            {
                swap(&records[j-1], &records[j]);
            }
        }
    }
}

void bubblesort_fname(MyRecord* records, int size)
{
    int i = 0;
    int j = 0;
    for (i = (size - 1); i > 0; i--)
    {
        for (j = 1; j <= i; j++)
        {
            if (strcmp(records[j-1].FirstName, records[j].FirstName) > 0)
            {
                swap(&records[j-1], &records[j]);
            }
        }
    }
}

void bubblesort_lname(MyRecord* records, int size)
{
    int i = 0;
    int j = 0;
    for (i = (size - 1); i > 0; i--)
    {
        for (j = 1; j <= i; j++)
        {
            if (strcmp(records[j-1].LastName, records[j].LastName) > 0)
            {
                swap(&records[j-1], &records[j]);
            }
        }
    }
}

int process_c_args(int argc, char *argv[], args *list)
{
    int c;
    if(argc == 1)
    {
        printf("No arguments provided\n");
        return -1;
    }
    //process command line args
    while((c = getopt (argc, argv, "b:e:a:f:n:")) != -1)
    {
        switch (c)
        {
            case 'b':
                list->beg = atoi(optarg);
                break;
            case 'e':
                list->end = atoi(optarg);
                break;
            case 'a':
                list->attrib = atoi(optarg);
                break;
            case 'f':
                list->file_name = optarg;
                break;
            case 'n':
                list->num = atoi(optarg);
                break;
            case '?':
                if (isprint(optopt))
                {
                    fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                }
                else
                {
                    fprintf(stderr,"Unknown option character `\\x%x'.\n", optopt);
                    return -1;
                }
            default:
                return -1;
       }
    }

    if(list->beg < 0 || list->end  < 0)
    {
        printf("Range can't be negative\n");
        return -1;
    }

    if(list->attrib < 0 || list->attrib > 3)
    {
        printf("Attribute number must be between 0 and 3\n");
        return -1;
    }

    if(list->num < 0)
    {
        printf("Node number cannot be negative\n");
        return -1;
    }
    return 0;
}

int read_file(MyRecord *records, char *file_name, int size, int start)
{
    int fd;
    int rb;
    int s = sizeof(records[0])*size;

    fd = open(file_name, O_RDONLY);
    if (fd < 0)
    {
        perror("Unable to open file!");
        return -1;
    }

    if(lseek(fd,sizeof(records[0])*start,SEEK_SET) < 0)
    {
        perror("Lseek error!");
        return -1;
    }

    rb = read(fd, &records[0], s);
    if(rb < 0 || rb != s )
    {
        perror("File reading failed!");
        return -1;
    }

    close(fd);
    return 0;
}

int write_file(MyRecord *records, int size, int num)
{
    int fd;
    int wb;
    int s = sizeof(records[0])*size;
    char pipe_name[50];
    snprintf(pipe_name, 50, "/tmp/%d_p", num);

    fd = open(pipe_name, O_WRONLY);

    if (fd < 0)
    {
        perror("Unable to open file!");
        return -1;
    }

    wb = write(fd, &records[0], s);
    if(wb < 0 || wb != s )
    {
        printf("File writing failed to %s!\n",pipe_name);
        return -1;
    }
    //close(fd);
    return 0;
}
