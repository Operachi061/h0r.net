#include "time.h"
#include <drivers/io/portio.h>

// "good artists borrow, great artists steal"
// https://github.com/elydre/profanOS/
char bcd;

unsigned char read_register(unsigned char reg) {
    outb8(0x70, reg);
    return inb8(0x71);
}

void write_register(unsigned char reg, unsigned char value) {
    outb8(0x70, reg);
    outb8(0x71, value);
}

unsigned char bcd2bin(unsigned char in_bcd) {
    return (bcd) ? ((in_bcd >> 4) * 10) + (in_bcd & 0x0F) : in_bcd;
}

void time_get(i_time_t *target) {
    target->seconds = bcd2bin(read_register(0x00));
    target->minutes = bcd2bin(read_register(0x02));
    target->hours = bcd2bin(read_register(0x04));
    target->day_of_week = bcd2bin(read_register(0x06));
    target->day_of_month = bcd2bin(read_register(0x07));
    target->month = bcd2bin(read_register(0x08));
    target->year = bcd2bin(read_register(0x09));

    target->full[0] = target->seconds;
    target->full[1] = target->minutes;
    target->full[2] = target->hours;
    target->full[3] = target->day_of_month;
    target->full[4] = target->month;
    target->full[5] = target->year;
}

int rtc_init() {
    unsigned char status;
    status = read_register(0x0B);
    status |=  0x02;             // 24 hour clock
    status |=  0x10;             // update ended interrupts
    status &= ~0x20;             // no alarm interrupts
    status &= ~0x40;             // no periodic interrupt
    bcd = !(status & 0x04);      // check if data type is BCD
    write_register(0x0B, status);
    return 0;
}