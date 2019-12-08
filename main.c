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
#include <assert.h>

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
    int temp_count;
    char temp_char;
};

struct buffer_temp_output buffer_temp_output_array[];

// =====================================================================================================================

void *czip_child_thread(struct buffer_for_file temp_part_file) {

    struct buffer_temp_output temp_output;
    struct buffer_temp_output local_temp_output_array[temp_part_file.part_size];
    int array_pointer = 0;

    // scan for the entire temp_part_file
    for (int j = 0; j < temp_part_file.part_size; j++) {
        this_char = temp_part_file.address[j];
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
                temp_output.temp_count = count;
                temp_output.temp_char = last_char;
                local_temp_output_array[array_pointer++] = temp_output;
                count = 1;
            }
            last_char = this_char;
        }
    }

    for (int n = 0; n < sizeof(local_temp_output_array); n++) {
        buffer_temp_output_array[n] = local_temp_output_array[n];
    }
}

// =====================================================================================================================

size_t getFileSize (const char* filename) {
    struct stat st;
    fstat(filename, &st);
    return st.st_size;
}

// =====================================================================================================================

void print_for_testing(struct buffer_for_file temp_buffer) {
    for (int k = 0; k < temp_buffer.part_size; k++) {
        printf("%c", temp_buffer.address[k]);
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
        // Check if the input file is successfully opened and read
        assert(temp_file != -1);

        // test if there is no more new file
        if (temp_file == NULL)
            printf("No such file.");

        // get the size of the temp_file
        size_t size_temp_file = getFileSize(temp_file);

        // Access the data mapped in the address space by using the pointer returned from the mmap() function
        char *pointer_to_temp_file = (char *) mmap(0, size_temp_file, PROT_READ, MAP_SHARED, temp_file, 0);
        // Check if the data is successfully mapped
        assert(pointer_to_temp_file != MAP_FAILED);

        // Obtain the 1st part of the input file
        struct buffer_for_file part_1_temp_file;
        part_1_temp_file.address = pointer_to_temp_file;
        part_1_temp_file.part_size = size_temp_file / 2;

        // Obtain the 2nd part of the input file
        struct buffer_for_file part_2_temp_file;
        part_2_temp_file.address = pointer_to_temp_file + size_temp_file / 2;
        part_2_temp_file.part_size = size_temp_file - size_temp_file / 2;

        // print each part of the input file for testing
        print_for_testing(part_1_temp_file);
        print_for_testing(part_2_temp_file);

        // Testing for single thread
        czip_child_thread(part_1_temp_file);
        czip_child_thread(part_2_temp_file);

        // Child threads are the consumers to czip a part of large files divided by the parent thread in advance.
        //TODO: 2. Solve mutex lock problem (where is the critical section?)
//        pthread_t child_t1, child_t2;
//        pthread_create(&child_t1, NULL, czip_child_thread, part_1_temp_file.address); //Create child thread t1
//        pthread_create(&child_t2, NULL, czip_child_thread, part_2_temp_file.address); //Create child thread t2

        // join waits for the child threads to finish
//        pthread_join(child_t1, NULL);
//        pthread_join(child_t2, NULL);
//        printf("pzip_parent_thread: end\n");


        // Print all count and char
        for (int m = 0; m < sizeof(buffer_temp_output_array); m++) {
            // printf("%d%c", temp_output_array[l].temp_count, temp_output_array[l].temp_char);
            fwrite(&buffer_temp_output_array[m].temp_count, sizeof(int), 1, stdout);
            fwrite(&buffer_temp_output_array[m].temp_char, sizeof(char), 1, stdout);
        }

        // Cleanup and unmap
        int rc = munmap(pointer_to_temp_file, size_temp_file);
        // Check if it's successfully unmapped to clean up
        assert(rc == 0);

        // Complete reading the temp_file and close it
        close(temp_file);
    }

    return 0;
}