ENTRY(kmain);

SECTIONS {
	. = 0x100000;
	  /* mark BSS segment */
	__bss_start = ADDR(.bss);
	__bss_end   = ADDR(.bss) + SIZEOF(.bss);
}
