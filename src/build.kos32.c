#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <obfmc.h>

typedef struct {
    char magic[8];
    uint32_t version;
    uint32_t entry;
    uint32_t sizeOfFile;
    uint32_t memoryNeed;
    uint32_t esp;
    uint32_t params;
    uint32_t argv0;
} Header;

int buildKos32(Bf *restrict bf, const char *restrict name) {
    int errorCode = 0;
    FILE *runtime = NULL;
    size_t runtimeSize = 0;
    size_t imageSize = 0;
    size_t codeSize = 0;
    char *code = NULL;
    char *image = NULL;
    
    if (!(runtime = fopen("runtime/kos32.bin", "rb"))) { return ERROR_CAN_NOT_OPEN_RUNTIME; }
    // get size of runtime
    fseek(runtime, 0, SEEK_END);
    runtimeSize = ftell(runtime);
    rewind(runtime);
    // get code and its size
    if ((errorCode = genI386(bf, &codeSize, &code))) { return errorCode; }
    // now we can allocate enough memory for output file
    imageSize = runtimeSize + codeSize;
    if (!(image = malloc(imageSize))) { return ERROR_OUT_OF_MEMORY; }
    // copy runtime code
    fread(image, 1, runtimeSize, runtime);
    fclose(runtime);
    // copy brainfuck code
    memcpy(image + runtimeSize, code, codeSize);
    // configure image header
    //! FIXME: UB
    Header *header = (Header *)image;
    header->sizeOfFile = imageSize;
    header->memoryNeed = imageSize + 1024;
    header->esp = header->memoryNeed;
    // write image to file
    {
        char newName[strlen(name) + 5];
        FILE *fp = NULL;

        strcpy(newName, name);
        strcat(newName, ".kex");
        fp = fopen(newName, "wb");
        if (!fp) { return ERROR_OUTPUT_FILE_IS_NOT_CREATED; }
        fwrite(image, 1, imageSize, fp);
        fclose(fp);
    }
    return 0;
}