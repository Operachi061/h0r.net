#ifndef __ISR_H__
#define __ISR_H__
#include <types/stdtypes.h>

typedef struct 
{
	uint64_t rax;
	uint64_t rbx;
	uint64_t rcx;
	uint64_t rdx;
	uint64_t rsi;
	uint64_t rdi;
	uint64_t rbp;
	uint64_t r8;
	uint64_t r9;
	uint64_t r10;
	uint64_t r11;
	uint64_t r12;
	uint64_t r13;
	uint64_t r14;
	uint64_t r15;

	uint64_t interrupt;
	uint64_t error;

	uint64_t rip;
	uint64_t cs;
	uint64_t rflags;
	uint64_t rsp;
	uint64_t ss;
} __attribute__((packed)) Registers;

typedef void (*ISRHandler)(Registers* regs);

void init_ISR();
void ISR_RegisterHandler(int irq, ISRHandler handler);

#endif // __ISR_H__
