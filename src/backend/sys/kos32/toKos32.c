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

void toKos32(Bf *bf) {
    FILE *runtime = fopen("start.debug.bin", "rb");
    size_t runtimeSize = 0;
    size_t imageSize = 0;
    size_t codeSize = 0;
    char *code = NULL;
    char *image = NULL;
    
    // get size of runtime
    fseek(runtime, 0, SEEK_END);
    runtimeSize = ftell(runtime);
    rewind(runtime);
    // get code and its size
    toX86(bf, &codeSize, &code);
    if (!code) { printf("Error in %s:%d\n", __FILE__, __LINE__); return; }
    // now we can allocate enough memory for output file
    imageSize = runtimeSize + codeSize;
    image = malloc(imageSize);
    // copy runtime code
    fread(image, 1, runtimeSize, runtime);
    fclose(runtime);
    // copy brainfuck code
    memcpy(image + runtimeSize, code, codeSize);
    // configure image header
    Header *header = (Header *)image;
    header->sizeOfFile = imageSize;
    header->memoryNeed = imageSize + 1024;
    header->esp = header->memoryNeed;
    // write image to file
    FILE *fp = fopen("a.kex", "wb");
    fwrite(image, 1, imageSize, fp);
    fclose(fp);
}