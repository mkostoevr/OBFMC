format binary as 'bin'

include 'proc32.inc'

use32
        db      'MENUET01'
        dd      1
        dd      start
        dd      0 ; size of executable, configure in compiler
        dd      0 ; memory need, configure in compiler
        dd      0 ; esp, configure in compiler
        dd      0 ; arguments, not used
        dd      0 ; app name, not used

start:
        stdcall load_dll_and_import, dllname, imports
        test    eax, eax
        jz      exit
	push    caption
        push    -1
        push    -1
        push    -1
        push    -1
        call    [con_init]
        ;int3
        ; initialize 30000 byte array
        mov	ecx, 30000
        mov     eax, 68
        mov     ebx, 12
        int     0x40
        test    eax, eax
        jz      exit
        mov     ebx, bf_putchar
        mov     ecx, bf_getchar
        call    brainfuck
        push    0
        call    [con_exit]
exit:
        or      eax, -1
        int     0x40

bf_putchar:
        push eax
	push ebx
	push ecx
        push dword[eax]
        push fmt
        call [con_printf]
        add  esp, 8
	pop  ecx
        pop  ebx
        pop  eax
        ret

bf_getchar:
        push ecx
        push ebx
        push eax
        call [con_getch]
        mov ebx, eax
        pop eax
        mov byte[eax], bl
	pop ebx
        pop ecx
        ret

proc load_dll_and_import stdcall, _dllname:dword, _imports:dword
        pushad
; load DLL
        push    68
        pop     eax
        push    19
        pop     ebx
        mov     ecx, [_dllname]
        int     0x40
        test    eax, eax
        jz      import_fail

; initialize import
        mov     edi, eax
        mov     esi, [_imports]
import_loop:
        lodsd
        test    eax, eax
        jz      import_done
        mov     edx, edi
import_find:
        mov     ebx, [edx]
        test    ebx, ebx
        jz      import_not_found
        push    eax
@@:
        mov     cl, [eax]
        cmp     cl, [ebx]
        jnz     import_find_next
        test    cl, cl
        jz      import_found
        inc     eax
        inc     ebx
        jmp     @b
import_find_next:
        pop     eax
        add     edx, 8
        jmp     import_find
import_found:
        pop     eax
        mov     eax, [edx+4]
        mov     [esi-4], eax
        jmp     import_loop
import_not_found:
import_fail:
        popad
        xor     eax, eax
        ret
import_done:
        popad
        xor     eax, eax
        inc     eax
        ret
endp

align 4

imports:
con_init           dd szcon_init
con_printf         dd szcon_printf
con_exit           dd szcon_exit
con_getch          dd szcon_getch
                   dd 0

szcon_init         db 'con_init',0
szcon_printf       db 'con_printf',0
szcon_exit         db 'con_exit',0
szcon_getch        db 'con_getch',0

dllname            db '/sys/lib/console.obj',0
fmt                db '%c', 0
caption            db 'Brainfuck app',0

brainfuck: