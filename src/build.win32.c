#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <obfmc.h>

unsigned aligned(unsigned n, int align) {
    return (n + align-1) & ~(align-1);
}

int getOffsetOfSpecialMark(char *code, char *max) {
    int size = 0;

    while (code < max &&
        memcmp(code, "PLACE_FOR_BRAINFUCK_CODE", strlen("PLACE_FOR_BRAINFUCK_CODE")))
        { code++; size++; }
    if (code >= max) { return 0; }
    return size;
}

//! ACHTUNG: Look at "HARDCODED" achtungs before runtime modification
int buildWin32(Bf *restrict bf, const char *restrict name) {
    int errorCode = 0;
    int runtimeSize = 0;
    int imageSize = 0;
    char *image = NULL;
    size_t codeSize = 0;
    char *code = NULL;

    {
        FILE *runtime = NULL;
        
        if (!(runtime = fopen("runtime/win32.bin", "rb"))) { return ERROR_CAN_NOT_OPEN_RUNTIME; }
        // get size of runtime
        fseek(runtime, 0, SEEK_END);
        runtimeSize = ftell(runtime);
        rewind(runtime);
        // get code and its size
        if ((errorCode = genI386(bf, &codeSize, &code))) { return errorCode; }
        // allocate enough memory for runtime (which is aligned and so have space for our code)
        // possilbly (if app code will be large) we will need expand image later
        imageSize = runtimeSize;
        if (!(image = malloc(imageSize))) { return ERROR_OUT_OF_MEMORY; }
        // copy runtime code
        fread(image, 1, runtimeSize, runtime);
        fclose(runtime);
    }
    {
        int codeSectionOffset = 0;
        int sizeOfRuntimeCode = 0;
        int maxCodeInSection = 0;
        int codeOffset = 0;

        // check if we need to expand image
        codeSectionOffset = *(int *)(image + 0x1b4); //! ACHTUNG: HARDCODED
        if (!(sizeOfRuntimeCode = getOffsetOfSpecialMark(image + codeSectionOffset,
            image + runtimeSize))) { return ERROR_PLACE_FOR_CODE_NOT_FOUND; }
        codeOffset = codeSectionOffset + sizeOfRuntimeCode;
        maxCodeInSection = 0x200 - sizeOfRuntimeCode;
        // if need - expand it
        if (codeSize > maxCodeInSection) {
            int additional512ByteParts = 0;
            int extraCode = 0;
            
            extraCode = codeSize - maxCodeInSection;
            additional512ByteParts = extraCode / 512 + 1;
            // expand image
            imageSize += additional512ByteParts * 512;
            image = realloc(image, imageSize);
            // set up PE file fields
            //! FIXME: UB
            //! ACHTUNG: hardcoded
            if (additional512ByteParts) {
                // OptionalHeader::SizeOfCode
                *(int *)(image + 0x9c) += additional512ByteParts * 512;
                // OptionalHeader::SizeOfImage
                *(int *)(image + 0xd0) = 0x2000 + aligned(codeSize + sizeOfRuntimeCode, 0x1000);
                // SectionHeader::VirtualSize (.text)
                *(int *)(image + 0x1a8) += additional512ByteParts * 512;
                // SectionHeader::SizeOfRawDara (.text)
                *(int *)(image + 0x1b0) += additional512ByteParts * 512;
            }
        }
        // now we can safely copy brainfuck code
        memcpy(image + codeOffset, code, codeSize);
    }
    // write image to file
    {
        char newName[strlen(name) + 5];
        FILE *fp = NULL;

        strcpy(newName, name);
        strcat(newName, ".exe");
        fp = fopen(newName, "wb");
        if (!fp) { return ERROR_OUTPUT_FILE_IS_NOT_CREATED; }
        fwrite(image, 1, imageSize, fp);
        fclose(fp);
    }
    return 0;
}
