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

void *pzip_thread(int argc, char** argv) {

    FILE *temp_file;
    char last_char = '\0';
    char this_char;
    int count = 1;

    // for all input files
    for (int i=0; i < (argc-1); i++) {
        // open and read one of the input files
        temp_file = fopen(argv[i+1], "r");

        // test if there is no more new file
        if (temp_file == NULL)
            printf("No such file.");

        // read the first character
        fread(&this_char, sizeof(char), 1, temp_file);

        // while it is not the end of the file
        while(!feof(temp_file)) {
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

    pthread_t t1, t2;
    printf("pzip: begin\n");
    // unable to input more than 1 arg into pzip_thread
    pthread_create(&t1, NULL, pzip_thread, argc, argv);
    pthread_create(&t2, NULL, pzip_thread, argc, argv);

    // join waits for the threads to finish
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    printf("pzip: end\n");

    return 0;
}