format PE console
entry start

section '.idata' import data readable writeable

  dd 0,0,0,RVA kernel_name,RVA kernel_table
  dd 0,0,0,RVA crt_name,RVA crt_table
  dd 0,0,0,0,0

  kernel_table:
    ExitProcess    dd RVA _ExitProcess
    GetProcessHeap dd RVA _GetProcessHeap
    HeapAlloc      dd RVA _HeapAlloc
    dd 0
  crt_table:
    putchar dd RVA _putchar
    getchar dd RVA _getchar
    dd 0

  kernel_name db 'KERNEL32.DLL',0
  crt_name db 'MSVCRT.DLL',0

  _ExitProcess dw 0
    db 'ExitProcess',0
  _HeapAlloc dw 0
    db 'HeapAlloc',0
  _GetProcessHeap dw 0
    db 'GetProcessHeap',0
  _putchar dw 0
    db 'putchar', 0
  _getchar dw 0
    db '_getch', 0

section '.text' code readable executable

  bf_putchar:
	push eax
	push ebx
	push ecx
	push dword[eax]
	call [putchar]
	add  esp, 4
	pop  ecx
	pop  ebx
	pop  eax
	ret

  bf_getchar:
	push ebx
	push ecx
	push eax
	call [getchar]
	mov  ebx, eax
	pop  eax
	mov  byte[eax], bl
	pop  ecx
	pop  ebx
	ret

  start:

	call    [GetProcessHeap]
	push    30000
	push    8
	push    eax
	call    [HeapAlloc]
	mov     ebx, bf_putchar
        mov     ecx, bf_getchar
	call    brainfuck
	push	0
	call	[ExitProcess]
  brainfuck:
	db 	"PLACE_FOR_BRAINFUCK_CODE"
