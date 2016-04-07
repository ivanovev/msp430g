
#include <msp430g2231.h>
#include <legacymsp430.h>
#include "ue2hd.h"
#include <stdint.h>

void i2c_init(void)
{
    P1OUT = SDA_PIN + SCL_PIN;
    P1REN |= SDA_PIN + SCL_PIN;
    USICTL0 = USIPE6+USIPE7+USISWRST;    // Port & USI mode setup
    USICTL1 = USII2C+USIIE+USISTTIE;     // Enable I2C mode & USI interrupts
    USICKCTL = USICKPL;                  // Setup clock polarity
    USICNT |= USIIFGCC;                  // Disable automatic clear control
    USICTL0 &= ~USISWRST;                // Enable USI
    USICTL1 &= ~USIIFG;                  // Clear pending flag
    _EINT();
}

void sleep(uint16_t ms)
{
    volatile unsigned int i = ms*120;
    do(i--);
    while(i != 0);

}

void lcd_clk(void)
{
    sleep(1);
    P1OUT |= HD_E;
    sleep(1);
    P1OUT &= ~HD_E;
    sleep(1);
}

void lcd_data(uint8_t chr)
{
    P1OUT |= HD_RS;
    if(chr & 0x80)
        P1OUT |= HD_D7;
    if(chr & 0x40)
        P1OUT |= HD_D6;
    if(chr & 0x20)
        P1OUT |= HD_D5;
    if(chr & 0x10)
        P1OUT |= HD_D4;
    lcd_clk();
    if(chr & 0x08)
        P1OUT |= HD_D7;
    if(chr & 0x04)
        P1OUT |= HD_D6;
    if(chr & 0x02)
        P1OUT |= HD_D5;
    if(chr & 0x01)
        P1OUT |= HD_D4;
    lcd_clk();
}

void lcd_init(void)
{
    sleep(15);
    P1DIR |= (HD_E + HD_RW + HD_D4 + HD_D5 + HD_D6 + HD_D7);
    P1OUT = ~(HD_E + HD_RW + HD_D6 + HD_D7);
    P2SEL &= ~(HD_BL + HD_RS);
    P2DIR |= (HD_BL + HD_RS);
    P2OUT &= ~(HD_BL + HD_RS);
    lcd_clk();
    sleep(5);
    lcd_clk();
    sleep(5);
    lcd_clk();
    sleep(5);

    P1OUT &= ~HD_D5;
    lcd_clk();

    // Function set
    sleep(5);
    P1OUT &= ~HD_D4;
    lcd_clk();
    sleep(5);

    // Display off
    P1OUT = 0;
    lcd_clk();
    P1OUT |= HD_D7;
    lcd_clk();

    // Display clear
    P1OUT = 0;
    lcd_clk();
    P1OUT |= HD_D4;
    lcd_clk();

    // Entry mode set
    P1OUT = 0;
    lcd_clk();
    P1OUT = HD_D5;
    lcd_clk();

    // write smth
    lcd_data('B');
    for(;;)
    {
        //P1OUT ^= HD_E;
        sleep(10);
    }
}

int main()
{
    WDTCTL = WDTPW + WDTHOLD;
    //i2c_init();
    lcd_init();
    P2SEL &= 0x7F;
    P2DIR |= 0x80;
    P2OUT ^= 0x80;
    for(;;)
    {
        volatile unsigned int i = 60000;
        //P2OUT ^= 0x80;
        do(i--);
        while(i != 0);
    }
    return 0;
}
