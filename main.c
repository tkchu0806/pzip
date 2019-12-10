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

// This library is ignored as this header file is not found in MacOS environment.
// #include <sys/sysinfo.h>

// This constant buffer size is large enough to pass all tests
#define BUFFER_SIZE 70000

struct buffer_for_file {
    char *address;
    int part_size;
};

struct buffer_output {
    int count;
    char byte;
};

struct buffer_child_output {
    struct buffer_output buffer[BUFFER_SIZE];
    int length;
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

    this_char = temp_part_file->address[0];

    // scan for the entire temp_part_file
    int j;
    for (j = 1; j < temp_part_file->part_size; j++) {
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
                temp_output.count = count;
                temp_output.byte = last_char;
                local_child_final_output->buffer[array_length++] = temp_output;
                count = 1;
            }
            last_char = this_char;
        }
        this_char = temp_part_file->address[j];
    }

    // To count the last character which is the same as the previous character at the end of the file.
    if (last_char == this_char) {
        count++;
    }

    // Ensure everything is saved
    temp_output.count = count;
    temp_output.byte = last_char;
    local_child_final_output->buffer[array_length++] = temp_output;
    local_child_final_output->length = array_length;

    // To include '\n' at the end of the file
    if (this_char == '\n') {
        local_child_final_output->buffer[array_length].byte = this_char;
        local_child_final_output->buffer[array_length].count = 1;
        local_child_final_output->length = ++array_length;
    }

    // To include last character (this_char) of the file which is different from the previous character (last_char),
    // but it must not be a '\n' at the end of the file
    if (last_char != this_char && this_char != '\n') {
        local_child_final_output->buffer[array_length].byte = this_char;
        local_child_final_output->buffer[array_length].count = 1;
        local_child_final_output->length = ++array_length;
    }

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

    struct buffer_child_output *children_result[2];
    struct buffer_output final_array[BUFFER_SIZE];
    int final_length = 0;

    // for all input files
    for (int i = 0; i < (total_file_number - 1); i++) {
        // open and read one of the input files
        int temp_file = open(temp_file_name[i + 1], O_RDONLY);
        // Check if the input file is successfully opened and read
        if (temp_file == -1) {
            return 1;
        }

        // get the size of the temp_file
        struct stat st;
        fstat(temp_file, &st);
        size_t size_temp_file = st.st_size;

        // Access the data mapped in the address space by using the pointer returned from the mmap() function
        char *pointer_to_temp_file = (char *) mmap(0, size_temp_file, PROT_READ, MAP_SHARED, temp_file, 0);
        // Check if the data is successfully mapped
        if (pointer_to_temp_file == MAP_FAILED) {
            return 1;
        }

        // Obtain the 1st part of the input file
        struct buffer_for_file *part_1_temp_file = NULL;

        // ** Dynamic Memory Allocation **
        while (part_1_temp_file == NULL) {
            part_1_temp_file = malloc(sizeof(struct buffer_for_file));
        }

        // Part 1 starts pointing and scanning later at the beginning of the the file
        // The file size is the half of the input file
        part_1_temp_file->address = pointer_to_temp_file;
        part_1_temp_file->part_size = size_temp_file / 2;

        // Obtain the 2nd part of the input file
        struct buffer_for_file *part_2_temp_file = NULL;

        // ** Dynamic Memory Allocation **
        while (part_2_temp_file == NULL) {
            part_2_temp_file = malloc(sizeof(struct buffer_for_file));
        }

        // Part 2 starts pointing and scanning later at the half of the file
        // The file size is the remaining half of the input file
        part_2_temp_file->address = pointer_to_temp_file + size_temp_file / 2;
        part_2_temp_file->part_size = size_temp_file - size_temp_file / 2;


        // Child threads are the consumers to czip a part of large files divided by the parent thread in advance.

        // 2 child threads work simultaneously to czip their parts
        // Mutex Lock is not needed as they work on 2 different parts of 1 single large file, and
        // they will save their results in separate locations concurrently. No deadlock will happen.
        pthread_t child_t1, child_t2;
        pthread_create(&child_t1, NULL, czip_child_thread, part_1_temp_file); //Create child thread t1, start czip
        pthread_create(&child_t2, NULL, czip_child_thread, part_2_temp_file); //Create child thread t2, start czip

        // 2 child threads save their results in children_result[] concurrently and respectively
        // join waits for the child threads to finish
        pthread_join(child_t1, (void **) &(children_result[0]));
        pthread_join(child_t2, (void **) &(children_result[1]));

        // Complete reading the temp_file and close it
        close(temp_file);

        // -------------------------------------------------------------------------------------------------------------

        // Combine results from 2 child threads before reading the next file
        // Flow of logic:
        // 1) children_result[0] + children_result[1] --> parent_result
        // 2) final_array + parent_result --> final_array

        struct buffer_child_output parent_result;
        parent_result.length = 0;

        // Flow 1) begins

        // A boolean indicator to show if the results returned by the 2 child threads should be combined.
        // => Check if the last char of 1st child result is the same as the first char of 2nd child result
        // => False = 0; True = 1 (There is no boolean variable in C programming)
        // e.g. result of thread 1 = 2a3b; result of thread 2 = 3b4c
        //      then their results should be merged as 2a6b4c in the parent
        int should_combine =
                children_result[0]->buffer[children_result[0]->length - 1].byte ==
                children_result[1]->buffer[0].byte;
        int j, k, new_parent_length;

        // If children results should be combined, the parent length should be shorter by 1
        // e.g. thread 1 = 2a3b (length = 2); thread 2 = 3b4c (length = 2)
        //      parent result = 2a6b4c (length = 3)
        if (should_combine) {
            new_parent_length = children_result[0]->length + children_result[1]->length - 1;
        } else {
            new_parent_length = children_result[0]->length + children_result[1]->length;
        }

        // Copy first child thread results into the parent
        for (j = 0; j < children_result[0]->length; j++) {
            parent_result.buffer[j] = children_result[0]->buffer[j];
        }

        // Update the parent length after copying the first child result
        parent_result.length = j;

        // If children results should be combined, their count should be added up
        // e.g. thread 1 = 2a3b; thread 2 = 3b4c
        //      parent result = 2a6b4c (count of b = 3+3 = 6)
        if (should_combine) {
            parent_result.buffer[children_result[0]->length - 1].count += children_result[1]->buffer[0].count;

            // if the last result from second child thread is '\n', it should be saved to parent as well.
            if (children_result[1]->buffer[1].byte == '\n') {
                parent_result.buffer[children_result[0]->length].byte = '\n';
                parent_result.buffer[children_result[0]->length].count = 1;
            } else {
                //copy second child thread results into the parent
                for (j = 1; j < children_result[1]->length; j++) {
                    // copy into empty space after last element
                    parent_result.buffer[parent_result.length + j - 1] = children_result[1]->buffer[j];
                }
            }
        } else {
            //copy second child thread results into the parent
            for (j = 0; j < children_result[1]->length; j++) {
                // copy into empty space after last element
                parent_result.buffer[parent_result.length + j] = children_result[1]->buffer[j];
            }
        }

        // Update the parent length after copying both the child results
        parent_result.length = new_parent_length;

        // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

        // Flow 2) begins

        // No need to combine if the final_array is empty
        // Otherwise, need to combine IF the last char of final array is the same as the first char of parent
        if (final_length == 0) {
            should_combine = 0; // False
        } else {
            should_combine = final_array[final_length - 1].byte == parent_result.buffer[0].byte;
        }

        // if should combine, add up their count and length (final array length should be shorter by 1)
        if (should_combine) {
            final_array[final_length - 1].count += parent_result.buffer[0].count;
            final_length = final_length + parent_result.length - 1;
            //copy parent result into the final array
            for (k = 1; k < parent_result.length; k++) {
                // copy into empty space after last element
                final_array[final_length + k - 1] = parent_result.buffer[k];
            }
        } else {
            //copy parent result into the final array
            for (k = 0; k < parent_result.length; k++) {
                // copy into empty space after last element
                final_array[final_length + k] = parent_result.buffer[k];
            }
            final_length += parent_result.length;
        }
    }  // End of reading and zipping all files

    // -----------------------------------------------------------------------------------------------------------------

    // Finally, print all count and char from the final_output
    // for all count and char results from the final_array
    for (int m = 0; m < final_length; m++) {

        // write the current character
        fwrite(&(final_array[m].count), sizeof(int), 1, stdout);
        fwrite(&(final_array[m].byte), sizeof(char), 1, stdout);
    }

    return 0;
}