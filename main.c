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

// This library is ignored as this header file is not found in MacOS environment.
// #include <sys/sysinfo.h>

FILE *temp_file;
char last_char = '\0';
char this_char;
int count = 1;
int total_file_number;
char **temp_file_name;

struct buffer_for_file {
    char *address;
    int part_size;
};

struct buffer_temp_output {
    char temp_output;
};

// =====================================================================================================================

//TODO: 1. Use mmap() function to replace fwrite(), fread(), etc.
void *czip_child_thread(char *temp_part_file_address) {

    // read the first character
    fread(&this_char, sizeof(char), 1, temp_part_file);

    // while it is not the end of the file
    while (!feof(temp_part_file)) {
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
        fread(&this_char, sizeof(char), 1, temp_part_file);
    }

    // Ensure everything is printed and saved
    // printf("%d%c", count, last_char);
    fwrite(&count, sizeof(int), 1, stdout);
    fwrite(&last_char, sizeof(char), 1, stdout);
}

// =====================================================================================================================

size_t getFileSize (const char* filename) {
    struct stat st;
    fstat(filename, &st);
    return st.st_size;
}

// =====================================================================================================================

void print_for_testing(struct buffer_for_file temp_buffer) {
    for (int j = 0; j < temp_buffer.part_size; j++) {
        printf("%c", temp_buffer.address[j]);
    }
    printf("\n<this part end>\n");
}

// =====================================================================================================================

int main(int argc, char **argv) {
    // argc 0 = the program name itself
    // argc 1 = the space next to the command line
    // argc 2 = file1
    // argc 3 = file2 ...
    if (argc < 2) {
        printf("czip: file1 [file2 ...]\n");
        return 1;
    }


    // -----------------------------------------------------------------------------------------------------------------

    // This part is used to find out the number of configured and available processors to determine
    // the maximum number of threads that can be created.

    // printf("This system has %d processors configured and " "%d processors available.\n",
    //         get_nprocs_conf(), get_nprocs());

    // However, since this is just a trivial problem, and only 2 threads are already enough to complete this project,
    // this part is archived. The focus should be on
    //                        1. mmap() function
    //                        2. mutex lock problem (where is the critical section?)

    // -----------------------------------------------------------------------------------------------------------------


    // Parent thread is the producer to divide the big files into several small parts.
    printf("pzip_parent_thread: begin\n");

    total_file_number = argc;
    temp_file_name = argv;

    // for all input files
    for (int i = 0; i < (total_file_number - 1); i++) {
        // open and read one of the input files
        temp_file = open(temp_file_name[i + 1], O_RDONLY, 0);

        // test if there is no more new file
        if (temp_file == NULL)
            printf("No such file.");

        // get the size of the temp_file
        size_t size_temp_file = getFileSize(temp_file);

        // Access the data mapped in the address space by using the pointer returned from the mmap() function
        char *pointer_to_temp_file = (char *) mmap(0, size_temp_file, PROT_READ, MAP_SHARED, temp_file, 0);

        // Obtain the 1st part of the input file
        struct buffer_for_file part_1_temp_file;
        part_1_temp_file.address = pointer_to_temp_file;
        part_1_temp_file.part_size = size_temp_file / 2;

        // Obtain the 2nd part of the input file
        struct buffer_for_file part_2_temp_file;
        part_2_temp_file.address = pointer_to_temp_file + size_temp_file / 2;
        part_2_temp_file.part_size = size_temp_file - size_temp_file / 2;

        // print each part of the input file for testing
        // print_for_testing(part_1_temp_file);
        // print_for_testing(part_2_temp_file);


        // Child threads are the consumers to czip a part of large files divided by the parent thread in advance.
        //TODO: 2. Solve mutex lock problem (where is the critical section?)
        pthread_t child_t1, child_t2;
        pthread_create(&child_t1, NULL, czip_child_thread, part_1_temp_file.address); //Create child thread t1
        pthread_create(&child_t2, NULL, czip_child_thread, part_2_temp_file.address); //Create child thread t2

        // join waits for the child threads to finish
        pthread_join(child_t1, NULL);
        pthread_join(child_t2, NULL);
        printf("pzip_parent_thread: end\n");

        // Complete reading the temp_file and close it
        close(temp_file);
    }

    return 0;
}