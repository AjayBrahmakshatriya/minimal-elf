	.text
	.globl _start
_start:
	movl	$1, %eax
	movl	$1, %edi
	leaq	hello(%rip), %rsi
	movl	$12, %edx
	syscall
	movl	$60, %eax
	xorq	%rdi, %rdi
	syscall
hello:
	.ascii "Hello World\n"
