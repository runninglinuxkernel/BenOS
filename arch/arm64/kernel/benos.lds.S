#include <asm/mm.h>

OUTPUT_ARCH(aarch64)
ENTRY(_start)

SECTIONS
{
	. = 0x80000;
	_text_boot = .;
	.text.boot : {
		*(.text.boot)
	}
	_etext_boot = .;
	_text = .;
	.text : {
		*(.text)
	}
	_etext = .;

	. = ALIGN(PAGE_SIZE);
	_rodata = .;
	.rodata : {
		*(.rodata)
	}
	_erodata = .;

	. = ALIGN(PAGE_SIZE);
	_data = .;
	.data : {
		*(.data)
		. = ALIGN(PAGE_SIZE);
		*(.data.init_task)
	}
	. = ALIGN(PAGE_SIZE);
	idmap_pg_dir = .;
	. += 3*PAGE_SIZE;
	idmap_pg_end = .;

	. = ALIGN(0x8);
	_edata = .;

	. = ALIGN(PAGE_SIZE);
	_bss = .;
	bss_begin = .;
	.bss : {
		*(.bss*)
		. = ALIGN(PAGE_SIZE);
	}
	bss_end = .;
	_ebss = .;
	_end = .;

}
