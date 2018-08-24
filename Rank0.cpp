#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <cstring>
#include <iostream>
#include <fstream>
#include <iomanip>

#include "MPI.h"
#include "ap_axi_sdata.h"

using namespace std;

int binary_content_check(int filelen, unsigned char * buffer){
    int indentation = 0;
    int group = 0;
    unsigned int row_number = 0;
    printf("%08x: ", row_number);
    for (int i = 0; i < filelen; i++){
        if (indentation == 16){
            row_number += 16;
            printf("\n");
            printf("%08x: ", row_number);
            indentation = 0;
        }
        indentation++;
        if (group%2 != 0)
            printf("%02x ", buffer[i]);
        else
            printf("%02x", buffer[i]);
        group++;
    }
    printf("\n");
    return 0;
}

int main()
{
    float output_size_bytes = 255;			// NEEDS TO BE UPDATED
	int final_output_size_bytes = 255;		// NEED TO BE MODIFIED
    static int target_rank = 1;
    float number_commands = 1;
	unsigned char number_commands_byte[4] =  {0, 0, 0, 1 };
    float batch_size = 1.0;
    float num_ranks = 1.0;



    // Prepare weight data for 2nd transaction
    FILE *fileptr;
    // Readin data into byte array
    unsigned char *weights_buffer;
    int filelen;

    fileptr = fopen("BIN/weights_reshape.bin", "rb"); // Open the file in binary mode
    fseek(fileptr, 0, SEEK_END);                      // Jump to the end of the file
    filelen = ftell(fileptr);                         // Get the current byte offset in the file
    rewind(fileptr);                                  // Jump back to the beginning of the file

    int weights_size_bytes = filelen * sizeof(unsigned char);
    weights_buffer = (unsigned char *)malloc(weights_size_bytes); // Enough memory for file + \0
    fread(weights_buffer, filelen, 1, fileptr);          // Read in the entire file
    fclose(fileptr);                                     // Close the file
    // assert(binary_content_check(filelen, weights_buffer) == 0);
    


    // Prepare input feature map data for 3rd transaction
    // Readin data into byte array
    unsigned char *input_buffer;
    fileptr = fopen("BIN/input_reshape.bin", "rb"); // Open the file in binary mode
    fseek(fileptr, 0, SEEK_END);                    // Jump to the end of the file
    filelen = ftell(fileptr);                       // Get the current byte offset in the file
    rewind(fileptr);                                // Jump back to the beginning of the file

    int input_size_bytes = filelen * sizeof(unsigned char);
    input_buffer = (unsigned char *)malloc(input_size_bytes); // Enough memory for file + \0
    fread(input_buffer, filelen, 1, fileptr);        // Read in the entire file
    fclose(fileptr);                                 // Close the file

    // setup mem offset, size for input image dma_in and output feature map dma_out, payload 4 floats
    // 1st flit: higher 32 bits: offset in mem to dma_in
    //           lower  32 bits: size (bytes) in mem to dma_in
    // 2nd flit: higher 32 bits: offset in mem to dma_out
    //           lower  32 bits: size (bytes) in mem to dma_out
    // example: dma_in at offset 0x80000100, prepare to dma_in (input_size_bytes) bytes as input image
    // example: dma_out at offset 0x00f00000, prepare to dma_out (output_size_bytes) bytes as output feature map
	// NOTE: ADRESS HERE NEEDS TO BE CONSISTENT WITH THE COMMAND ADDRESS FIELD
    float transaction_3[4] = {(float)0x80000100, (float)input_size_bytes, (float)0x00f00000, output_size_bytes};
    // assert(binary_content_check(filelen, input_buffer) == 0);



    // Prepare data for 1st transaction
    // first  32 bits: offset in mem to dma_in
    // second  32 bits: size (bytes) in mem to dma_in
    // example: dma_in at offset 0x800C0000, prepare to dma_in (weights_size_bytes) bytes
    float transaction_1[2] = {(float)0x800C0000, (float)weights_size_bytes}; // {size to dma in, offset}



    // Prepare data for 4th transaction
    // Readin command from binary file into float array
    // For reference purposes
    const int cmd_conv_len = 12;
    const int cmd_addr_len = 9;
    const int cmd_mode_len = 2;
    const int cmd_pool_len = 8;
    const int cmd_rsvd_len = 12;
    const int cmd_size_bytes = 128;
    unsigned short int cmd_conv[cmd_conv_len] = {6, 6, 3, 3, 1, 0, 4, 4, 1, 1, 1, 1};
    unsigned int cmd_addr[cmd_addr_len] = {2147483904, 36, 288, 0, 2148270080, 16, 2147491840, 72, 576};
    unsigned short int cmd_mode[cmd_mode_len] = {0, 0};
    unsigned short int cmd_pool[cmd_pool_len] = {4, 4, 0, 0, 0, 0, 0, 0};
    unsigned int cmd_rsvd[cmd_rsvd_len] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    unsigned char *command_buffer;                    // DOUBLE CHECK
    fileptr = fopen("BIN/command.bin", "rb"); // Open the file in binary mode
    fseek(fileptr, 0, SEEK_END);              // Jump to the end of the file
    filelen = ftell(fileptr);                 // Get the current byte offset in the file, filelen is 128 fixed
    rewind(fileptr);                          // Jump back to the beginning of the file

	// add 16 bytes to store the extra 3 float
    int command_size_bytes = (filelen + 8) * sizeof(unsigned char);
    command_buffer = (unsigned char *)malloc(command_size_bytes); // Enough memory for file + \0
    fread(command_buffer+4, filelen, 1, fileptr);           // Read in the entire file
    fclose(fileptr);                                      // Close the file

	// assign number_commands at the start
	for (int iter = 0; iter < 4; iter++) {
		command_buffer[iter] = number_commands_byte[iter];
	}
    //assert(binary_content_check(filelen + 12, command_buffer) == 0);
	

    //prepare 5th commands, batch_size and num_ranks
    float transaction_5[2] = {batch_size, num_ranks};

    // Transaction begins
    while (!MPI_Send(transaction_1, 2, MPI_FLOAT, target_rank, 0, MPI_COMM_WORLD));
	// DMA in weights
    while (!MPI_Send(weights_buffer, weights_size_bytes/4, MPI_FLOAT, target_rank, 0, MPI_COMM_WORLD));
    while (!MPI_Send(transaction_3, input_size_bytes/4, MPI_FLOAT, target_rank, 0, MPI_COMM_WORLD));
    while (!MPI_Send(command_buffer, 33, MPI_FLOAT, target_rank, 0, MPI_COMM_WORLD));
	while (!MPI_Send(transaction_5, 2, MPI_FLOAT, target_rank, 0, MPI_COMM_WORLD));
	// Accumulated cycle count from rank 0 is 0
	while (!MPI_Send((float)0, 1, MPI_FLOAT, target_rank, 0, MPI_COMM_WORLD));
	// DMA in input feature map
	while (!MPI_Send(input_buffer, input_size_bytes/4, MPI_FLOAT, target_rank, 0, MPI_COMM_WORLD));

	// Wait for receive
	float accumulated_cycle_count;
	while (!MPI_Recv(&accumulated_cycle_count, 1, MPI_FLOAT, 0, 0 /*not used*/, MPI_COMM_WORLD /*not used*/));

	unsigned char *final_output_bytes;
	final_output_bytes = (unsigned char *)malloc(final_output_size_bytes);
	while (!MPI_Recv(final_output_bytes, final_output_size_bytes, MPI_FLOAT, 0, 0 /*not used*/, MPI_COMM_WORLD /*not used*/));


    return 0;


}
