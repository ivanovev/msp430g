#include <msp430g2231.h>
#include <legacymsp430.h>

volatile int do_spray = 0;
volatile unsigned int counter = 0;

void Spray()
{
    P1OUT |= BIT4;
    P1OUT |= BIT5;
	volatile unsigned int i = 2;
    for(i = 0; i < 4; i++)
        __delay_cycles(100000);
    P1OUT &= ~BIT4;
    P1OUT &= ~BIT5;
}

void Blink()
{
    P1OUT |= BIT3;
    __delay_cycles(100000);
    __delay_cycles(100000);
    __delay_cycles(100000);
    P1OUT &= ~BIT3;
}

int main(void)
{
    WDTCTL = WDTPW + WDTHOLD;

    for(counter = 0; counter < 15; counter++)
        __delay_cycles(100000);

    P1SEL = 0x00;

    P1DIR |= (BIT3 | BIT4 | BIT5);
    P1OUT &= ~(BIT3 | BIT4 | BIT5);

    P1DIR &= ~BIT2;
    P1IE |= BIT2;
    P1IES |= BIT2;
    P1IFG &= ~BIT2;


    P2SEL = 0x00;
    P2DIR |= BIT6;
    P2OUT &= ~BIT6;

/*
    P1DIR |= BIT0;
    P1OUT &= ~BIT0;
    P2DIR |= BIT6;
    P2SEL = 0x00;
    P2OUT &= ~BIT6;
    P1DIR &= ~(BIT1 | BIT6);
    P1REN |= BIT6;
    P1IE |= BIT6;
    P1IES |= BIT6;
    P1IFG &= ~BIT6;
*/
    //_BIS_SR(CPUOFF + GIE);
    _BIS_SR(GIE);
    for(;;)
    {
        if(do_spray)
        {
            Spray();
            do_spray = 0;
            counter = 0;
        }
        else if(counter >= 30)
        {
            Blink();
            counter = 0;
        }
        else
        {
            __delay_cycles(100000);
            counter++;
        }
    }
/*
    {
	volatile unsigned int i;
	P2OUT ^= 0xC0;
	i = 100000;
	do(i--);
	while(i != 0);
    }
*/
    return 0;
}

#if 1
interrupt(PORT1_VECTOR) button_pressed(void)
{
    //P2OUT ^= BIT6;
    //P1OUT ^= BIT5;
    do_spray = 1;
    P1IFG &= ~BIT2;
}
#endif

