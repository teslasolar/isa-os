; ════════════════════════════════════════════════════════════════════════
; ISA-OS Bootloader (boot.asm)
; Target: x86_64 (starts in 16-bit real mode)
; Purpose: Load kernel and transition to protected mode
; ════════════════════════════════════════════════════════════════════════

[BITS 16]
[ORG 0x7C00]

boot:
    cli                         ; Disable interrupts during setup
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00              ; Stack grows down from bootloader

    ; Print boot message
    mov si, boot_msg
    call print_string

    ; Load kernel from disk (sector 2+, 32 sectors = 16KB)
    mov ah, 0x02                ; BIOS read sectors
    mov al, 32                  ; Number of sectors to read
    mov ch, 0                   ; Cylinder 0
    mov cl, 2                   ; Start from sector 2 (after boot sector)
    mov dh, 0                   ; Head 0
    mov bx, 0x1000              ; Load kernel to 0x1000
    int 0x13                    ; BIOS disk interrupt
    jc disk_error               ; Jump if carry flag set (error)

    ; Print success message
    mov si, load_ok_msg
    call print_string

    ; Enable A20 line (access memory above 1MB)
    call enable_a20

    ; Load GDT
    lgdt [gdt_descriptor]

    ; Enter protected mode
    mov eax, cr0
    or eax, 1                   ; Set PE (Protection Enable) bit
    mov cr0, eax

    ; Far jump to flush pipeline and load CS
    jmp 0x08:protected_mode     ; 0x08 = code segment selector

disk_error:
    mov si, disk_err_msg
    call print_string
    hlt

; ────────────────────────────────────────────────────────────────────────
; Function: print_string (16-bit real mode)
; Input: SI = pointer to null-terminated string
; ────────────────────────────────────────────────────────────────────────
print_string:
    pusha
.loop:
    lodsb                       ; Load byte from SI into AL, increment SI
    or al, al                   ; Check if zero (end of string)
    jz .done
    mov ah, 0x0E                ; BIOS teletype output
    int 0x10                    ; Print character
    jmp .loop
.done:
    popa
    ret

; ────────────────────────────────────────────────────────────────────────
; Function: enable_a20
; Enable A20 line using keyboard controller method
; ────────────────────────────────────────────────────────────────────────
enable_a20:
    cli
    call .wait_input
    mov al, 0xAD                ; Disable keyboard
    out 0x64, al

    call .wait_input
    mov al, 0xD0                ; Read controller output port
    out 0x64, al

    call .wait_output
    in al, 0x60
    push ax

    call .wait_input
    mov al, 0xD1                ; Write controller output port
    out 0x64, al

    call .wait_input
    pop ax
    or al, 2                    ; Enable A20
    out 0x60, al

    call .wait_input
    mov al, 0xAE                ; Enable keyboard
    out 0x64, al

    call .wait_input
    sti
    ret

.wait_input:
    in al, 0x64
    test al, 2
    jnz .wait_input
    ret

.wait_output:
    in al, 0x64
    test al, 1
    jz .wait_output
    ret

; ────────────────────────────────────────────────────────────────────────
; GDT (Global Descriptor Table)
; ────────────────────────────────────────────────────────────────────────
gdt_start:
    ; Null descriptor (required)
    dq 0x0000000000000000

    ; Code segment descriptor (ring 0)
    ; Base=0, Limit=0xFFFFF, Access=9A, Flags=CF
    dq 0x00CF9A000000FFFF

    ; Data segment descriptor (ring 0)
    ; Base=0, Limit=0xFFFFF, Access=92, Flags=CF
    dq 0x00CF92000000FFFF

    ; User code segment descriptor (ring 3)
    ; Base=0, Limit=0xFFFFF, Access=FA, Flags=CF
    dq 0x00CFFA000000FFFF

    ; User data segment descriptor (ring 3)
    ; Base=0, Limit=0xFFFFF, Access=F2, Flags=CF
    dq 0x00CFF2000000FFFF

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1  ; GDT size
    dd gdt_start                ; GDT base address

; ────────────────────────────────────────────────────────────────────────
; Protected Mode Entry (32-bit)
; ────────────────────────────────────────────────────────────────────────
[BITS 32]
protected_mode:
    ; Setup segment registers
    mov ax, 0x10                ; Data segment selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000            ; Set kernel stack

    ; Print protected mode message
    mov esi, pmode_msg
    call print_string_pm

    ; Jump to kernel entry point
    jmp 0x1000                  ; Kernel loaded at 0x1000

; ────────────────────────────────────────────────────────────────────────
; Function: print_string_pm (32-bit protected mode)
; Input: ESI = pointer to null-terminated string
; ────────────────────────────────────────────────────────────────────────
print_string_pm:
    pusha
    mov edi, 0xB8000            ; VGA text mode buffer
.loop:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0F                ; White on black
    stosw                       ; Write character + attribute
    jmp .loop
.done:
    popa
    ret

; ────────────────────────────────────────────────────────────────────────
; Data Section
; ────────────────────────────────────────────────────────────────────────
boot_msg:       db 'ISA-OS Bootloader v1.0', 13, 10, 0
load_ok_msg:    db 'Kernel loaded', 13, 10, 0
disk_err_msg:   db 'DISK ERROR!', 13, 10, 0
pmode_msg:      db 'Protected Mode OK', 0

; ────────────────────────────────────────────────────────────────────────
; Boot Signature (must be at offset 510)
; ────────────────────────────────────────────────────────────────────────
times 510-($-$$) db 0
dw 0xAA55                       ; Boot signature
