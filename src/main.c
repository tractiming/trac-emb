#define SETUP
//#define USE_BATTERY_MONITOR    /* Uncomment to enable battery monitoring. */
#include "picsetup.h"
#include "comm.h"
#include "gsm.h"
#include "rfid.h"
#include "lcd.h"

#define LOOP_DELAY 2750                   // Delay between updates (in msec)
#define RSSI_CHECK    7                   // Interval yo check signal strength
#define RFID_CHECK   10                   // Interval to check alien status

const char reader_id[] = "Z2112";         // Unique reader id for this device
int battery_state = 1;

int main(void)
{
        int gsm_not_init = 1;
        int cnt, total = 0;
        int rssi;
        int loop = 0;
        int alien_chk = 0;
        int battery_state = 1;
        char ctime[50];
        char post_msg[MAX_MSG_LEN];

        CFGCONbits.JTAGEN = 0;
        SYSTEMConfigPerformance(SYS_FREQ);
        setup_pins();
        GSM_LED = 0;
        RFID_LED = 1;

        lcd_init_spi();
        lcd_init();
        lcd_init_display();
        lcd_set_battery(BATTERY_OK);
        lcd_set_cellular(CELLULAR_PENDING);
        lcd_set_status(STAT_BOOTING);
        lcd_set_tags(0);

#ifdef USE_BATTERY_MONITOR
        setup_battery_int();
#endif

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
                        delay_ms(2000);
                        gsm_pwr_off(&gsm_state);
                        delay_ms(10000); // wait 10 sec before trying again
                }
        }
        lcd_set_cellular(CELLULAR_OK);

        if (rfid_init()) {
                lcd_set_status(STAT_READER_ERROR);
        } else {
                gsm_get_time(&gsm_state, ctime, 50);
                rfid_set_time(ctime);

                // Point to /api/splits w/ header
                gsm_cfg_split_endpoint(&gsm_state);
                delay_ms(1000);
                GSM_LED = 1;
                lcd_set_status(STAT_READY);
        }

        while (1) {
                update_splits(&rfid_split_queue, &rfid_line_buffer);
                cnt = get_update_msg(&rfid_split_queue, reader_id, post_msg);

                if (cnt) {
                        GSM_LED = 0;
                        if (!gsm_http_post(&gsm_state, post_msg)) {
                                rfid_split_queue.tail = INCR_SPLIT_INDX(
                                        rfid_split_queue.tail, cnt);
                                total += cnt;
                        }
                        GSM_LED = 1;
                        alien_chk = 0;
                        lcd_set_tags(total);
                }

                delay_ms(LOOP_DELAY);

                loop++;
                if (loop == RSSI_CHECK) {
                        rssi = gsm_get_signal_strength(&gsm_state);
                        if ((rssi == 99) || (rssi <= GSM_LOW_SIGNAL))
                                lcd_set_cellular(CELLULAR_LOW);
                        else
                                lcd_set_cellular(CELLULAR_OK);
                        loop = 0;
                }

                alien_chk++;
                if (alien_chk == RFID_CHECK) {
                        if (!rfid_check_running(&rfid_line_buffer))
                                lcd_set_status(STAT_READER_ERROR);
                        else
                                lcd_set_status(STAT_READY);
                        alien_chk = 0;
                }
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
        lcd_set_status(STAT_SHUTDOWN);

        // Send shutoff signal to GSM.
        if (gsm_on)
                gsm_pwr_off(&gsm_state);

        // Send KILL signal to timer. (Not implemented.)

        // Disable the other UART interrupts, stopping all communication.
        INTEnable(RFID_RX_INT, INT_DISABLED);
        INTEnable(GSM_RX_INT, INT_DISABLED);

        GSM_LED = 0;
        RFID_LED = 0;

        mINT3ClearIntFlag();
}

#ifdef USE_BATTERY_MONITOR
/* Battery monitor ISR. */
void __ISR(_EXTERNAL_1_VECTOR, IPL5AUTO) voltage_isr(void)
{
        battery_state = 0;
        INTClearFlag(INT_INT1);
}
#endif
