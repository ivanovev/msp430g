
#if 0 // v0
#define HD_RW   BIT0
#define HD_E    BIT1

#define HD_D4   BIT2
#define HD_D5   BIT3
#define HD_D6   BIT4
#define HD_D7   BIT5

#define SDA_PIN BIT7
#define SCL_PIN BIT6

#define HD_RS   BIT6
#define HD_BL   BIT7
#else // v1
#define HD_RW   BIT0
#define HD_E    BIT1

#define HD_D4   BIT2
#define HD_D5   BIT3
#define HD_D6   BIT4
#define HD_D7   BIT5

#define SDA_PIN BIT7
#define SCL_PIN BIT6

#define HD_BL   BIT6
#define HD_RS   BIT7
#endif
