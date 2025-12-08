	.text
	.globl _main, ___main, _start, ___stack_top
_start:
	mov pc, sp
	sub $4, sp
	jsr pc, _main
	halt

___main:
	rts pc
