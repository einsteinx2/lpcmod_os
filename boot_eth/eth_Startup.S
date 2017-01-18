#include "../include/memory_layout.h"

.code32

.section .vmlinuz_start, "ax"

.org 0x00
    
    xor    %eax, %eax
    xor    %edx, %edx          
    cld
    
    mov     $CODE_LOC_START, %edi        // Destiation
    mov     $0x00100100, %esi    // Source
    mov     $0x400000, %ecx        // 4MB
    rep movsb
    wbinvd
/*    
    mov $0xc004, %dx 
    mov $0x20, %al 
    out %al, %dx 
    mov $0xc008, %dx 
    mov $0x8, %al 
    out %al, %dx 
    mov $0xc006, %dx 
    mov $0xa6, %al 
    out %al, %dx 
    mov $0xc006, %dx 
    in %dx,%al 
    mov $0xc002, %dx 
    mov $0x1a, %al 
    out %al, %dx 
    mov $0xc000, %dx 

    ledspin: in %dx, %al 
    cmp $0x10, %al 
    jnz ledspin 
    mov $0xc004, %dx 
    mov $0x20, %al 
    out %al, %dx 
    mov $0xc008, %dx 
    mov $0x7, %al 
    out %al, %dx 
    mov $0xc006, %dx 
    mov $0x1, %al 
    out %al, %dx 
    mov $0xc006, %dx 
    in %dx,%al 
    mov $0xc002, %dx 
    mov $0x1a, %al 
    out %al, %dx 
    mov $0xc000, %dx 

    ledspin1: in %dx, %al
    cmp $0x10, %al
    jnz ledspin1 

    jmp ledspin1 
*/
    
    cld    
    ljmp     $0x10, $CODE_LOC_START

.org 0x0100    // append crom image here
