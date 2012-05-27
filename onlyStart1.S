    .text
    .globl _start
_start:
    # mov $1, %eax
    xor %eax, %eax
    inc %eax
    mov $42, %bl
    int $0x80
