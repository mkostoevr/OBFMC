#include <stdio.h>
#include <string.h>

#include <obfmc.h>

// Required environment:
// EAX = ptr in brainfuck memory
// EBX = bf_putchar
// ECX = bf_getchar
void toX86(Bf *bf, size_t *_outSize, void **_out) {
    size_t outCap = 100;
    size_t outSize = 0;
    char *out = malloc(outCap);
    struct Loop {
        int beginAddress;
        struct Loop *prev;
        struct Loop *next;
    } *loop = calloc(sizeof(struct Loop), 1), *prev = NULL;
    
    *_out = NULL;
    *_outSize = 0;
    for (size_t i = 0; i < bf->irSize; i += 2) {
        if (outSize + 100 >= outCap) {
            outCap += outCap >> 1;
            out = realloc(out, outCap);
            printf("New size: %d\n", outCap);
        }
        switch (bf->ir[i]) {
        case '>':
            // ADD EAX, bf->ir[i + 1]
            out[outSize++] = 0x05;
            out[outSize++] = bf->ir[i + 1];
            out[outSize++] = 0x00;
            out[outSize++] = 0x00;
            out[outSize++] = 0x00;
            break;
        case '<':
            // SUB EAX, bf->ir[i + 1]
            out[outSize++] = 0x2d;
            out[outSize++] = bf->ir[i + 1];
            out[outSize++] = 0x00;
            out[outSize++] = 0x00;
            out[outSize++] = 0x00;
            break;
        case '+':
            // ADD [EAX], bf->ir[i + 1]
            out[outSize++] = 0x80;
            out[outSize++] = 0x00;
            out[outSize++] = bf->ir[i + 1];
            break;
        case '-':
            // SUB [EAX], bf->ir[i + 1]
            out[outSize++] = 0x80;
            out[outSize++] = 0x28;
            out[outSize++] = bf->ir[i + 1];
            break;
        case '.':
            // CALL EBX
            out[outSize++] = 0xff;
            out[outSize++] = 0xd3;
            break;
        case ',':
            // CALL ECX
            out[outSize++] = 0xff;
            out[outSize++] = 0xd1;
            break;
        case '[':
            loop->beginAddress = outSize;
            loop->next = calloc(sizeof(struct Loop), 1);
            prev = loop;
            loop = loop->next;
            loop->prev = prev;
            // CMP [EAX], 0
            out[outSize++] = 0x80;
            out[outSize++] = 0x38;
            out[outSize++] = 0x00;
            // JE  END_OF_CICLE
            out[outSize++] = 0x0f;
            out[outSize++] = 0x84;
            // here is loopBegin + 5
            // targetAddress - (thisAddress + 4)
            out[outSize++] = 0x00;
            out[outSize++] = 0x00;
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
                // loopBegin - (thisAddress + 4)
                instructionEndAddress = outSize + 4;
                target = loop->beginAddress - instructionEndAddress;
                out[outSize++] = (target & 0xff) >> 0;
                out[outSize++] = (target & 0xff00) >> 8;
                out[outSize++] = (target & 0xff0000) >> 16;
                out[outSize++] = (target & 0xff000000) >> 24;
                // set JMP from loop beginning (jmp here, to end of loop)
                loopBegiTarget = outSize - (loop->beginAddress + 5 + 4);
                out[loop->beginAddress + 5] = (loopBegiTarget & 0xff) >> 0;
                out[loop->beginAddress + 6] = (loopBegiTarget & 0xff00) >> 8;
                out[loop->beginAddress + 7] = (loopBegiTarget & 0xff0000) >> 16;
                out[loop->beginAddress + 8] = (loopBegiTarget & 0xff000000) >> 24;
            }
            break;
        case '\0':
            break;
        default:
            puts("Error: Illegal instruction");
            return;
        }
    }
    out[outSize++] = 0xc3; // RET
    free(loop);
    *_out = out;
    *_outSize = outSize;
}
