use16
org 0x7C00
;osdev

use16
org 0x7C00
jmp _start

SECTOR_NUM = 0x64
LOAD_LOCATION = 0x7e00
_start:  ; arg num 0x0
mov ax, 0x0

    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ds, ax
    mov sp, 0x7C00
    
mov [BOOT_DISK], dl
mov ah, 0x2
mov al, SECTOR_NUM
mov cx, 0x2
mov dh, 0x0
mov bx, LOAD_LOCATION
int 0x13

call LOAD_LOCATION
mov ax, 0xe30
int 0x10
jmp $
ret
BOOT_DISK: db 0x0

times 510-($-$$) db 0x00
dw 0xAA55
jmp _os_start

INTRO: db "Welcome to llos. A simple 16 bit OS.", 0xa, 0x0
LINEOVERFLOW: db 0xa, "InputError: line overflow.", 0xa, 0x0
SET_COLOR: db 0x2
LINEMAX = 0x100
_os_start:  ; arg num 0x0

call _clrscr
push INTRO

call _puts
.F0:

call _shell

jmp .F0
.EF0:
ret
ret
_shell:  ; arg num 0x0

call _getc
cmp al, 0x0
jle .EI0
push ax

call _putc

.EI0:
ret
ret
_getc:  ; arg num 0x0
xor ax, ax
int 0x16
ret
ret
_putc:  ; arg num 0x1
pop bp
pop ax
push bp
mov ah, 0xe
int 0x10
cmp al, 0x8
jnz .M1
mov al, 0x20
int 0x10
mov al, 0x8
int 0x10

.M1:
cmp al, 0xd
jnz .M2
mov al, 0xa
int 0x10

.M2:
cmp al, 0xa
jnz .M3
mov al, 0xd
int 0x10

.M3:
ret
ret
_puts:  ; arg num 0x2
push ax
push si
.L0:
mov al, [si]
push ax

call _putc

cmp al, 0x0
jnz .L0
.EL0:
pop si
pop ax
ret
_clrscr:  ; arg num 0x0
push ax
push bx
push cx
push dx
mov ax, 0x600
mov bh, [SET_COLOR]
mov cx, 0x0
mov dx, 0xffff
int 0x10
pop dx
pop cx
pop bx
pop ax
ret
ret
