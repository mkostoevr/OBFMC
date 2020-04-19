#include <stdio.h>

#include <obfmc.h>

void toC(Bf *bf) {
    int level = 1;
    
    puts("#include <stdio.h>");
    puts("int main() {");
    puts("    char memory[30000] = { 0 };");
    puts("    char *ptr = memory;");
    for (size_t i = 0; i < bf->irSize; i += 2) {
        if (bf->ir[i]) {
            for (int i = 0; i < level; i++) {
                printf("    ");
            }
        }
        switch (bf->ir[i]) {
        case '>':
            printf("ptr += %d;\n", bf->ir[i + 1]);
            break;
        case '<':
            printf("ptr -= %d;\n", bf->ir[i + 1]);
            break;
        case '+':
            printf("*ptr += %d;\n", bf->ir[i + 1]);
            break;
        case '-':
            printf("*ptr -= %d;\n", bf->ir[i + 1]);
            break;
        case '.':
            puts("putchar(*ptr);");
            break;
        case ',':
            puts("*ptr = getchar();");
            break;
        case '[':
            puts("while (*ptr) {");
            level++;
            break;
        case ']':
            puts("}");
            level--;
            break;
        case '\0':
            break;
        default:
            puts("Error: Illegal instruction");
            return;
        }
    }
    puts("}");
}
