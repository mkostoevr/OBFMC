#include <stdio.h>
#include <string.h>

#include <obfmc.h>

static void dump(size_t size, const void *data) {
	char ascii[17];
	size_t i, j;
	ascii[16] = '\0';
	for (i = 0; i < size; ++i) {
		printf("%02X ", ((unsigned char*)data)[i]);
		if (((unsigned char*)data)[i] >= ' ' && ((unsigned char*)data)[i] <= '~') {
			ascii[i % 16] = ((unsigned char*)data)[i];
		} else {
			ascii[i % 16] = '.';
		}
		if ((i+1) % 8 == 0 || i+1 == size) {
			printf(" ");
			if ((i+1) % 16 == 0) {
				printf("|  %s \n", ascii);
			} else if (i+1 == size) {
				ascii[(i+1) % 16] = '\0';
				if ((i+1) % 16 <= 8) {
					printf(" ");
				}
				for (j = (i+1) % 16; j < 16; ++j) {
					printf("   ");
				}
				printf("|  %s \n", ascii);
			}
		}
	}
}

// translates to IR of format: byte operation, byte executionCount
// currently every operation executes once, later some operations will be merged
static void expand(size_t irSize, char *ir, char *source) {
    for (size_t i = 0; i < irSize; i++) {
        if (i % 2 == 0) { // operation
            ir[i] = source[i / 2];
        } else { // execution count
            ir[i] = '\1';
        }
    }
}

// remove everything but brainfuck syntax elements from source IR
static void removeGarbage(size_t irSize, char *ir) {
    for (size_t i = 0; i < irSize; i += 2) {
        char c = ir[i];

        if (c != '[' && c != ']' && c != '<' && c != '>' &&
            c != '+' && c != '-' && c != '.' && c != ',') {
            ir[i] = '\0';
        }
    }
}

// translates sequences like -1-1-1-1-1-1 to -6
static void simplificate(size_t irSize, char *ir) {
    int sameOffset = 0;
    char last = 0;

    for (size_t i = 0; i < irSize; i += 2) {
        char c = ir[i];

        sameOffset++;
        if (!c) { continue; }
        if (last == c && (c == '+' || c == '-' || c == '<' || c == '>')) {
            if ((unsigned char)ir[i - sameOffset * 2 + 1] == 255) {
                sameOffset = 0;
            } else {
                ir[i] = '\0';
                ir[i + 1] = '\0';
                ir[i - sameOffset * 2 + 1]++;
            }
        } else { sameOffset = 0; }
        last = c;
    }
}

// translates [-] to =0
static void optimizeZerofications(size_t irSize, char *ir) {
    struct {
        char c;
        size_t i;
    } prev[2];

    for (size_t i = 0; i < irSize; i += 2) {
        char c = ir[i];

        if (prev[1].c == '[' && prev[0].c == '-' && c == ']') {
            ir[i] = '\0';
            ir[prev[0].i] = '\0';
            // *ptr = 0
            ir[prev[1].i] = '=';
            ir[prev[1].i + 1] = 0;
        }
        prev[1] = prev[0];
        prev[0].c = ir[i];
        prev[0].i = i;
    }
}

int bfInit(Bf *restrict bf, size_t sourceSize, char *restrict source) {
    size_t irSize = sourceSize * 2;
    char *ir = malloc(irSize);
    
    //puts("Was:");
    //dump(sourceSize, source);
    expand(irSize, ir, source);
    //puts("After expansion:");
    //dump(irSize, ir);
    removeGarbage(irSize, ir);
    //puts("After garbage removing:");
    //dump(irSize, ir);
    simplificate(irSize, ir);
    //puts("After simplification:");
    //dump(irSize, ir);
    optimizeZerofications(irSize, ir);
    //puts("After zerofications optimization:");
    //dump(irSize, ir);
    bf->irSize = irSize;
    bf->ir = ir;
    return 0;
}
