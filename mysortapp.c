/*
 * Yuriy Koziy
 * CS1550
 * Multiprocess sorter: Tax records sorter
 */

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include "myRecordDef.h"

typedef struct{
    char *depth;
    char *file_name;
    char *attrib;
} args;

/* processes commnd line arguments */
int process_c_args(int argc, char *argv[], args *list);

/* reads the fifo and prints it*/
void read_fifo(int fd, int size);

/* read and print statistics file*/
void read_stat();

int main(int argc, char *argv[])
{
    args list;
    int fd;
    pid_t pid;
    int size = 0;
    int i = 1;
    int nodes_num = 0;
    char sz[25];
    char pipe_path[30];

    if(process_c_args(argc, argv, &list))
    {
        return -1;
    }

    size = strtol(list.file_name, NULL, 10);
    snprintf(sz, 25, "%d", size);

    if(mkfifo ("/tmp/1_p",0666) == -1)
    {
        perror("Error creating named pipe");
    }


    pid = fork();
    if(pid == 0) //child
    {
        //SM node arguments: lower bound, upper bound, sorting attribute, file name, node number, current depth, max depth
        execl("smnode", "smnode", "0", sz, list.attrib, list.file_name, "1", "0", list.depth, (char *)0);
        fflush(stdout);
        perror("Can't execute smnode\n");
    } else if (pid < 0) //fork failed
    {
        perror("Fork error!\n");
    } else //parent
    {
        fd = open("/tmp/1_p", O_RDONLY);
        read_fifo(fd,size);
        close(fd);
    }

    printf("\n\n ======================= Sorting Statistics ======================= \n\n");
    read_stat();
    printf("\n\n ================================================================== \n\n");

    //remove the pipes when finished
    nodes_num = (int)(pow(2,atoi(list.depth)+1)) - 1;
    for(i = 1; i <= nodes_num; i++)
    {
        snprintf(pipe_path, 30, "/tmp/%d_p", i);
        unlink(pipe_path);
    }
    return 0;
}

int process_c_args(int argc, char *argv[], args *list)
{
    int c;
    if(argc == 1)
    {
        printf("No arguments provided\n");
        printf("./mysortapp -d DepthOfBinTree(1-6) -f FileName -a AttributeNumber(0-3)\n");
        return -1;
    } else if(argc != 7)
    {
	printf("Not enough arguments provided\n");
	printf("./mysortapp -d DepthOfBinTree(1-6) -f FileName -a AttributeNumber(0-3)\n");
	return -1;
    }
    while((c = getopt (argc, argv, "d:f:a:")) != -1)
    {
        switch (c)
        {
            case 'd':
                list->depth = optarg;
                break;
            case 'f':
                list->file_name = optarg;
                break;
            case 'a':
                list->attrib = optarg;
                break;
            case '?':
                if (isprint (optopt))
                {
                    fprintf (stderr, "Unknown option `-%c'.\n", optopt);
                }
                else
                {
                    fprintf (stderr,"Unknown option character `\\x%x'.\n", optopt);
                    return -1;
                }
            default:
                return -1;
       }
    }

    int depth = atoi(list->depth);
    if(depth < 1 || depth  > 6)
    {
        printf("Depth of tree must be between 1 and 6\n");
        return -1;
    }

    int attrib = atoi(list->attrib);
    if(attrib < 0 || attrib > 3)
    {
        printf("Attribute number must be between 0 and 3\n");
        return -1;
    }
    return 0;
}

void read_fifo(int fd, int size)
{
	MyRecord record;
	int i = 0;
	int rd = 0;
	for(i = 0; i < size; i++)
	{
	    rd = read(fd, &record, sizeof(record));
	    if(rd < 0 || rd != sizeof(record))
	    {
	        perror("Reading failed!");
	        break;
	    }
        printf("%d %s %s %d \n",record.ssn, record.FirstName, record.LastName, record.income);
	}
	return;
}

void read_stat()
{
    int c;
    FILE *file;
    file = fopen("stat.txt", "r");
    if (file) {
        while ((c = getc(file)) != EOF)
            putchar(c);
        fclose(file);
    }
    remove("stat.txt");
    return;
}
