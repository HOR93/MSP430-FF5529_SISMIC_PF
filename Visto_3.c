#include <msp430.h>
#define TRUE 1
#define FALSE 0

// Defini��o do endere�o do PCF_8574
#define PCF_ADR1 0x3F
#define PCF_ADR2 0x27
#define PCF_ADR PCF_ADR2

#define BR_100K 11 //SMCLK/100K = 11
#define BR_50K 21 //SMCLK/50K = 21
#define BR_10K 105 //SMCLK/10K = 105

void lcd_inic(void);
void lcd_aux(char dado);
int pcf_read(void);
void pcf_write(char dado);
int pcf_teste(char adr);
void led_vd(void);
void led_VD(void);
void led_vm(void);
void led_VM(void);
void i2c_config(void);
void gpio_config(void);
void delay(long limite);
void led_char(char x);
void lcd_str(char *pt);
void lcd_dec8(char x);
void lcd_cursor (char x);
void led_float(float z, char nr);
void TA0_CONFIG(void);
void ADC_CONFIG(void);


int main(void){
    WDTCTL = WDTPW | WDTHOLD; // stop watchdog timer

    gpio_config();
    i2c_config();
    TA0_CONFIG();
    ADC_CONFIG();

    volatile unsigned int medx, medy;
    float vx, vy;

    if (pcf_teste(PCF_ADR)==FALSE){
        led_VM(); //Indicar que n�o houve ACK
        while(TRUE); //Travar
    }
    else led_VD(); //Houve ACK, tudo certo

    lcd_inic(); //Inicializar LCD
    pcf_write(8); //Acender Back Light

    while((ADC12IFG&ADC12IFG7)==0);
    medx = ADC12MEM0 + ADC12MEM2 + ADC12MEM4 + ADC12MEM6;
    medy = ADC12MEM1 + ADC12MEM3 + ADC12MEM5 + ADC12MEM7;
    medx = medx/4;
    medy = medy/4;

    vx = medx*(3.3/4095);

    lcd_cursor(0x40);
    led_float(vx, vy);

    while(TRUE); //Travar execu��o

    return 0;
}
// Incializar LCD modo 4 bits
void lcd_inic(void){

    // Preparar I2C para operar
    UCB0I2CSA = PCF_ADR; //Endere�o Escravo
    UCB0CTL1 |= UCTR | //Mestre TX
            UCTXSTT; //Gerar START
    while ( (UCB0IFG & UCTXIFG) == 0); //Esperar TXIFG=1
    UCB0TXBUF = 0; //Sa�da PCF = 0;
    while ( (UCB0CTL1 & UCTXSTT) == UCTXSTT); //Esperar STT=0
    if ( (UCB0IFG & UCNACKIFG) == UCNACKIFG) //NACK?
        while(1);

    // Come�ar inicializa��o
    lcd_aux(0); //RS=RW=0, BL=1
    delay(20000);
    lcd_aux(3); //3
    delay(10000);
    lcd_aux(3); //3
    delay(10000);
    lcd_aux(3); //3
    delay(10000);
    lcd_aux(2); //2

    // Entrou em modo 4 bits
    lcd_aux(2); lcd_aux(8); //0x28
    lcd_aux(0); lcd_aux(8); //0x08
    lcd_aux(0); lcd_aux(1); //0x01
    lcd_aux(0); lcd_aux(6); //0x06
    lcd_aux(0); lcd_aux(0xF); //0x0F

    while ( (UCB0IFG & UCTXIFG) == 0) ; //Esperar TXIFG=1
    UCB0CTL1 |= UCTXSTP; //Gerar STOP
    while ( (UCB0CTL1 & UCTXSTP) == UCTXSTP) ; //Esperar STOP
    delay(50);
}

// Auxiliar inicializa��o do LCD (RS=RW=0)
// *** S� serve para a inicializa��o ***
void lcd_aux(char dado){
    while ( (UCB0IFG & UCTXIFG) == 0); //Esperar TXIFG=1
    UCB0TXBUF = ((dado<<4)&0XF0) | BIT3; //PCF7:4 = dado;
    delay(50);
    while ( (UCB0IFG & UCTXIFG) == 0); //Esperar TXIFG=1
    UCB0TXBUF = ((dado<<4)&0XF0) | BIT3 | BIT2; //E=1
    delay(50);
    while ( (UCB0IFG & UCTXIFG) == 0); //Esperar TXIFG=1
    UCB0TXBUF = ((dado<<4)&0XF0) | BIT3; //E=0;
}
// Ler a porta do PCF
int pcf_read(void){
    int dado;
    UCB0I2CSA = PCF_ADR; //Endere�o Escravo
    UCB0CTL1 &= ~UCTR; //Mestre RX
    UCB0CTL1 |= UCTXSTT; //Gerar START
    while ( (UCB0CTL1 & UCTXSTT) == UCTXSTT);
    UCB0CTL1 |= UCTXSTP; //Gerar STOP + NACK
    while ( (UCB0CTL1 & UCTXSTP) == UCTXSTP) ; //Esperar STOP
    while ( (UCB0IFG & UCRXIFG) == 0); //Esperar RX
    dado=UCB0RXBUF;
    return dado;
}

// Escrever dado na porta
void pcf_write(char dado){
    UCB0I2CSA = PCF_ADR; //Endere�o Escravo
    UCB0CTL1 |= UCTR | //Mestre TX
            UCTXSTT; //Gerar START
    while ( (UCB0IFG & UCTXIFG) == 0) ; //Esperar TXIFG=1
    UCB0TXBUF = dado; //Escrever dado
    while ( (UCB0CTL1 & UCTXSTT) == UCTXSTT) ; //Esperar STT=0
    if ( (UCB0IFG & UCNACKIFG) == UCNACKIFG) //NACK?
        while(1); //Escravo gerou NACK
    UCB0CTL1 |= UCTXSTP; //Gerar STOP
    while ( (UCB0CTL1 & UCTXSTP) == UCTXSTP) ; //Esperar STOP
}

// Testar endere�o I2C
// TRUE se recebeu ACK
int pcf_teste(char adr){
    UCB0I2CSA = adr; //Endere�o do PCF
    UCB0CTL1 |= UCTR | UCTXSTT; //Gerar START, Mestre transmissor
    while ( (UCB0IFG & UCTXIFG) == 0); //Esperar pelo START
    UCB0CTL1 |= UCTXSTP; //Gerar STOP
    while ( (UCB0CTL1 & UCTXSTP) == UCTXSTP); //Esperar pelo STOP
    if ((UCB0IFG & UCNACKIFG) == 0) return TRUE;
    else return FALSE;
}

// Configurar UCSB0 e Pinos I2C
// P3.0 = SDA e P3.1=SCL
void i2c_config(void){
    UCB0CTL1 |= UCSWRST; // UCSI B0 em ressete
    UCB0CTL0 = UCSYNC | //S�ncrono
            UCMODE_3 | //Modo I2C
            UCMST; //Mestre
    UCB0BRW = BR_100K; //100 kbps
    P3SEL |= BIT1 | BIT0; // Use dedicated module
    UCB0CTL1 = UCSSEL_2; //SMCLK e remove ressete
}

void led_vd(void) {P4OUT &= ~BIT7;} //Apagar verde
void led_VD(void) {P4OUT |= BIT7;} //Acender verde
void led_vm(void) {P1OUT &= ~BIT0;} //Apagar vermelho
void led_VM(void) {P1OUT |= BIT0;} //Acender vermelho

// Configurar leds
void gpio_config(void){
    P1DIR |= BIT0; //Led vermelho
    P1OUT &= ~BIT0; //Vermelho Apagado
    P4DIR |= BIT7; //Led verde
    P4OUT &= ~BIT7; //Verde Apagado
}

void delay(long limite){
    volatile long cont=0;
    while (cont++ < limite) ;
}

void led_char(char X){

    char msn, lsn;

    msn = X&0xf0;
    lsn = (X&0xf)<<4;

    pcf_write(msn|9);
    pcf_write(msn|0xD);
    pcf_write(msn|9);

    pcf_write(lsn|9);
    pcf_write(lsn|0xD);
    pcf_write(lsn|9);

}



void lcd_str(char *pt){
    char i=0;
    while (pt[i] !='\0'){
        led_char(pt[i]);
        i++;
    }
}

void lcd_cursor (char x){
    x = x|0x80;
    char msn, lsn;

    msn = x&0xf0;
    lsn = (x&0xf)<<4;
}

void TA0_CONFIG(void){
    TA0CTL = TASSEL_1 | MC_1;
    TA0CCR0 = 1024;
    TA0CCTL1 = OUTMOD_6;
    TA0CCR1 = TA0CCR0/2;
}

void ADC_CONFIG(void){
    ADC12CTL0 = ADC12ON;
    ADC12CTL1 = ADC12CSTARTADD_0 | ADC12SSEL_3 | ADC12CONSEQ_3 | ADC12SHS_1;
    ADC12CTL2 = ADC12RES_2;

    ADC12MCTL0 = ADC12INCH_1;
    ADC12MCTL1 = ADC12INCH_2;
    ADC12MCTL2 = ADC12INCH_1;
    ADC12MCTL3 = ADC12INCH_2;
    ADC12MCTL4 = ADC12INCH_1;
    ADC12MCTL5 = ADC12INCH_2;
    ADC12MCTL6 = ADC12INCH_1;
    ADC12MCTL7 = ADC12INCH_2;
    ADC12MCTL7 |= ADC12EOS;

    ADC12CTL0 |= ADC12ENC;

}
void led_float(float z, char nr){
    volatile float v = z;
    char x;
    x = v;
    led_char(0x30+x);
    led_char(',');
    v = v - x;
    while(nr > 0){
        v = 10*v;
        x=v;
        led_char(0x30+x);
        v=v-x;
        nr--;
    }
}
