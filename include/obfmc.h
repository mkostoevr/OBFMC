#pragma once

#include <stdint.h>

typedef struct {
    size_t irSize;
    char *ir;
} Bf;

int bfInit(Bf *restrict bf, size_t sourceSize, char *restrict source);
