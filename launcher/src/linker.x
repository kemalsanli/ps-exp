/*
 * Flat-binary linker script for launch_pskit.
 * Produces one contiguous segment: .text, then .rodata, then .data (empty).
 * objcopy -O binary dumps the whole thing raw. RIP-relative addressing keeps
 * the code position-independent when the WebKit loader mmaps it anywhere.
 */

ENTRY(_main)

SECTIONS
{
    . = 0;

    .text : {
        *(.text._main)
        *(.text*)
    }

    .rodata : ALIGN(16) {
        *(.rodata*)
    }

    .data : ALIGN(16) {
        *(.data*)
    }

    /DISCARD/ : {
        *(.bss*)
        *(.comment*)
        *(.eh_frame*)
        *(.note*)
        *(.gnu.*)
        *(.interp)
        *(.dynamic)
        *(.dynsym)
        *(.dynstr)
        *(.hash)
        *(.gnu.hash)
        *(.plt*)
        *(.got*)
        *(.rela*)
        *(.rel*)
    }
}
