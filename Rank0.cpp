#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <cstring>
#include <iostream>
#include <fstream>
#include <iomanip>

// #include "MPI.h"
// #include "ap_axi_sdata.h"

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

    // #pragma HLS resource core = AXI4Stream variable = stream_out
    // #pragma HLS resource core = AXI4Stream variable = stream_in
    // #pragma HLS DATA_PACK variable = stream_out
    // #pragma HLS DATA_PACK variable = stream_in

    int output_size_bytes = 255; // NEEDS TO BE UPDATED
    static int target_rank = 1;
    float number_commands = 1;
    float batch_size = 1;
    float num_ranks = 1;



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
    // example: dma_in at offset 0x00001000, prepare to dma_in 8 bytes as input image
    //          payload: (hex) 0000000008
    // example: dma_out at offset 0x00005000, prepare to dma_out 8 bytes as output feature map
    //          payload: (hex) 00005000 00000008
    float transaction_3[4] = {0x00005000, (float)input_size_bytes, 0x0000f000, float(output_size_bytes)};
    // assert(binary_content_check(filelen, input_buffer) == 0);



    // Prepare data for 1st transaction
    // higher  32 bits: size (bytes) in mem to dma_in
    // lower 32 bits: offset in mem to dma_in
    // example: dma_in at offset 0, prepare to dma_in 8 bytes
    // data: (hex) 41000000 (float 8 in hex) 00000000
    float transaction_1[2] = {(float)weights_size_bytes, 0.0}; // {size to dma in, offset}



    // Prepare data for 4th transaction
    // Readin command from binary file into float array
    // For reference purposes
    // int cmd_conv_len = 12;
    // int cmd_addr_len = 9;
    // int cmd_mode_len = 2;
    // int cmd_pool_len = 8;
    // int cmd_rsvd_len = 12;
    // int cmd_size_bytes = 128;
    // unsigned short int cmd_conv[cmd_conv_len] = {6, 6, 3, 3, 1, 0, 4, 4, 1, 1, 1, 1};
    // unsigned int cmd_addr[cmd_addr_len] = {2147483904, 36, 288, 0, 2148270080, 16, 2147491840, 72, 576};
    // unsigned short int cmd_mode[cmd_mode_len] = {0, 0};
    // unsigned short int cmd_pool[cmd_pool_len] = {4, 4, 0, 0, 0, 0, 0, 0};
    // unsigned int cmd_rsvd[cmd_rsvd_len] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    unsigned char *command_buffer;                    // DOUBLE CHECK
    fileptr = fopen("BIN/command.bin", "rb"); // Open the file in binary mode
    fseek(fileptr, 0, SEEK_END);              // Jump to the end of the file
    filelen = ftell(fileptr);                 // Get the current byte offset in the file
    rewind(fileptr);                          // Jump back to the beginning of the file

    int command_size_bytes = filelen * sizeof(unsigned char);
    command_buffer = (unsigned char *)malloc(command_size_bytes); // Enough memory for file + \0
    fread(command_buffer, filelen, 1, fileptr);           // Read in the entire file
    fclose(fileptr);                                      // Close the file

    assert(binary_content_check(filelen, command_buffer) == 0);

    float transaction_4[35];
    transaction_4[0] = number_commands;
    // Total size of command is 128 bytes = 32 floats
    // combining 4 bytes in command.bin into float
    // unsigned char current_float[4];
    // int command_bin_iter = 0;
    memcpy(&transaction_4[1], command_buffer, sizeof(command_buffer));
    // for (int i = 0; i < 32; i++)
    // {
        // for (int j = 0; j < 4; j++){
        //     current_float[j] = command_buffer[command_bin_iter + j];
        // }
        // command_bin_iter += 4;
        // memcpy(&transaction_4[i+1], command_buffer, sizeof(command_buffer));
        // printf("%d ", transaction_4[i + 1]);
        // for (int j = 0; j < 4; j++){
        //     printf("%d ", current_float[j]);
        // }
        // printf("\n");
    // }
    transaction_4[33] = batch_size;
    transaction_4[34] = num_ranks;

    for (int j = 0; j < 35; j++){
        printf("%d ", (int)transaction_4[j]);
    }



    // Transaction begins
    // while (!MPI_Send(transaction_1, 2, MPI_FLOAT, target_rank, 0, MPI_COMM_WORLD));
    // while (!MPI_Send(transaction_2, weights_size_bytes/4, MPI_FLOAT, target_rank, 0, MPI_COMM_WORLD));
    // while (!MPI_Send(transaction_3, input_size_bytes/4, MPI_FLOAT, target_rank, 0, MPI_COMM_WORLD));
    // while (!MPI_Send(transaction_4, 35, MPI_FLOAT, target_rank, 0, MPI_COMM_WORLD));

    return 0;


}