/*
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your group information here.
 *
 * Group No.: 26 (Join a project group in Canvas)
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

#define BUFFER_SIZE 70000

struct buffer_for_file {
    char *address;
    int part_size;
};

struct buffer_output {
    int buffer_count;
    char buffer_char;
};

struct buffer_child_output {
    struct buffer_output buffer_child_output_array[BUFFER_SIZE];
    int buffer_child_array_length;
};

// =====================================================================================================================

void *czip_child_thread(void *arg) {

    struct buffer_output temp_output;
    struct buffer_child_output *local_child_final_output = NULL;
    struct buffer_for_file *temp_part_file = (struct buffer_for_file *) arg;
    int array_length = 0;
    char last_char = '\0';
    char this_char;
    int count = 1;

    // ** Dynamic Memory Allocation **
    // Allocate a space to save the local_child_final_output
    // such that the result will not disappear after the end of this function.
    while (local_child_final_output == NULL) {
        local_child_final_output = malloc(sizeof(struct buffer_child_output));
    }

    // scan for the entire temp_part_file
    for (int j = 0; j < temp_part_file->part_size; j++) {
        this_char = temp_part_file->address[j];
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
                temp_output.buffer_count = count;
                temp_output.buffer_char = last_char;
                local_child_final_output->buffer_child_output_array[array_length++] = temp_output;
                count = 1;
            }
            last_char = this_char;
        }
    }

    // Ensure everything is saved
    temp_output.buffer_count = count;
    temp_output.buffer_char = last_char;
    local_child_final_output->buffer_child_output_array[array_length++] = temp_output;
    local_child_final_output->buffer_child_array_length = array_length;

    return local_child_final_output;
}

// =====================================================================================================================

int main(int argc, char **argv) {
    // argc 0 = the program name itself
    // argc 1 = the space next to the command line
    // argc 2 = file1
    // argc 3 = file2 ...
    if (argc < 2) {
        printf("pzip: file1 [file2 ...]\n");
        return 1;
    }

    // -----------------------------------------------------------------------------------------------------------------

    // Parent thread is the producer to divide the big files into several small parts.

    int total_file_number = argc;
    char **temp_file_name = argv;

    struct buffer_child_output *temp_child_pointer_array[2];
    struct buffer_output global_temp_output_array[BUFFER_SIZE];
    int global_temp_output_array_length = 0;

    // for all input files
    for (int i = 0; i < (total_file_number - 1); i++) {
        // open and read one of the input files
        int temp_file = open(temp_file_name[i + 1], O_RDONLY);
        // Check if the input file is successfully opened and read
        // assert(temp_file != -1);

        // get the size of the temp_file
        struct stat st;
        fstat(temp_file, &st);
        size_t size_temp_file = st.st_size;

        // Access the data mapped in the address space by using the pointer returned from the mmap() function
        char *pointer_to_temp_file = (char *) mmap(0, size_temp_file, PROT_READ, MAP_SHARED, temp_file, 0);
        // Check if the data is successfully mapped
        // assert(pointer_to_temp_file != MAP_FAILED);

        // Obtain the 1st part of the input file
        struct buffer_for_file part_1_temp_file;
        part_1_temp_file.address = pointer_to_temp_file;
        part_1_temp_file.part_size = size_temp_file / 2;

        // Obtain the 2nd part of the input file
        struct buffer_for_file part_2_temp_file;
        part_2_temp_file.address = pointer_to_temp_file + size_temp_file / 2;
        part_2_temp_file.part_size = size_temp_file - size_temp_file / 2;


        // Child threads are the consumers to czip a part of large files divided by the parent thread in advance.

        // 2 child threads work simultaneously to czip their parts
        // Mutex Lock is not needed as they work on 2 different parts of 1 single large file, and
        // they will save their results in separate locations concurrently. No deadlock will happen.
        pthread_t child_t1, child_t2;
        pthread_create(&child_t1, NULL, czip_child_thread, &part_1_temp_file); //Create child thread t1, start czip
        pthread_create(&child_t2, NULL, czip_child_thread, &part_2_temp_file); //Create child thread t2, start czip

        // 2 child threads save their results in temp_child_pointer_array[] concurrently and respectively
        // join waits for the child threads to finish
        pthread_join(child_t1, (void **) &(temp_child_pointer_array[0]));
        pthread_join(child_t2, (void **) &(temp_child_pointer_array[1]));

        // Cleanup and unmap
//      int rc = munmap(pointer_to_temp_file, size_temp_file);
        // Check if it's successfully unmapped to clean up
//      assert(rc == 0);

        // Complete reading the temp_file and close it
        close(temp_file);

        // -------------------------------------------------------------------------------------------------------------

        //combine results from 2 child threads before reading the next file

        //TODO: 3 problems
        // 1) unable to continue to save multiple files results into global_temp_output_array for Test 2
        //      (previous file result will be covered by that of new files)
        // 2) unexpected differences between my and expected results at the end of the standard output in Test 4
        // 3) Little differences between my and expected results in Test 5-9

        // Save the results returned from the 1st child thread first
        for (int n = 0; n < temp_child_pointer_array[0]->buffer_child_array_length; n++) {
            global_temp_output_array[n] = temp_child_pointer_array[0]->buffer_child_output_array[n];
            global_temp_output_array_length++;
        }

        // Prepare to save the results returned from the 2nd child thread
        int array_starting_location = temp_child_pointer_array[0]->buffer_child_array_length;

        // If the last character from the 1st child thread result is
        // the same as the first character from the 2nd child thread result,
        // combine their count
        if (global_temp_output_array[array_starting_location - 1].buffer_char ==
            temp_child_pointer_array[1]->buffer_child_output_array[0].buffer_char) {
            global_temp_output_array[array_starting_location -
                                     1].buffer_count += temp_child_pointer_array[1]->buffer_child_output_array[0].buffer_count;
        }

        // Save the results returned from the 2nd child thread
        for (int p = 0; p < temp_child_pointer_array[1]->buffer_child_array_length; p++) {

            // check if it is the last element of the array
            if (p == global_temp_output_array_length - 1) {
                break;
            }

            global_temp_output_array[array_starting_location +
                                     p] = temp_child_pointer_array[1]->buffer_child_output_array[p + 1];
            global_temp_output_array_length++;
        }
    }

    // -----------------------------------------------------------------------------------------------------------------

    // Finally, print all count and char from the final_output
    // for all count and char results from the global_temp_output_array
    for (int m = 0; m < global_temp_output_array_length; m++) {

        // write the current character
        fwrite(&(global_temp_output_array[m].buffer_count), sizeof(int), 1, stdout);
        fwrite(&(global_temp_output_array[m].buffer_char), sizeof(char), 1, stdout);
    }

    return 0;
}