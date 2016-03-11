#ifndef LCD_H
#define	LCD_H

typedef enum {
        BATTERY_OK,
        BATTERY_LOW
} BatteryMessage;

typedef enum {
        CELLULAR_OK,
        CELLULAR_FAILED,
        CELLULAR_PENDING,
        CELLULAR_LOW
} CellularMessage;

typedef enum {
        STAT_BOOTING,
        STAT_READY,
        STAT_SHUTDOWN
} StatusMessage;

void lcd_init(void);
void lcd_init_display(void);
void lcd_init_spi(void);
void lcd_write_string(char*, unsigned char, unsigned char);
void lcd_write_bitmap(unsigned char*);
void lcd_clear(void);
void lcd_set_battery(BatteryMessage);
void lcd_set_cellular(CellularMessage);
void lcd_set_tags(int);
void lcd_set_status(StatusMessage);

#endif	/* LCD_H */