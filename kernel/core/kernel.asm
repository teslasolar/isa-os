; ════════════════════════════════════════════════════════════════════════
; ISA-OS Kernel Assembly (kernel.asm)
; Target: x86_64 (32-bit compatibility mode)
; Purpose: Kernel entry, syscall interface, context switching
; ════════════════════════════════════════════════════════════════════════

[BITS 32]
[GLOBAL kernel_entry]
[GLOBAL ctx_switch]
[GLOBAL enable_interrupts]
[GLOBAL disable_interrupts]
[GLOBAL load_idt]
[EXTERN kmain]
[EXTERN syscall_dispatcher]
[EXTERN timer_interrupt_handler]

section .text

; ────────────────────────────────────────────────────────────────────────
; Kernel Entry Point (called from bootloader)
; ────────────────────────────────────────────────────────────────────────
kernel_entry:
    cli                         ; Disable interrupts
    mov esp, kernel_stack_top   ; Set kernel stack

    ; Clear BSS section
    mov edi, bss_start
    mov ecx, bss_end
    sub ecx, edi
    xor eax, eax
    rep stosb

    ; Initialize IDT
    call setup_idt
    lidt [idt_descriptor]

    ; Enable interrupts
    sti

    ; Jump to C kernel main
    call kmain

    ; If kmain returns, halt
.hang:
    hlt
    jmp .hang

; ────────────────────────────────────────────────────────────────────────
; Context Switch (save/restore agent context)
; void ctx_switch(uint32_t **old_sp, uint32_t **new_sp)
; ────────────────────────────────────────────────────────────────────────
ctx_switch:
    ; Save old context (if old_sp is not NULL)
    mov eax, [esp + 4]          ; old_sp pointer
    test eax, eax
    jz .load_new                ; Skip save if NULL

    ; Save all general purpose registers
    pushf                       ; EFLAGS
    push ebp
    push edi
    push esi
    push edx
    push ecx
    push ebx
    push eax                    ; Save EAX too

    ; Save current stack pointer to *old_sp
    mov eax, [esp + 36]         ; Get old_sp again (after pushes)
    mov [eax], esp              ; Save current ESP

.load_new:
    ; Load new context
    mov eax, [esp + 40]         ; new_sp pointer (or +8 if we skipped save)
    cmp dword [esp + 4], 0      ; Check if we skipped save
    jne .after_load_adjust
    mov eax, [esp + 8]          ; Adjust offset if skipped

.after_load_adjust:
    mov esp, [eax]              ; Load new ESP

    ; Restore all registers
    pop eax
    pop ebx
    pop ecx
    pop edx
    pop esi
    pop edi
    pop ebp
    popf

    ret

; ────────────────────────────────────────────────────────────────────────
; Interrupt Control Functions
; ────────────────────────────────────────────────────────────────────────
enable_interrupts:
    sti
    ret

disable_interrupts:
    cli
    ret

; ────────────────────────────────────────────────────────────────────────
; Load IDT
; void load_idt(void *idt_ptr)
; ────────────────────────────────────────────────────────────────────────
load_idt:
    push ebp
    mov ebp, esp
    mov eax, [ebp + 8]
    lidt [eax]
    pop ebp
    ret

; ────────────────────────────────────────────────────────────────────────
; Setup IDT (Interrupt Descriptor Table)
; ────────────────────────────────────────────────────────────────────────
setup_idt:
    ; Install timer interrupt (IRQ0 -> INT 32)
    mov eax, timer_int_handler
    mov [idt + 32*8], ax        ; Low 16 bits of handler address
    shr eax, 16
    mov [idt + 32*8 + 6], ax    ; High 16 bits of handler address
    mov word [idt + 32*8 + 2], 0x08  ; Code segment selector
    mov byte [idt + 32*8 + 5], 0x8E  ; Present, ring 0, 32-bit interrupt gate

    ; Install syscall interrupt (INT 0x80)
    mov eax, syscall_handler
    mov [idt + 0x80*8], ax
    shr eax, 16
    mov [idt + 0x80*8 + 6], ax
    mov word [idt + 0x80*8 + 2], 0x08
    mov byte [idt + 0x80*8 + 5], 0xEE  ; Present, ring 3, 32-bit interrupt gate

    ; Program PIC (Programmable Interrupt Controller)
    ; Remap IRQs to INT 32-47
    mov al, 0x11
    out 0x20, al                ; Initialize PIC1
    out 0xA0, al                ; Initialize PIC2

    mov al, 0x20
    out 0x21, al                ; PIC1 offset (INT 32)
    mov al, 0x28
    out 0xA1, al                ; PIC2 offset (INT 40)

    mov al, 0x04
    out 0x21, al                ; PIC1 cascade
    mov al, 0x02
    out 0xA1, al                ; PIC2 cascade

    mov al, 0x01
    out 0x21, al                ; 8086 mode
    out 0xA1, al

    mov al, 0xFE                ; Unmask IRQ0 (timer) only
    out 0x21, al
    mov al, 0xFF
    out 0xA1, al

    ; Program PIT (Programmable Interval Timer) for 1ms ticks
    mov al, 0x36                ; Channel 0, square wave, 16-bit counter
    out 0x43, al
    mov ax, 1193                ; 1.193182 MHz / 1193 ≈ 1000 Hz (1ms)
    out 0x40, al                ; Low byte
    mov al, ah
    out 0x40, al                ; High byte

    ret

; ────────────────────────────────────────────────────────────────────────
; Timer Interrupt Handler (IRQ0)
; ────────────────────────────────────────────────────────────────────────
timer_int_handler:
    pusha                       ; Save all registers

    call timer_interrupt_handler ; Call C handler

    ; Send EOI (End Of Interrupt) to PIC
    mov al, 0x20
    out 0x20, al

    popa                        ; Restore registers
    iret

; ────────────────────────────────────────────────────────────────────────
; Syscall Handler (INT 0x80)
; Entry: EAX = syscall number
;        EBX, ECX, EDX, ESI = arguments
; Return: EAX = return value
; ────────────────────────────────────────────────────────────────────────
syscall_handler:
    push ebp
    push edi
    push esi
    push edx
    push ecx
    push ebx

    ; Call C dispatcher: syscall_dispatcher(num, arg1, arg2, arg3, arg4)
    push esi                    ; arg4
    push edx                    ; arg3
    push ecx                    ; arg2
    push ebx                    ; arg1
    push eax                    ; syscall number
    call syscall_dispatcher
    add esp, 20                 ; Clean up arguments

    pop ebx
    pop ecx
    pop edx
    pop esi
    pop edi
    pop ebp

    iret

; ────────────────────────────────────────────────────────────────────────
; ISA-OS Syscall Wrappers (userspace interface)
; ────────────────────────────────────────────────────────────────────────

[GLOBAL isa_l0_read]
isa_l0_read:
    ; uint32_t isa_l0_read(uint32_t sensor_id, uint32_t *buffer)
    mov eax, 0                  ; Syscall 0
    mov ebx, [esp + 4]          ; sensor_id
    mov ecx, [esp + 8]          ; buffer
    int 0x80
    ret

[GLOBAL isa_l1_write]
isa_l1_write:
    ; void isa_l1_write(uint32_t actuator_id, uint32_t value)
    mov eax, 1                  ; Syscall 1
    mov ebx, [esp + 4]          ; actuator_id
    mov ecx, [esp + 8]          ; value
    int 0x80
    ret

[GLOBAL isa_l2_alarm]
isa_l2_alarm:
    ; void isa_l2_alarm(uint32_t priority, const char *msg)
    mov eax, 2                  ; Syscall 2
    mov ebx, [esp + 4]          ; priority
    mov ecx, [esp + 8]          ; msg
    int 0x80
    ret

[GLOBAL isa_l3_batch_start]
isa_l3_batch_start:
    ; uint32_t isa_l3_batch_start(uint32_t recipe_id)
    mov eax, 3                  ; Syscall 3
    mov ebx, [esp + 4]          ; recipe_id
    int 0x80
    ret

[GLOBAL isa_l4_erp_sync]
isa_l4_erp_sync:
    ; uint32_t isa_l4_erp_sync(void)
    mov eax, 4                  ; Syscall 4
    int 0x80
    ret

[GLOBAL isa_agent_spawn]
isa_agent_spawn:
    ; uint32_t isa_agent_spawn(uint32_t level, void *fn, uint32_t priority)
    mov eax, 5                  ; Syscall 5
    mov ebx, [esp + 4]          ; level
    mov ecx, [esp + 8]          ; fn
    mov edx, [esp + 12]         ; priority
    int 0x80
    ret

[GLOBAL isa_agent_send]
isa_agent_send:
    ; void isa_agent_send(uint32_t dest_id, uint32_t msg)
    mov eax, 6                  ; Syscall 6
    mov ebx, [esp + 4]          ; dest_id
    mov ecx, [esp + 8]          ; msg
    int 0x80
    ret

[GLOBAL isa_yield]
isa_yield:
    ; void isa_yield(void)
    mov eax, 7                  ; Syscall 7
    int 0x80
    ret

; ────────────────────────────────────────────────────────────────────────
; Data Section
; ────────────────────────────────────────────────────────────────────────
section .data

idt_descriptor:
    dw 256*8 - 1                ; IDT limit (256 entries)
    dd idt                      ; IDT base address

section .bss

; IDT (256 entries, 8 bytes each)
idt:
    resb 256*8

; Kernel stack (16KB)
kernel_stack:
    resb 16384
kernel_stack_top:

; BSS markers (for clearing)
bss_start:
bss_end:
