static void inline syscall4(int syscall_id, unsigned long long arg0, unsigned long long arg1, unsigned long long arg2) {
	asm volatile ("syscall": : "a" (syscall_id), "D" (arg0), "S" (arg1), "d" (arg2));
}
static void inline syscall2(int syscall_id, unsigned long arg0) {
	asm volatile ("syscall": : "a" (syscall_id), "D" (arg0) :);
}

static void hello(void);

void __attribute__((naked)) __attribute__((section(".text"),no_reorder)) _start(void) {
	syscall4(1, 1, (unsigned long long)hello, 12);
	syscall2(60, 0);	
}

static void __attribute__((optimize("align-functions=1"))) __attribute__((naked)) __attribute__((section(".text"),no_reorder)) hello(void)  {
	asm volatile (".ascii \"Hello World\"");
	asm volatile (".byte 0xa");
}


