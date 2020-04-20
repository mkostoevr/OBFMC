format binary as 'bin'
org 100h
    jmp    start
    bufptr dw buf ; this will be modified by compiler (+code size)
start:
    cld
    mov    cx, 30000
    mov    di, [bufptr]
    xor    ax, ax
    rep    stosb
    mov    si, [bufptr]
    mov    bx, bf_putchar
    mov    cx, bf_getchar
    call   brainfuck
    mov    ah, 4ch
    int    21h

bf_putchar:
    push   si
    push   bx
    push   cx
    mov    dl, [ds:si]
    mov    ah, 2
    int    21h
    pop    cx
    pop    bx
    pop    si
    ret

bf_getchar:
    push   si
    push   bx
    push   cx
    mov    ah, 1
    int    21h
    mov    [ds:si], al
    pop    cx
    pop    bx
    pop    si
    ret

brainfuck:

buf rb 30000
