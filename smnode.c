/*
 * Yuriy Koziy
 * CS1550
 * Multiprocess sorter: Tax records sorter (splitter/merger node)
 */

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "myRecordDef.h"

 #define min(a,b) \
   ({typeof(a) _a = (a); \
       typeof(b) _b = (b); \
     _a < _b ? 1 : -1; })

typedef struct{
	int lower_bound;
	int upper_bound;
    int sort_attribute;
	int node_number;
	int current_depth;
	int max_depth;
} args;

typedef struct{
	char lower_bound_r[10];
	int lower_r;
	int lower_l;
    int upper_r;
	int upper_l;
	char lower_bound_l[10];
	char upper_bound_r[10];
	char upper_bound_l[10];
	char node_number_l[10];
	char node_number_r[10];
	char new_depth[10];
} newargs;

/* get command line args into struct */
int process_c_args(int argc, char *argv[], args *list);

/* get args for spawning children*/
void new_args(args oldargs, newargs *newargs);

/* split range for right side */
void split_r(int *ap, int *bp, int a, int b);

/* split range for left side */
void split_l(int *ap, int *bp, int a, int b);

/* responsible for creating the pipes for child processes */
void sm_node(int num);

/* start sorting algorithm based on sort_type */
void sort_start(int a, int b, int attrib, char *file, int num, char *sort_type);

/* pick appropriate sorting based on the number in i*/
void so_node(int a, int b, int attrib, char *file, int num, int i);

/* compare attributes within left and right struct */
int compare(MyRecord rA, MyRecord rB, int attrib);

/* merge two child pipes into parent pipe */
void merge(char *pipe1, char *pipe2, char *pipe0, int attrib);

int main(int argc, char *argv[], char *envp[])
{
    char left_pipe[30], right_pipe[30];
    char parent_pipe[30];
    args list;
    newargs newlist;

    if(process_c_args(argc, argv, &list))
    {
        return -1;
    }

    new_args(list, &newlist);

    snprintf(left_pipe, 30, "/tmp/%d_p", 2*list.node_number);
    snprintf(right_pipe, 30, "/tmp/%d_p", 2*list.node_number+1);

    sm_node(list.node_number);

    if(list.current_depth < (list.max_depth-1))
    {
        int left = fork();
        if(left < 0)
        {
            perror("Fork error!");
            return -1;
        }

        if(left == 0)
        {
            execl("smnode", "smnode", newlist.lower_bound_l, newlist.upper_bound_l, argv[3], argv[4], newlist.node_number_l, newlist.new_depth, argv[7], (char *)0);
            fflush(stdout);
        } else
        {
            int right = fork();

            if(right < 0)
            {
                perror("Fork error!");
                return -1;
            }

            if(right == 0)
            {
                execl("smnode", "smnode", newlist.lower_bound_r, newlist.upper_bound_r, argv[3], argv[4], newlist.node_number_r, newlist.new_depth, argv[7], (char *)0);
                fflush(stdout);
            } else
            {
                if(list.current_depth == 0)
                {
                    strncpy(parent_pipe, "/tmp/1_p", 30);
                } else
                {
                    snprintf(parent_pipe, 30, "/tmp/%d_p", list.node_number);
                }
                merge(left_pipe, right_pipe, parent_pipe, list.sort_attribute);
            }
        }
    } else
    {
        if (list.current_depth == 0)
        {
            strncpy(parent_pipe, "/tmp/1_p", 30);
        }
          else
        {
            snprintf(parent_pipe, 30, "/tmp/%d_p", list.node_number);
        }
        so_node(newlist.lower_l, newlist.upper_l, list.sort_attribute, argv[4], atoi(newlist.node_number_l), atoi(newlist.node_number_l)%4);
        so_node(newlist.lower_r, newlist.upper_r, list.sort_attribute, argv[4], atoi(newlist.node_number_r), atoi(newlist.node_number_r)%4);
        merge(left_pipe, right_pipe, parent_pipe, list.sort_attribute);
    }

    return 0;
}

int process_c_args(int argc, char *argv[], args *list)
{
    if(argc == 1)
    {
        printf("No arguments provided\n");
        return -1;
    }

    list->lower_bound = atoi(argv[1]);
    list->upper_bound = atoi(argv[2]);
    list->sort_attribute = atoi(argv[3]);
    list->node_number = atoi(argv[5]);
    list->current_depth = atoi(argv[6]);
    list->max_depth = atoi(argv[7]);

    if(list->lower_bound < 0 || list->upper_bound  < 0)
    {
        printf("Range can't be negative\n");
        return -1;
    }

    if(list->sort_attribute < 0 || list->sort_attribute > 3)
    {
        printf("Attribute number must be between 0 and 3\n");
        return -1;
    }

    if(list->node_number < 0)
    {
        printf("Node number cannot be negative\n");
        return -1;
    }

    if(list->max_depth < 1 || list->max_depth  > 6)
    {
        printf("Max depth of tree must be between 1 and 6\n");
        return -1;
    }

    return 0;
}

void new_args(args oldargs, newargs *newargs)
{
    int left_a, left_b;
    int right_a, right_b;

    split_l(&left_a, &left_b, oldargs.lower_bound, oldargs.upper_bound);
    newargs->lower_l = left_a;
    newargs->upper_l = left_b;

    split_r(&right_a, &right_b, left_b, oldargs.upper_bound);
    newargs->lower_r = right_a;
    newargs->upper_r = right_b;

    snprintf(newargs->lower_bound_l, 10, "%d", left_a);
    snprintf(newargs->upper_bound_l, 10, "%d", left_b);
    snprintf(newargs->lower_bound_r, 10, "%d", right_a);
    snprintf(newargs->upper_bound_r, 10, "%d", right_b);

    snprintf(newargs->node_number_l, 10, "%d", oldargs.node_number*2);
    snprintf(newargs->node_number_r, 10, "%d", oldargs.node_number*2+1);

    snprintf(newargs->new_depth, 10, "%d", oldargs.current_depth+1);

    return;
}

void split_l(int *ap, int *bp, int a, int b)
{
    *ap = a;
    *bp = (a+b)/2;

}

void split_r(int *ap, int *bp, int a, int b)
{
    *ap = a;
    *bp = b;
}

void sm_node(int num)
{
    char pipe1[30];
    char pipe2[30];
    snprintf(pipe1, 30, "/tmp/%d_p", 2*num);
    snprintf(pipe2, 30, "/tmp/%d_p", 2*num+1);

    if(mkfifo (pipe1,0666) == -1)
    {
        perror("Error creating named pipe");
    }

    if(mkfifo (pipe2,0666) == -1)
    {
        perror("Error creating named pipe");
    }
}

void sort_start(int a, int b, int attrib, char *file, int num, char *sort_type)
{
    char *arg[7];
    char ca[20];
    char cb[20];
    char ct[20];
    char file_name[50];
    char n[10];

    arg[0] = sort_type;
    snprintf(ca, 20, "-b%d", a);
    arg[1] = ca;

    snprintf(cb, 20, "-e%d", b);
    arg[2] = cb;

    snprintf(ct, 20, "-a%d", attrib);
    arg[3] = ct;

    snprintf(file_name, 50, "-f%s", file);
    arg[4] = file_name;

    snprintf(n, 10, "-n%d", num);
    arg[5] = n;

    arg[6] = NULL;
    if (fork() == 0)
    {
        execvp(arg[0], arg);
        fflush(stdout);
    }
}

void so_node(int a, int b, int attrib, char *file, int num, int i)
{
    switch(i)
    {
        case 0:
            sort_start(a, b, attrib, file, num, "./sSort");
            break;
        case 1:
            sort_start(a, b, attrib, file, num, "./qSort");
            break;
        case 2:
            sort_start(a, b, attrib, file, num, "./soSort");
            break;
        case 3:
            sort_start(a, b, attrib, file, num, "./bSort");
            break;
    }
}

void merge(char *pipe1, char *pipe2, char *pipe0, int attrib)
{
    int fd1 = 0;
    int fd2 = 0;
    int fd3 = 0;

    int rd1 = 0;
    int rd2 = 0;
    int minv = 0;
    MyRecord rA;
    MyRecord rB;

    fd3 = open(pipe0, O_WRONLY);
    if(fd3 < 0)
    {
        perror("Failed to open named pipe");
    }
    fd1 = open(pipe1, O_RDONLY);
    if(fd1 < 0)
    {
        perror("Failed to open named pipe");
    }
    fd2 = open(pipe2, O_RDONLY);
    if(fd2 < 0)
    {
        perror("Failed to open named pipe");
    }

    rd1 = read(fd1, &rA, sizeof(rA));
    rd2 = read(fd2, &rB, sizeof(rB));
    while((rd1 > 0) || (rd2 > 0))
    {
        if((rd1 == 0) || (rd2 == 0))
        {
            if(rd1 == 0)
            {
                write(fd3, &rB, sizeof(rB));
                rd2 = read(fd2, &rB, sizeof(rB));
            }else if(rd2 == 0)
            {
                write(fd3, &rA, sizeof(rA));
                rd1 = read(fd1, &rA, sizeof(rA));
            }
        } else
        {
            minv = compare(rA, rB, attrib);
            if(minv == 1)
            {
                write(fd3, &rA, sizeof(rA));
                rd1 = read(fd1, &rA, sizeof(rA));
            } else
            {
                write(fd3, &rB, sizeof(rB));
                rd2 = read(fd2, &rB, sizeof(rB));
            }
        }
    }
    close(fd3);
    close(fd1);
    close(fd2);
}

int compare(MyRecord rA, MyRecord rB, int attrib)
{
    //ssn
    if(attrib == 0)
    {
        //minv = min(rA.ssn,rB.ssn);
        if(min(rA.ssn,rB.ssn) > 0)
        {
            return 1;
        } else
        {
            return 2;
        }
    }

    //first name
    if(attrib == 1)
    {
        if(strcmp(rA.FirstName, rB.FirstName) >= 0)
        {
            return 2;
        } else
        {
           return 1;
        }
    }

    //last name
    if(attrib == 2)
    {
        if(strcmp(rA.LastName, rB.LastName) >= 0)
        {
            return 2;
        } else
        {
            return 1;
        }
    }

    //income
    if(attrib == 3)
    {
        if(min(rA.income,rB.income) > 0)
        {
            return 1;
        } else
        {
            return 2;
        }
    }
    return -1;
}


