#include <interrupts/IDT.h>

#define PIC1_COMMAND 0x20
#define PIC1_DATA 0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA 0xA1

#define ICW1_INIT 0x10
#define ICW1_ICW4 0x01
#define ICW4_8086 0x01


extern "C" void isr1();
extern "C" void isr0();
extern "C" void isrIgnore();
extern "C" void loadidt();
void RemapPIC(){
    uint_8 a1,a2;
    a1 = inb8(PIC1_DATA);
    a2 = inb8(PIC2_DATA);

    outb8(PIC1_COMMAND,ICW1_INIT | ICW1_ICW4);
    outb8(PIC2_COMMAND,ICW1_INIT | ICW1_ICW4);

    outb8(PIC1_DATA,0);
    outb8(PIC2_DATA,8);

    outb8(PIC1_DATA,4);
    outb8(PIC2_DATA,2);

    outb8(PIC1_DATA,ICW4_8086);
    outb8(PIC2_DATA,ICW4_8086);

    outb8(PIC1_DATA,a1);
    outb8(PIC2_DATA,a2);
}
void InitIDT()
{

    for (uint_64 i = 0; i < 256; i++)
        AddEntry(&isrIgnore,0b10001110,i);


    AddEntry(&isr0,0b10001110,0);
    AddEntry(&isr1,0b10001110,1);


    RemapPIC();
    
    outb8(0x21,0xfd);
    outb8(0xa1,0xff);
    loadidt();
}
void AddEntry(void* Isr,int attr,int _num){
 _idt[_num].zero = 0;
 _idt[_num].ist = 0;
 _idt[_num].offset_low  = (uint_16)(((uint_64)Isr & 0x000000000000ffff));
 _idt[_num].offset_mid  = (uint_16)(((uint_64)Isr & 0x00000000ffff0000) >> 16);
 _idt[_num].offset_high = (uint_32)(((uint_64)Isr & 0xffffffff00000000) >> 32);
 _idt[_num].selector = 8;
 _idt[_num].types_attr = attr;
}
extern "C" void Isr1(){
print(int2str((uint_64)inb8(0x60)));
outb8(0x20,0x20);
outb8(0xa0,0x20);
}