/*
 * Yuriy Koziy
 * CS1550
 * Program interface for unix sort
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

/* read binary and write to tmp text file*/
int read_write_txt(char *file_name, int size, int start, int num);

/* read text and write binary*/
int read_write_binary(int size, int num);

/* get command line args into struct */
int process_c_args(int argc, char *argv[], args *list);

void read_binary(int size);

int main(int argc, char *argv[], char *envp[])
{
    //timing vars
    double t1, t2, cpu_time;
    struct tms tb1, tb2;
    double ticspersec;

    FILE* out_file;
    args list;
    int size = 0;
    char cmd[100];

    if(process_c_args(argc, argv, &list))
    {
        return -1;
    }

    size = list.end - list.beg;

    if(read_write_txt(list.file_name, size, list.beg, list.num))
    {
       return -1;
    }

    ticspersec = (double) sysconf(_SC_CLK_TCK);
    t1 = (double)times(&tb1);

    switch (list.attrib)
    {
    case 0:
        snprintf(cmd, 100, "sort tmp%d.txt -k1 -n -o out%d.txt", list.num, list.num);
        system(cmd); //sort by ssn
        break;
    case 1:
        snprintf(cmd, 100, "sort tmp%d.txt -k2 -o out%d.txt", list.num, list.num);
        system(cmd); //sort by fname
        break;
    case 2:
        snprintf(cmd, 100, "sort tmp%d.txt -k3 -o out%d.txt", list.num, list.num);
        system(cmd); //sort by lname
        break;
    case 3:
        snprintf(cmd, 100, "sort tmp%d.txt -k4 -n -o out%d.txt", list.num, list.num);
        system(cmd); //sort by income
        break;
    default:
        break;
    }

    t2 = (double)times(&tb2);
    cpu_time = (double)((tb2.tms_utime + tb2.tms_stime) - (tb1.tms_utime + tb1.tms_stime));

    if(read_write_binary(size,list.num))
    {
        return -1;
    }

    //write timing to a file
    out_file = fopen("stat.txt", "a");
    if(fprintf(out_file, "SO took %lf sec, and used CPU for %lf sec. Range: %d-%d\n", (t2-t1)/ticspersec, cpu_time/ticspersec, list.beg, list.end) < 0)
    {
        printf("File writing failed to stat.txt!\n");
        return -1;
    }
    fclose(out_file);

    return 0;
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

int read_write_txt(char *file_name, int size, int start, int num)
{
    int i = 0;
    int fd = 0;
    int rb = 0;
    char f_name[20];
    MyRecord *records = NULL;
    int s = sizeof(records[0])*size;
    snprintf(f_name, 20, "tmp%d.txt", num);
    records = (MyRecord*)malloc(sizeof(MyRecord) * size);
    s = sizeof(records[0])*size;

    //read binary file
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

    FILE *file;
    file = fopen(f_name,"w");

	for(i = 0; i < size; i++)
	{
        fprintf(file,"%d %s %s %d\n", records[i].ssn, records[i].FirstName, records[i].LastName, records[i].income);
	}
    fclose(file);
    return 0;
}

int read_write_binary(int size, int num)
{
    int i = 0;
    FILE *file;
    int fd = 0;
    int wb = 0;
    char pipe_name[50];
    char f_name[20];
    char t_name[20];

    snprintf(f_name, 20, "out%d.txt", num);
    snprintf(t_name, 20, "tmp%d.txt", num);
    snprintf(pipe_name, 50, "/tmp/%d_p", num);
    MyRecord *record = NULL;
    int s = sizeof(record[0])*size;

    record = (MyRecord*)malloc(sizeof(MyRecord) * size);
    s = sizeof(record[0])*size;
    file = fopen(f_name,"r");

    if(file == NULL)
    {
    	perror("Unable to open file!");
    	return 1;
    }

    for(i = 0; i < size; i++)
    {
        if((fscanf(file,"%d %s %s %d", &record[i].ssn, record[i].FirstName, record[i].LastName, &record[i].income)) != 4)
        {
            perror("File in wrong format");
            return 1;
        }
    }
    fclose(file);

    //writing to binary
    fd = open(pipe_name, O_WRONLY);
    if (fd < 0)
    {
        perror("Unable to open file!");
        return -1;
    }

    wb = write(fd, &record[0], s);
    if(wb < 0 || wb != s )
    {
        perror("File writing failed");
        return -1;
    }

    //close(fd);
    remove(f_name);
    remove(t_name);
    return 0;
}

