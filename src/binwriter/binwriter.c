#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

static char *inputFileName  = NULL;
static char *outputFileName = NULL;

void printHelp()
{
    printf("Usage: binwriter inputFileName outputFileName\n");
    printf("Description:\n");
    printf("  Finds inputFileName and parses it for hexadecimal ascii. Only the first two\n");
    printf("  characters of each line is read, so everything after it is treated as a\n");
    printf("  comment. Make sure to include the leading zero of anything of value 0F or\n");
    printf("  less. Both capital and lowercase characters are supported. The parsed binary\n");
    printf("  will then be output into outputFileName.\n");
}

bool parseArgs(int argc, char* argv[])
{
    if (argc < 3)
    {
        printf("Insufficiant arguments\n\n");
        printHelp();
        return false;
    }
    else if (argc > 3)
    {
        printf("Too many arguments\n\n");
        printHelp();
        return false;
    }

    // Input file
    inputFileName = calloc(strlen(argv[1]) + 1, sizeof(char));
    snprintf(inputFileName, strlen(argv[1]) + 1, "%s", argv[1]);

    // Output file
    outputFileName = calloc(strlen(argv[2]) + 1, sizeof(char));
    snprintf(outputFileName, strlen(argv[2]) + 1, "%s", argv[2]);

    return true;
}

int8_t asciiHexToNumber(char character)
{
    switch (character)
    {
        case '0':
            return 0;
        case '1':
            return 1;
        case '2':
            return 2;
        case '3':
            return 3;
        case '4':
            return 4;
        case '5':
            return 5;
        case '6':
            return 6;
        case '7':
            return 7;
        case '8':
            return 8;
        case '9':
            return 9;
        case 'a':
        case 'A':
            return 10;
        case 'b':
        case 'B':
            return 11;
        case 'c':
        case 'C':
            return 12;
        case 'd':
        case 'D':
            return 13;
        case 'e':
        case 'E':
            return 14;
        case 'f':
        case 'F':
            return 15;
    }

    return -1;
}

bool parseNumber(char *buffer, uint8_t *output)
{
    int8_t msb = -1;
    int8_t lsb = -1;

    msb = asciiHexToNumber(buffer[0]);
    lsb = asciiHexToNumber(buffer[1]);

    if (-1 == msb ||
        -1 == lsb)
    {
        return false;
    }

    *output = (msb * 16) + lsb;

    return true;
}

bool parseFiles()
{
    FILE    *inputFile         = NULL;
    FILE    *outputFile        = NULL;
    char     inputBuffer[2048] = {0};
    uint8_t  parsedNumber      = 0;
    uint32_t lineNumber        = 0;

    inputFile = fopen(inputFileName, "r");

    if (NULL == inputFile)
    {
        printf("Failed to open input file \"%s\"\n", inputFileName);
        return false;
    }

    outputFile = fopen(outputFileName, "wb");

    if (NULL == outputFile)
    {
        printf("Failed to create output file \"%s\"\n", outputFileName);
        fclose(inputFile);
        return false;
    }

    while (fgets(inputBuffer, sizeof(inputBuffer), inputFile))
    {
        lineNumber++;
        if (true == parseNumber(inputBuffer, &parsedNumber))
        {
            fwrite(&parsedNumber, sizeof(uint8_t), 1, outputFile);
        }
        else
        {
            printf("Failed to parse line %u:\n%s\n", lineNumber, inputBuffer);
            fclose(inputFile);
            fclose(outputFile);
            return false;
        }
    }

    fclose(inputFile);
    fclose(outputFile);

    return true;
}

int main(int argc, char* argv[])
{
    if (false == parseArgs(argc, argv))
    {
        return -1;
    }

    if (false == parseFiles())
    {
        return -1;
    }

    printf("Success!\n");

    return 0;
}