use16
org 0x7C00
;hello world
main:  ; arg num 0x0
mov si, MSG
mov ah, 0xe
mov al, [si]
.L0:
cmp al, 0x0
jz .EL0
int 0x10
inc si
mov al, [si]

jmp .L0
.EL0:
cmp al, 0x1
jle .EI0
cmp al, 0x2
jle .EI1
.L1:
cmp al, 0x3
jg .EL1
.L2:
cmp al, 0x4
jg .EL2

jmp .L2
.EL2:

jmp .L1
.EL1:

.EI1:

.EI0:
jmp $
MSG: db "Hello World", 0xa, 0xd, 0x0
times 510-($-$$) db 0x0
MAGIC: dw 0xaa55
