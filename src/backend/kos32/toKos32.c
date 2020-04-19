#include <stdio.h>
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
    size_t bufCap = 1000000000;
    char *buf = malloc(bufCap);
    Header *header = (Header *)buf;

    memcpy(header->magic, "MENUET01", 8);
    header->version = 1;
    header->entry = sizeof(Header);
    header->sizeOfFile = sizeof(Header);
    header->params = 0;
    header->argv0 = 0;
    {
        FILE *fp = fopen("start.debug.bin", "rb");
        size_t size = 0;

        fseek(fp, 0, SEEK_END);
        size = ftell(fp);
        rewind(fp);
        fread(&buf[header->entry], 1, size, fp);
        fclose(fp);
        header->sizeOfFile += size;
    }
    // EAX = ptr
    // EBX = bf_putchar
    // ECX = bf_getchar
    struct Loop {
        int beginAddress;
        struct Loop *prev;
        struct Loop *next;
    } *loop = calloc(sizeof(struct Loop), 1), *prev = NULL;
    for (size_t i = 0; i < bf->irSize; i += 2) {
        switch (bf->ir[i]) {
        case '>':
            // ADD EAX, bf->ir[i + 1]
            buf[header->sizeOfFile++] = 0x05;
            buf[header->sizeOfFile++] = bf->ir[i + 1];
            buf[header->sizeOfFile++] = 0x00;
            buf[header->sizeOfFile++] = 0x00;
            buf[header->sizeOfFile++] = 0x00;
            break;
        case '<':
            // SUB EAX, bf->ir[i + 1]
            buf[header->sizeOfFile++] = 0x2d;
            buf[header->sizeOfFile++] = bf->ir[i + 1];
            buf[header->sizeOfFile++] = 0x00;
            buf[header->sizeOfFile++] = 0x00;
            buf[header->sizeOfFile++] = 0x00;
            break;
        case '+':
            // ADD [EAX], bf->ir[i + 1]
            buf[header->sizeOfFile++] = 0x80;
            buf[header->sizeOfFile++] = 0x00;
            buf[header->sizeOfFile++] = bf->ir[i + 1];
            break;
        case '-':
            // SUB [EAX], bf->ir[i + 1]
            buf[header->sizeOfFile++] = 0x80;
            buf[header->sizeOfFile++] = 0x28;
            buf[header->sizeOfFile++] = bf->ir[i + 1];
            break;
        case '.':
            // CALL EBX
            buf[header->sizeOfFile++] = 0xff;
            buf[header->sizeOfFile++] = 0xd3;
            break;
        case ',':
            // CALL ECX
            buf[header->sizeOfFile++] = 0xff;
            buf[header->sizeOfFile++] = 0xd1;
            break;
        case '[':
            loop->beginAddress = header->sizeOfFile;
            loop->next = calloc(sizeof(struct Loop), 1);
            prev = loop;
            loop = loop->next;
            loop->prev = prev;
            // CMP [EAX], 0
            buf[header->sizeOfFile++] = 0x80;
            buf[header->sizeOfFile++] = 0x38;
            buf[header->sizeOfFile++] = 0x00;
            // JE  END_OF_CICLE
            buf[header->sizeOfFile++] = 0x0f;
            buf[header->sizeOfFile++] = 0x84;
            // here is loopBegin + 5
            // targetAddress - (thisAddress + 6)
            buf[header->sizeOfFile++] = 0x00;
            buf[header->sizeOfFile++] = 0x00;
            buf[header->sizeOfFile++] = 0x00;
            buf[header->sizeOfFile++] = 0x00;
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
                buf[header->sizeOfFile++] = 0xe9;
                // loopBegin - (thisAddress + 4)
                instructionEndAddress = header->sizeOfFile + 4;
                target = loop->beginAddress - instructionEndAddress;
                buf[header->sizeOfFile++] = (target & 0xff) >> 0;
                buf[header->sizeOfFile++] = (target & 0xff00) >> 8;
                buf[header->sizeOfFile++] = (target & 0xff0000) >> 16;
                buf[header->sizeOfFile++] = (target & 0xff000000) >> 24;
                // set JMP from loop beginning (jmp here, to end of loop)
                loopBegiTarget = header->sizeOfFile - (loop->beginAddress + 5 + 4);
                buf[loop->beginAddress + 5] = (loopBegiTarget & 0xff) >> 0;
                buf[loop->beginAddress + 6] = (loopBegiTarget & 0xff00) >> 8;
                buf[loop->beginAddress + 7] = (loopBegiTarget & 0xff0000) >> 16;
                buf[loop->beginAddress + 8] = (loopBegiTarget & 0xff000000) >> 24;
            }
            break;
        case '\0':
            break;
        default:
            puts("Error: Illegal instruction");
            return;
        }
    }
    buf[header->sizeOfFile++] = 0xc3; // RET
    header->memoryNeed = header->sizeOfFile + 1024; // space for stack
    header->esp = header->memoryNeed; // stack at end of memory
    FILE *out = fopen("a.kex", "wb");
    fwrite(buf, 1, header->sizeOfFile, out);
    fclose(out);
}
