#include <p32xxxx.h>
#include <plib.h>

#ifndef PICSETUP_H
#define	PICSETUP_H

#define SYS_FREQ 40000000L
#define PBCLK  (SYS_FREQ)

#define Fsck	50000
#define BRG_VAL ((PBCLK/2/Fsck)-2)

#define POWERKEY LATAbits.LATA2
#define GSM_LED LATBbits.LATB4
#define RFID_LED LATBbits.LATB15

void setup_pins(void);
void pic_reset(void);

#endif	/* PICSETUP_H */