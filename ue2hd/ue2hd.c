
#include <msp430g2231.h>
#include <legacymsp430.h>
#include "ue2hd.h"
#include <stdint.h>

#define USISTATE_ADDR   (1 << 0)
#define USISTATE_ACK    (1 << 1)
#define USISTATE_IN     (1 << 2)
#define USISTATE_OUT    (1 << 3)

#define LCD_ADDR_DATA   0x10
#define LCD_ADDR_CMD    0x20
#define LCD_ADDR_BL     0x30

#define Q_SZ            40
struct Queue {
    volatile uint16_t head, tail;
    uint8_t q[Q_SZ];
} q1;

void i2c_init(void)
{
    USICTL0 = USIPE6+USIPE7+USISWRST;    // Port & USI mode setup
    USICTL1 = USII2C+USISTTIE;     // Enable I2C mode & USI interrupts
    USICKCTL = USICKPL;                  // Setup clock polarity
    USICNT |= USISCLREL;  // Disable automatic clear control
    USICTL0 &= ~USISWRST;                // Enable USI
    USICTL1 &= ~USIIFG;                  // Clear pending flag
    USICNT = (USICNT & 0xE0) + 0x1F;
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

#define HD_PIN(c, mask, pin) do { \
    if(c & mask) \
        P1OUT |= pin; \
    else \
        P1OUT &= ~pin; \
} while(0)

void lcd_d(uint8_t chr)
{
    HD_PIN(chr, 0x08, HD_D7);
    HD_PIN(chr, 0x04, HD_D6);
    HD_PIN(chr, 0x02, HD_D5);
    HD_PIN(chr, 0x01, HD_D4);
    lcd_clk();
}

void lcd_data(uint8_t chr, uint8_t rs)
{
    if(rs)
        P2OUT |= HD_RS;
    lcd_d(chr >> 4);
    lcd_d(chr);
    P2OUT &= ~HD_RS;
    sleep(1);
}

void lcd_bl(uint8_t bl)
{
    if(bl)
        P2OUT |= HD_BL;
    else
        P2OUT &= ~HD_BL;
}

#if 0
uint16_t lcd_bf(void)
{
    uint16_t bf = 0;
    P1DIR &= ~(HD_D4 + HD_D5 + HD_D6 + HD_D7);
    P1OUT |= HD_RW;
    P2OUT &= ~HD_RS;
    P1OUT |= HD_E;
    sleep(1);
    bf = (P1IN & HD_D7);
    P1OUT &= ~HD_E;
    sleep(1);
    P1OUT |= HD_E;
    sleep(1);
    P1OUT &= ~HD_E;
    sleep(1);
    P1OUT &= ~HD_RW;
    P1DIR |= (HD_D4 + HD_D5 + HD_D6 + HD_D7);
    return bf;
}
#endif

void lcd_init(void)
{
    sleep(15);
    P1DIR |= (HD_E + HD_RW + HD_D4 + HD_D5 + HD_D6 + HD_D7);
    P1OUT &= ~(HD_E + HD_RW);
    P2SEL &= ~(HD_BL + HD_RS);
    P2DIR |= (HD_BL + HD_RS);
    P2OUT &= ~(HD_BL + HD_RS);

    lcd_d(0x3);
    sleep(5);
    lcd_clk();
    lcd_clk();

    lcd_d(0x2);

    // Function set
    lcd_data(0x0C, 0);

    // Display on/off
    lcd_data(0x0C, 0);

    // Display clear
    lcd_data(0x01, 0);

    // Entry mode set
    lcd_data(0x06, 0);

#if 0
    for(;;)
    {
        //P1OUT ^= HD_E;
        sleep(10);
    }
#endif
}

volatile uint16_t usi_state = 0;
volatile uint16_t usi_addr = 0;
volatile uint16_t usi_counter = 0;

int main()
{
    WDTCTL = WDTPW + WDTHOLD;
    lcd_init();
    i2c_init();
    q1.head = 0;
    q1.tail = 0;
    _BIS_SR(GIE);
    volatile uint8_t chr = 0, init = 1;
    for(;;)
    {
#if 1
        if((usi_addr != 0) && (usi_addr != LCD_ADDR_BL))
        {
            init = 0;
        }
        if(init == 1)
        {
            if(chr == 17)
            {
                lcd_data(0x01, 0);
                lcd_data(0x02, 0);
                chr = 0;
            }
            sleep(1000);
            lcd_data('.', 1);
            chr++;
        }
#endif
        if(q1.tail != q1.head)
        {
            chr = q1.q[q1.tail];
            q1.tail = ((q1.tail + 1) == Q_SZ) ? 0 : q1.tail + 1;
            if(usi_addr == LCD_ADDR_BL)
            {
                lcd_bl(chr);
                continue;
            }
            if(chr)
            {
                lcd_data(chr, (usi_addr == LCD_ADDR_CMD) ? 0 : 1);
                sleep(5);
            }
        }
    }
    return 0;
}

interrupt(USI_VECTOR) usi_interrupt(void)
{
    usi_counter += 1;
    if(USICTL1 & USISTTIFG) // START received....
    {
        USICTL1 &= ~USISTTIFG;           // Clear START flag...
        USICNT = 0x88;                   // 8 bits
        USICTL1 |= USIIE;                // Enable Counter Interrupt
        usi_state = USISTATE_ADDR;
        usi_addr = 0;
        return;
    }
    if(usi_state == USISTATE_ADDR)
    {
        usi_addr = USISRL;
#if 1
        if((usi_addr != LCD_ADDR_DATA) && (usi_addr != LCD_ADDR_CMD) && (usi_addr != LCD_ADDR_BL))
        {
            usi_state = 0;
            i2c_init();
            return;
        }
#endif
        usi_state &= ~USISTATE_ADDR;
        usi_state |= USISTATE_IN | USISTATE_ACK;
    }
    if(usi_state == USISTATE_IN)
    {
        q1.q[q1.head] = USISRL;
        q1.head = ((q1.head + 1) == Q_SZ) ? 0 : (q1.head + 1);
        usi_state |= USISTATE_ACK;
    }
    if(usi_state & USISTATE_ACK)
    {
        USISRL = 0;                  //Zero to ack
        if(USICTL0 & USIOE)
        {
            USICTL1 &= ~USIIFG;
            USICTL0 &= ~USIOE;
            USICNT = 0x08;
            usi_state &= ~USISTATE_ACK;
        }
        else
        {
            USICTL0 |= USIOE;            //Take SDA to ack
            //USICNT = 0x01 | USISCLREL;   //One bit to ack
            USICNT = 0x01;   //One bit to ack
        }
    }
}

