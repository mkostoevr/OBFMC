#include <stdio.h>
#include <stdlib.h>

#include <obfmc.h>

#ifdef ONE_SOURCE
#   include "obfmc.c"
//#   include "backend/c/toC.c"
#   include "backend/kos32/toKos32.c"
#endif

int main(int argc, char **argv) {
    int errorCode = 0;
    Bf bf = { 0 };
    char *source = NULL;
    size_t sourceSize = 0;
    
    if (argc < 2) {
        puts("Usage: bfc source.bf");
        return -1;
    }
    {
        FILE *fp = fopen(argv[1], "r");
        
        fseek(fp, 0, SEEK_END);
        sourceSize = ftell(fp);
        rewind(fp);
        source = malloc(sourceSize + 1);
        source[sourceSize] = 0;
        fread(source, 1, sourceSize, fp);
        fclose(fp);
    }
    if ((errorCode = bfInit(&bf, sourceSize, source))) {
        printf("Error #%d\n", errorCode);
        return errorCode;
    }
    //toC(&bf);
    toKos32(&bf);
    return 0;
}
