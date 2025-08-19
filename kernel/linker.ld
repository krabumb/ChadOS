ENTRY(_start)

SECTIONS {
    /* Multiboot2 header directly at start */
    . = 0x100000;

    /* Place header directly at this address in the file too */
    .multiboot2 : AT(0x100000) {
        KEEP(*(.multiboot2))
    }

    /* Code starts just after the header */
    .text : {
        *(.text*)
    }

    .rodata : {
        *(.rodata)
    }

    .data : {
        *(.data)
    }

    .bss : {
        *(.bss)
        *(COMMON)
    }
}
