/**
 * ════════════════════════════════════════════════════════════════════════
 * ISA-OS Linker Script (linker.ld)
 * Memory layout for kernel binary
 * ════════════════════════════════════════════════════════════════════════
 */

OUTPUT_FORMAT(binary)
ENTRY(kernel_entry)

SECTIONS
{
    /* Kernel loaded at 0x1000 by bootloader */
    . = 0x1000;

    /* Code section */
    .text ALIGN(16) : {
        *(.text)
        *(.text.*)
    }

    /* Read-only data */
    .rodata ALIGN(16) : {
        *(.rodata)
        *(.rodata.*)
    }

    /* Initialized data */
    .data ALIGN(16) : {
        *(.data)
        *(.data.*)
    }

    /* Uninitialized data (BSS) */
    .bss ALIGN(16) : {
        bss_start = .;
        *(.bss)
        *(.bss.*)
        *(COMMON)
        bss_end = .;
    }

    /* Discard debug symbols and other sections */
    /DISCARD/ : {
        *(.comment)
        *(.note*)
        *(.eh_frame)
    }
}
