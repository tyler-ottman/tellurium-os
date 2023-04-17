OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)

ENTRY(_start)

SECTIONS
{
    . = 0x8000000;

    /* text section - code */
    .text : {
        . = ALIGN(4K);
        _stext = .; /* start of .text */
        *(.text .text.*)
        _etext = .; /* end of .text */
        . = ALIGN(4K);
    }

    /* rodata section - read-only variables */
    .rodata : {
        . = ALIGN(4K);
        _srodata = .; /* start of .rodata */
        *(.rodata .rodata.*)
        _erodata = .; /* end of of .rodata */
        . = ALIGN(4K);
    }

    /* data section - mutable variables */
    .data : {
        . = ALIGN(4K);
        _sdata = .; /* start of .data */
        *(.data .data.*)
        _edata = .; /* end of .data */
        . = ALIGN(4K);
    }

    /* bss section - zero initialized variables */
    .bss : {
        . = ALIGN(4K);
        _sbss = .; /* start of .bss */
        *(COMMON)
        *(.bss .bss.*)
        _ebss = .; /* end of .bss */
        . = ALIGN(4);
    }

}