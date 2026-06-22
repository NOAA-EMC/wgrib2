/**
 * This is a test for the gmerge helper function rd_msg().
 * 
 * Alyson Stahl
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "aux_progs.h"

int
main()
{
    printf("Testing gmerge helper function rd_msg()...\n");
    printf("EOF on first read. Should return -1.");
    {
        FILE *input, *output;
        int result;

        // Create empty file
        input = fopen("test_empty.tmp", "wb");
        fclose(input);
        
        input = fopen("test_empty.tmp", "rb");
        output = fopen("test_output.tmp", "wb");
        result = rd_msg(input, output);

        fclose(input);
        fclose(output);

        remove("test_empty.tmp");
        remove("test_output.tmp");

        if (result != -1) return 1;
    }
    printf("ok!\n");
    printf("Invalid GRIB header. Should return -1.");
    {
        FILE *input, *output;
        unsigned char bad_data[16]; 
        int result;
        
        memset(bad_data, 0, 16);
        strcpy((char*)bad_data, "BADHEADER");

        input = fopen("test_bad.tmp", "wb");
        fwrite(bad_data, 1, 16, input);
        fclose(input);
        
        input = fopen("test_bad.tmp", "rb");
        output = fopen("test_output.tmp", "wb");
        
        result = rd_msg(input, output);

        fclose(input);
        fclose(output);
        remove("test_bad.tmp");
        remove("test_output.tmp");

        if (result != -1) return 2;
    }
    printf("ok!\n");
    printf("Message too large (> 1GB). Should return -1.");
    {
        FILE *input, *output;
        unsigned char large_header[16]; 
        int result;
        
        memset(large_header, 0, 16);
        strcpy((char*)large_header, "GRIB");
        // Set message length to > 1GB
        large_header[8] = 0x00;
        large_header[9] = 0x00;
        large_header[10] = 0x00;
        large_header[11] = 0x40; // 1GB = 0x40000000
        large_header[12] = 0x00;
        large_header[13] = 0x00;
        large_header[14] = 0x00;
        large_header[15] = 0x00;

        input = fopen("test_large.tmp", "wb");
        fwrite(large_header, 1, 16, input);
        fclose(input);
        
        input = fopen("test_large.tmp", "rb");
        output = fopen("test_output.tmp", "wb");
        
        result = rd_msg(input, output);

        fclose(input);
        fclose(output);
        remove("test_large.tmp");
        remove("test_output.tmp");

        if (result != -1) return 3;
    }
    printf("ok!\n");
    printf("Incomplete message read. Should return -1.");
    {
        FILE *input, *output;
        unsigned char incomplete_header[16]; 
        int result;
        
        memset(incomplete_header, 0, 16);
        strcpy((char*)incomplete_header, "GRIB");
        // Set message length to 32 bytes
        incomplete_header[15] = 32; 

        input = fopen("test_incomplete.tmp", "wb");
        fwrite(incomplete_header, 1, 16, input);
        // Write only 8 more bytes instead of 16
        unsigned char partial_data[8] = {0};
        fwrite(partial_data, 1, 8, input);
        fclose(input);
        
        input = fopen("test_incomplete.tmp", "rb");
        output = fopen("test_output.tmp", "wb");
        
        result = rd_msg(input, output);

        fclose(input);
        fclose(output);
        remove("test_incomplete.tmp");
        remove("test_output.tmp");

        if (result != -1) return 4;
    }
    printf("ok!\n");
    printf("Valid GRIB message. Should return 0.");
    {
        FILE *input, *output;
        unsigned char valid_header[16]; 
        int result;
        
        memset(valid_header, 0, 16);
        strcpy((char*)valid_header, "GRIB");
        // Set message length to 32 bytes
        valid_header[15] = 32; 
        
        input = fopen("test_valid.tmp", "wb");
        fwrite(valid_header, 1, 16, input);
        // Write 16 more bytes of data
        unsigned char data[16] = {0};
        fwrite(data, 1, 16, input);
        fclose(input);
        
        input = fopen("test_valid.tmp", "rb");
        output = fopen("test_output.tmp", "wb");
        
        result = rd_msg(input, output);

        fclose(input);
        fclose(output);
        remove("test_valid.tmp");
        remove("test_output.tmp");

        if (result != 0) return 5;
    }
    printf("ok!\n");
    printf("SUCCESS!\n");
    return 0;
}
