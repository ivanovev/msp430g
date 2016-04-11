
#include <msp430g2231.h>
#include <legacymsp430.h>
#include "ue2hd.h"
#include <stdint.h>

#define NUM_OF_BYTE 0x1F

void i2c_init(void)
{
    _DINT();
    //P1OUT &= ~(SDA_PIN + SCL_PIN);
    //P1REN |= (SDA_PIN + SCL_PIN);
    USICTL0 = USIPE6+USIPE7+USISWRST;    // Port & USI mode setup
    USICTL1 = USII2C+USISTTIE;     // Enable I2C mode & USI interrupts
    USICKCTL = USICKPL;                  // Setup clock polarity
    USICNT |= USISCLREL;  // Disable automatic clear control
    USICTL0 &= ~USISWRST;                // Enable USI
    USICTL1 &= ~USIIFG;                  // Clear pending flag
    _EINT();
    USICTL0 &= ~USIOE;
    USICNT = (USICNT & 0xE0) + NUM_OF_BYTE;
    USISRL = 0;
    USISRH = 0;
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
    P2OUT |= HD_RS;
    if(chr & 0x80)
        P1OUT |= HD_D7;
    else
        P1OUT &= ~HD_D7;
    if(chr & 0x40)
        P1OUT |= HD_D6;
    else
        P1OUT &= ~HD_D6;
    if(chr & 0x20)
        P1OUT |= HD_D5;
    else
        P1OUT &= ~HD_D5;
    if(chr & 0x10)
        P1OUT |= HD_D4;
    else
        P1OUT &= ~HD_D4;
    lcd_clk();
    if(chr & 0x08)
        P1OUT |= HD_D7;
    else
        P1OUT &= ~HD_D7;
    if(chr & 0x04)
        P1OUT |= HD_D6;
    else
        P1OUT &= ~HD_D6;
    if(chr & 0x02)
        P1OUT |= HD_D5;
    else
        P1OUT &= ~HD_D5;
    if(chr & 0x01)
        P1OUT |= HD_D4;
    else
        P1OUT &= ~HD_D4;
    lcd_clk();
    P2OUT &= ~HD_RS;
}

void lcd_init(void)
{
    sleep(15);
    P1DIR |= (HD_E + HD_RW + HD_D4 + HD_D5 + HD_D6 + HD_D7);
    P1OUT |= (HD_D4 + HD_D5);
    P1OUT &= ~(HD_E + HD_RW + HD_D6 + HD_D7);
    P2SEL &= ~(HD_BL + HD_RS);
    P2DIR |= (HD_BL + HD_RS);
    P2OUT &= ~(HD_BL + HD_RS);
    lcd_clk();
    sleep(5);
    lcd_clk();
    sleep(5);
    lcd_clk();
    sleep(5);

    P1OUT &= ~HD_D4;
    lcd_clk();
    sleep(5);

    // Function set
    P1OUT &= ~(HD_D4 + HD_D5 + HD_D6 + HD_D7);
    lcd_clk();
    P1OUT |= (HD_D7 + HD_D6);
    lcd_clk();
    sleep(5);

    // Display on/off
    P1OUT &= ~(HD_D4 + HD_D5 + HD_D6 + HD_D7);
    lcd_clk();
    //P1OUT |= (HD_D7 + HD_D6 + HD_D5 + HD_D4);
    P1OUT |= (HD_D7 + HD_D6 + HD_D5 + HD_D4);
    lcd_clk();
    sleep(5);

    // Display clear
    P1OUT &= ~(HD_D4 + HD_D5 + HD_D6 + HD_D7);
    lcd_clk();
    P1OUT |= HD_D4;
    lcd_clk();
    sleep(5);

    // Entry mode set
    P1OUT &= ~(HD_D4 + HD_D5 + HD_D6 + HD_D7);
    lcd_clk();
    //P1OUT |= HD_D6;
    P1OUT |= (HD_D6 + HD_D5);
    lcd_clk();
    sleep(5);

    // write smth
    lcd_data('A');
    lcd_data('B');
    lcd_data('C');
    lcd_data('D');
    lcd_data('E');
    lcd_data('F');
#if 0
    for(;;)
    {
        //P1OUT ^= HD_E;
        sleep(10);
    }
#endif
}

volatile uint16_t usi_state = 0;
volatile uint16_t usi_a = 0;
volatile uint16_t usi_b = 0;
int main()
{
    WDTCTL = WDTPW + WDTHOLD;
    lcd_init();
    i2c_init();
    _BIS_SR(GIE);
    for(;;)
    {
        if(usi_b != 0)
        {
            lcd_data(usi_b);
            usi_b = 0;
        }
    }
    return 0;
}

interrupt(USI_VECTOR) usi_interrupt(void)
{
    if( USICTL1 & USISTTIFG ) // START received....
    {
        USICTL1&= ~USISTTIFG;           // Clear START flag...
        usi_state = 1;
        USICNT= 0x08;                   // 8 bits
        USICTL1|= USIIE;                // Enable Counter Interrupt
        return;
    }
    if(usi_state == 1)
    {
        usi_a = USISRL;
        USICTL0|= USIOE;            //Take SDA to ack
        USICNT= 0x01 | USISCLREL;   //One bit to ack
        usi_state = 2;
        return;
    }
    if(usi_state == 2)
    {
        USICTL1&= ~USIIFG;
        USICTL0&= ~USIOE;
        USICNT= 0x08;
        usi_state = 3;
        return;
    }
    if(usi_state == 3)
    {
        usi_b = USISRL;
        USISRL= 0;                  //Zero to ack
        USICTL0|= USIOE;            //Take SDA to ack
        USICNT= 0x01 | USISCLREL;   //One bit to ack
        usi_state = 4;
        return;
    }
    if(usi_state == 4)
    {
        USICTL0&= ~USIOE;
        USICTL1&= ~USIIFG;
        USICTL1&= ~USIIE;
        USICTL1&= ~USISTP;
        USICNT = (USICNT & 0xE0) + NUM_OF_BYTE;
        usi_state = 5;
    }
}

