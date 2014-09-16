#include <stdio.h>

#define POLYNOMIAL 0xedb88320

static unsigned long table[256];

int main (int argc, const char * argv[])
{
FILE *pFile;
u8 *inBuffer[0x3F000];  //Length of data we wish to calculate CRC on.
u8 *outBuffer[0x40000];  //Total size of output (262144 bytes)

unsigned long state, nbytes;


    printf("CRC32 calculator. Embed the result in bin file at specified offset.\n");
    
    if( argc < 3 ) {
        showUsage(argv[0]);
        exit(1);
    }

    pFile = fopen(argv[1],"wb");
    if(pFile == NULL){
        printf("File open error.\n");
        exit(1);
    }
    fseek(pFile, 0, SEEK_END);
    if(ftell(pFile) != 262144){
        printf("File size error.\n");
        exit(1);
    }
    memcpy(inBuffer, pFile, 0x3F000);   //Copy data up to persistent settings location.
    nbytes = 0x3F000;
    memcpy(outBuffer, pFile, 0x40000);  //Copy input data for output file generation.
    calculate_table();

    while (nbytes > 0)
    {
        state = UPDC32(inBuffer, state);
        ++inBuffer;
        --nbytes;
    }




    exit(0);
}

void showUsage(char *progname) {
    printf("Usage:\n");
    printf("%s 'bios.bin' 'offset(hex)' 'output.bin'\n",progname);

}

void calculate_table(void)
{
    unsigned long v,b,i;

    for (unsigned b = 0; b < 256; ++b)
    {
        v = b;
        i = 8;
        for (; --i >= 0; )
            v = (v & 1) ? ((v >> 1) ^ POLYNOMIAL) : (v >> 1);
        table[b] = v;
    }
}

unsigned long UPDC32(unsigned char octet, unsigned long crc)
{
    // The original code had this as a #define
    return table[(crc ^ octet) & 0xFF] ^ (crc >> 8);
}


