#pragma once

#include <stdint.h>

typedef struct {
    size_t irSize;
    char *ir;
} Bf;

enum {
    ERROR_SUCCESS,
    ERROR_OUT_OF_MEMORY,
    ERROR_FILE_IS_NOT_CREATED,
    ERROR_OUTPUT_FILE_IS_NOT_CREATED,
    ERROR_CAN_NOT_OPEN_RUNTIME,
    ERROR_ILLEGAL_INSTRUCTION,
};

int bfInit(Bf *restrict bf, size_t sourceSize, char *restrict source);
int buildDos16(Bf *restrict bf, const char *restrict name);
int buildKos32(Bf *restrict bf, const char *restrict name);
int buildWin32(Bf *restrict bf, const char *restrict name);
int buildC(Bf *restrict bf, const char *name);
int genI386(Bf *restrict bf, size_t *restrict _outSize, void *restrict *restrict _out);
int gen8086(Bf *restrict bf, size_t *restrict _outSize, void *restrict *restrict _out);
