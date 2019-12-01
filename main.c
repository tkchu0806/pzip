/*
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your group information here.
 *
 * Group No.: 10 (Join a project group in Canvas)
 *
 * First member's full name: CHU Tsz Kit
 * First member's email address: tkchu5-c@my.cityu.edu.hk
 *
 * Second member's full name: (leave blank if none)
 * Second member's email address: (leave blank if none)
 *
 * Third member's full name: (leave blank if none)
 * Third member's email address: (leave blank if none)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
// #include <sys/sysinfo.h>

// Thread creation
//void *mythread(void * arg) {
//    printf ("%s\n", (char *) arg);
//    return NULL;
//}

int main(int argc, char** argv)
{
    // argc 0 = the program name itself
    // argc 1 = the space next to the command line
    // argc 2 = file1
    // argc 3 = file2 ...
    if (argc<2) {
        printf("czip: file1 [file2 ...]\n");
        return 1;
    }

//    // create threads
//    pthread_t p1, p2;
//    int rc;
//    printf("main: begin\n");
//    pthread_create(&p1, NULL, mythread, "A");
//    pthread_create(&p2, NULL, mythread, "B");
//
//    // join waits for the threads to finish
//    pthread_join(p1, NULL);
//    pthread_join(p2, NULL);
//    printf("main: end\n");
//
    return 0;
}