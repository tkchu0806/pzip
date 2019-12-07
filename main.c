/*
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your group information here.
 *
 * Group No.: 10 (Join a project group in Canvas)
 *
 * First member's full name: CHU Tsz Kit Chronos
 * First member's email address: tkchu5-c@my.cityu.edu.hk
 *
 * Second member's full name: Ng Chak Lam Rica
 * Second member's email address: clrng2-c@my.cityu.edu.hk
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

FILE *temp_file;
char last_char = '\0';
char this_char;
int count = 1;
int total_file_number;
char **temp_file_name;

void *czip_child_thread(char **argv) {

    // for all input files
    for (int i = 0; i < (total_file_number - 1); i++) {
        // open and read one of the input files
        temp_file = fopen(argv[i + 1], "r");

        // test if there is no more new file
        if (temp_file == NULL)
            printf("No such file.");

        // read the first character
        fread(&this_char, sizeof(char), 1, temp_file);

        // while it is not the end of the file
        while (!feof(temp_file)) {
            // add up to count the number of the same character
            if (last_char == this_char)
                count++;
            else {
                // if it is not the first character of the line
                // i.e. last character != this character but count > 1
                if (count >= 1 && last_char != '\0') {
                    // write and save the number of count
                    // write and save the corresponding character
                    // printf("%d%c", count, last_char);
                    fwrite(&count, sizeof(int), 1, stdout);
                    fwrite(&last_char, sizeof(char), 1, stdout);
                    count = 1;
                }
                last_char = this_char;
            }
            // read the next character
            fread(&this_char, sizeof(char), 1, temp_file);
        }
        // finish and close this file
        fclose(temp_file);
    }

    // Ensure everything is printed and saved
    // printf("%d%c", count, last_char);
    fwrite(&count, sizeof(int), 1, stdout);
    fwrite(&last_char, sizeof(char), 1, stdout);
}


int main(int argc, char **argv) {
    // argc 0 = the program name itself
    // argc 1 = the space next to the command line
    // argc 2 = file1
    // argc 3 = file2 ...
    if (argc < 2) {
        printf("czip: file1 [file2 ...]\n");
        return 1;
    }

//    printf("This system has %d processors configured and " "%d processors available.\n",
//           get_nprocs_conf(), get_nprocs());

    // Parent thread is the producer to divide the big files into several small parts.
    printf("pzip_parent_thread: begin\n");

    total_file_number = argc;
    temp_file_name = argv;

    // Child threads are the consumers to czip a part of large files divided by the parent thread in advance.
    pthread_t child_t1, child_t2;
    pthread_create(&child_t1, NULL, czip_child_thread, temp_file_name); //Create child thread t1
    pthread_create(&child_t2, NULL, czip_child_thread, temp_file_name); //Create child thread t2

    // join waits for the child threads to finish
    pthread_join(child_t1, NULL);
    pthread_join(child_t2, NULL);
    printf("pzip_parent_thread: end\n");

    return 0;
}