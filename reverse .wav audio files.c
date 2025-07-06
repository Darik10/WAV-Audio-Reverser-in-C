#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <input.wav>\n", argv[0]);
        return 1;
    }

    FILE *input = fopen(argv[1], "rb");
    if (!input) {
        printf("Error opening input file: %s\n", argv[1]);
        return 2;
    }

    FILE *output = fopen("output.wav", "wb");
    if (!output)
    {
        printf("Error opening output file for writing output.wav\n");
        fclose(input);
        return 3;
    }

    char check[4];
    long data_start = -1;

    // Read the file in 4-byte chunks
    while (fread(check, 1, 4, input) == 4) {
        if (memcmp(check, "data", 4) == 0) {
            data_start = ftell(input) - 4;  // Adjust to start of "data"
            printf("'data' chunk found at byte position: %ld\n", data_start);
            break;
        }
        // Move back 3 bytes to check overlapping sequences
        fseek(input, -3, SEEK_CUR);
    }

    if (data_start == -1) 
    {
        printf("'data' chunk not found in the file.\n");
        fclose(input);
        fclose(output);
        return 4; // Exit if no 'data' chunk found
   }
    
    int audio_start = data_start + 8;
    uint8_t *header = malloc(audio_start);
    if (!header)
    {
        printf("Memory allocation failled.\n");
        fclose(input);
        fclose(output);
        return 5;
    }
    fseek(input, 0, SEEK_SET);
    fread(header, 1, audio_start, input);
    fwrite(header, 1, audio_start, output);
    free(header);
    // Go the end of the file to get the file size
    fseek(input, 0, SEEK_END);
    long file_size = ftell(input);
    fseek(input, audio_start - 4, SEEK_SET);
    uint32_t data_size;
    fread(&data_size, sizeof(uint32_t), 1, input);
    if (data_size != (file_size - audio_start))
    {
        printf("Data size mismatch: expected %ld, found %u\n", file_size -audio_start, data_size);
        fclose(input);
        fclose(output);
        return 7;
    }
    uint16_t *buffer = malloc(data_size);
    if (!buffer)
    {
        printf("Memory allocation failed for buffer.\n");
        fclose(input);
        fclose(output);
        return 6;
    }
    printf("Current possition in file: %ld\n", ftell(input));
    fseek(input, audio_start, SEEK_SET);
    printf("Reading audio data from possition: %ld\n", ftell(input));
    long bytes_read = fread(buffer, sizeof(uint16_t), data_size/2, input);
    for (long i = bytes_read - 1; i >=0; i--)
    {
          fwrite(&buffer[i], sizeof(uint16_t), 1, output);
    }
    
    free(buffer);
    fclose(input);
    fclose(output);
    printf("Output written to output.wav\n");
    return 0;
}