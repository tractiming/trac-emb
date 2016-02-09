#define SETUP
#include "picsetup.h"
#include "comm.h"
#include "gsm.h"
#include "rfid.h"

#define LOOP_DELAY 2750 // Delay between updates (in msec)

const char reader_id[] = "A1016"; // Unique reader id for this device.

int main(void)
{
        int gsm_not_init = 1;
        char ctime[50];
        char post_msg[MAX_MSG_LEN];

        CFGCONbits.JTAGEN = 0;
        SYSTEMConfigPerformance(SYS_FREQ);
        setup_pins();
        GSM_LED = 0;
        RFID_LED = 0;

        setup_shutdown_int();
        uart_init(GSM_UART, GSM_BAUDRATE, GSM_RX_INT, GSM_INT_VEC,
                  INT_PRIORITY_LEVEL_6, INT_SUB_PRIORITY_LEVEL_0);
        uart_init(RFID_UART, RFID_BAUDRATE, RFID_RX_INT, RFID_INT_VEC,
                  INT_PRIORITY_LEVEL_6, INT_SUB_PRIORITY_LEVEL_1);

        INTEnableSystemMultiVectoredInt();
        delay_ms(5000);

        while (gsm_not_init) {
                gsm_not_init = gsm_init(&gsm_state);
                if (gsm_not_init) {
                        gsm_pwr_off(&gsm_state);
                        delay_ms(10000); // wait 10 sec before trying again
                }
        }

        rfid_init();
        gsm_get_time(&gsm_state, ctime, 50);
        rfid_set_time(ctime);

        gsm_cfg_split_endpoint(&gsm_state); // Point to /api/splits w/ header
        delay_ms(1000);
        GSM_LED = 1;
        
        while (1) {
                update_splits(&rfid_split_queue, &rfid_line_buffer);

                if (get_update_msg(&rfid_split_queue, reader_id, post_msg)) {
                        GSM_LED = 0;
                        gsm_http_post(&gsm_state, post_msg);
                        GSM_LED = 1;
                }
        
                delay_ms(LOOP_DELAY);
        }

        return 0;
}

/* Interrupt for handling uart communication with gsm module. */
void __ISR(GSM_UART_VEC, IPL6SOFT) IntGSMUartHandler(void)
{
        if (INTGetFlag(INT_SOURCE_UART_RX(GSM_UART))) {
                char data = UARTGetDataByte(GSM_UART);
                gsm_add_to_buffer(&gsm_state, data);

                INTClearFlag(INT_SOURCE_UART_RX(GSM_UART));
        }

        if (INTGetFlag(INT_SOURCE_UART_TX(GSM_UART))) {
                INTClearFlag(INT_SOURCE_UART_TX(GSM_UART));
        }
}

/* Interrupt for handling uart communication with rfid reader. */
void __ISR(RFID_UART_VEC, IPL6SOFT) IntRFIDUartHandler(void)
{
        if (INTGetFlag(INT_SOURCE_UART_RX(RFID_UART))) {
                char data = UARTGetDataByte(RFID_UART);
                rfid_add_to_buffer(&rfid_line_buffer, data);

                INTClearFlag(INT_SOURCE_UART_RX(RFID_UART));
        }

        if (INTGetFlag(INT_SOURCE_UART_TX(RFID_UART))) {
                INTClearFlag(INT_SOURCE_UART_TX(RFID_UART));
        }
}

/* Shutdown ISR. */
void __ISR(_EXTERNAL_3_VECTOR, IPL5SOFT) ShutdownISR(void)
{    
        // Send shutoff signal to GSM.
        if (gsm_on)
            gsm_pwr_off(&gsm_state);

        // Send KILL signal to timer. (Not implemented.)

        // Disable the other UART interrupts, stopping all communication.
        INTEnable(RFID_RX_INT, INT_DISABLED);
        INTEnable(GSM_RX_INT, INT_DISABLED);

        GSM_LED = 0;

        mINT3ClearIntFlag();
}
