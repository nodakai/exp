    .data
msg:
    .asciz "Hello, world!\xa"
msgend:
    .set len, msgend - msg

    .text
    .globl _start
_start:
    mov $4, %rax
    mov $1, %rbx
    mov $msg, %rcx
    mov $len, %rdx
    int $0x80

    mov $1, %rax
    mov $42, %rbx
    int $0x80
