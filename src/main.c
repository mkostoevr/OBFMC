#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <obfmc.h>

#ifdef ONE_SOURCE
#   include "obfmc.c"
#   include "gen.8086.c"
#   include "gen.i386.c"
#   include "build.c.c"
#   include "build.dos16.c"
#   include "build.kos32.c"
#   include "build.win32.c"
#endif

struct Target {
    int (*builder)(Bf *, const char *);
    struct Target *next;
} *firstTarget;

static int addTarget(int (*builder)(Bf *, const char *)) {
    struct Target **targetPtr = &firstTarget;
    while (*targetPtr) { targetPtr = &((*targetPtr)->next); }
    if (!(*targetPtr = calloc(sizeof(**targetPtr), 1))) { return ERROR_OUT_OF_MEMORY; }
    (*targetPtr)->builder = builder;
    return 0;
}

static int buildTargets(struct Target *firstTarget, Bf *bf, const char *name) {
    int errorCode = 0;
    struct Target *target = firstTarget;

    while (target) {
        if ((errorCode = target->builder(bf, name))) { return errorCode; }
        target = target->next;
    }
    return 0;
}

int usage() {
    puts("Usage: bfc sourceName [-o outputName] [--kos32] [--dos16] [--c]\nOne taget is requred.");
    return -1;
}

int main(int argc, char **argv) {
    int errorCode = 0;
    Bf bf = { 0 };
    char *source = NULL;
    size_t sourceSize = 0;
    const char *inputName = NULL;
    const char *outputName = "a";

    if (argc < 2) { puts("Input file isn't specified."); return usage(); }
    inputName = argv[1];
    for (int i = 0; i < argc; i++) {
        if (!memcmp(argv[i], "-o", 2)) {
            if (strlen(argv[i]) > 2) {
                outputName = argv[i] + 2;
            } else {
                outputName = argv[++i];
            }
        } else if (!memcmp(argv[i], "--", 2)) {
            if (!memcmp(argv[i] + 2, "kos32", strlen("kos32"))) {
                addTarget(buildKos32);
            } else if (!memcmp(argv[i] + 2, "dos16", strlen("dos16"))) {
                addTarget(buildDos16);
            } else if (!memcmp(argv[i] + 2, "win32", strlen("win32"))) {
                addTarget(buildWin32);
            } else if (!memcmp(argv[i] + 2, "c", strlen("c"))) {
                addTarget(buildC);
            }
        }
    }
    if (!firstTarget) { puts("Target isn't specified."); return usage(); }
    {
        FILE *fp = fopen(inputName, "r");
        
        if (!fp) {
            fputs("Error: Input file not found\n", stderr);
            return -1;
        }
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
    if ((errorCode = buildTargets(firstTarget, &bf, outputName))) {
        printf("Error #%d\n", errorCode);
    }
    return 0;
}
