#include <stdio.h>
#include <string.h>

#include <obfmc.h>

// Required environment:
// SI = ptr in brainfuck memory
// BX = bf_putchar
// CX = bf_getchar
int gen8086(Bf *restrict bf, size_t *restrict _outSize, void *restrict *restrict _out) {
    size_t outCap = 100;
    size_t outSize = 0;
    char *out = malloc(outCap);
    struct Loop {
        int beginAddress;
        struct Loop *prev;
        struct Loop *next;
    } *loop = calloc(sizeof(struct Loop), 1), *prev = NULL;

    if (!loop) { return ERROR_OUT_OF_MEMORY; }
    *_out = NULL;
    *_outSize = 0;
    for (size_t i = 0; i < bf->irSize; i += 2) {
        if (outSize + 100 >= outCap) {
            outCap += outCap >> 1;
            if (!(out = realloc(out, outCap))) { return ERROR_OUT_OF_MEMORY; }
        }
        switch (bf->ir[i]) {
        case '>':
            // ADD SI, bf->ir[i + 1]
            out[outSize++] = 0x81;
            out[outSize++] = 0xc6;
            out[outSize++] = bf->ir[i + 1];
            out[outSize++] = 0x00;
            break;
        case '<':
            // SUB SI, bf->ir[i + 1]
            out[outSize++] = 0x81;
            out[outSize++] = 0xee;
            out[outSize++] = bf->ir[i + 1];
            out[outSize++] = 0x00;
            break;
        case '+':
            // ADD [SI], bf->ir[i + 1]
            out[outSize++] = 0x80;
            out[outSize++] = 0x04;
            out[outSize++] = bf->ir[i + 1];
            break;
        case '-':
            // SUB [SI], bf->ir[i + 1]
            out[outSize++] = 0x80;
            out[outSize++] = 0x2c;
            out[outSize++] = bf->ir[i + 1];
            break;
        case '.':
            // CALL BX
            out[outSize++] = 0xff;
            out[outSize++] = 0xd3;
            break;
        case ',':
            // CALL CX
            out[outSize++] = 0xff;
            out[outSize++] = 0xd1;
            break;
        case '[':
            loop->beginAddress = outSize;
            loop->next = calloc(sizeof(struct Loop), 1);
            prev = loop;
            loop = loop->next;
            loop->prev = prev;
            // CMP [SI], 0
            out[outSize++] = 0x80;
            out[outSize++] = 0x3c;
            out[outSize++] = 0x00;
            // JE  END_OF_CICLE
            out[outSize++] = 0x0f;
            out[outSize++] = 0x84;
            // here is loopBegin + 5
            // targetAddress - (thisAddress + 2)
            out[outSize++] = 0x00;
            out[outSize++] = 0x00;
            break;
        case ']':
            {
                int instructionEndAddress = 0;
                int target = 0;
                int loopBegiTarget = 0;

                loop = loop->prev;
                free(loop->next);
                loop->next = 0;
                // JMP START_OF_CICLE
                out[outSize++] = 0xe9;
                // loopBegin - (thisAddress + 2)
                instructionEndAddress = outSize + 2;
                target = loop->beginAddress - instructionEndAddress;
                out[outSize++] = (target & 0xff) >> 0;
                out[outSize++] = (target & 0xff00) >> 8;
                // set JMP from loop beginning (jmp here, to end of loop)
                loopBegiTarget = outSize - (loop->beginAddress + 5 + 2);
                out[loop->beginAddress + 5] = (loopBegiTarget & 0xff) >> 0;
                out[loop->beginAddress + 6] = (loopBegiTarget & 0xff00) >> 8;
            }
            break;
        case '\0':
            break;
        default:
            puts("Error: Illegal instruction");
            return ERROR_ILLEGAL_INSTRUCTION;
        }
    }
    out[outSize++] = 0xc3; // RET
    free(loop);
    *_out = out;
    *_outSize = outSize;
    return 0;
}
