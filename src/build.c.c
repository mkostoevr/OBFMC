#include <stdio.h>

#include <obfmc.h>

int buildC(Bf *restrict bf, const char *restrict name) {
    int level = 1;
    FILE *fp;
    char newName[strlen(name) + 2 + 1];
    
    // open output file
    strcpy(newName, name);
    strcat(newName, ".c");
    if (!(fp = fopen(newName, "w"))) { free(newName); return ERROR_FILE_IS_NOT_CREATED; }
    // generate code
    fputs(
        "#include <stdio.h>"           "\n"
        "int main() {"                 "\n"
        "    char memory[30000] = { 0 };\n"
        "    char *ptr = memory;"      "\n"
        , fp);
    for (size_t i = 0; i < bf->irSize; i += 2) {
        if (bf->ir[i]) {
            for (int i = 0; i < level; i++) {
                fprintf(fp, "    ");
            }
        }
        switch (bf->ir[i]) {
        case '>':
            fprintf(fp, "ptr += %d;\n", bf->ir[i + 1]);
            break;
        case '<':
            fprintf(fp, "ptr -= %d;\n", bf->ir[i + 1]);
            break;
        case '+':
            fprintf(fp, "*ptr += %d;\n", bf->ir[i + 1]);
            break;
        case '-':
            fprintf(fp, "*ptr -= %d;\n", bf->ir[i + 1]);
            break;
        case '=':
            fprintf(fp, "*ptr = %d;\n", bf->ir[i + 1]);
            break;
        case '.':
            fputs("putchar(*ptr);\n", fp);
            break;
        case ',':
            fputs("*ptr = getchar();\n", fp);
            break;
        case '[':
            fputs("while (*ptr) {\n", fp);
            level++;
            break;
        case ']':
            fputs("}\n", fp);
            level--;
            break;
        case '\0':
            break;
        default:
            fputs("Error: Illegal instruction", stderr);
            return ERROR_ILLEGAL_INSTRUCTION;
        }
    }
    fputs("}\n", fp);
    fclose(fp);
    return 0;
}
