use16
org 0x7C00
;hello world
main:  ; arg num 0x0
mov si, MSG
mov ah, 0xe
mov al, [si]
.L0:
int 0x10
inc si
mov al, [si]

cmp al, 0x0
jnz .L0
.EL0:
jmp $
MSG: db "Hello World", 0xa, 0xd, 0x0
times 510-($-$$) db 0x0
MAGIC: dw 0xaa55
