// aumenta e diminui HZ do led
#include <msp430.h> 
#define TRUE 1
#define FALSE 0
#define ABERTO 1
#define FECHADO 0

#define pwm 10480
#define passo 1048

int saida1(void);
int saida2(void);

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD; // stop watchdog timer

    P1DIR |= BIT0; //LED VERMELHO
    P1OUT &= ~BIT0;

    P2DIR &= ~BIT1; // PINO 2.1
    P2REN |= BIT1;
    P2OUT |= BIT1;

    P1DIR &= ~BIT1; // PINO 1.1
    P1REN |= BIT1;
    P1OUT |= BIT1;


    TA2CTL = TASSEL_2 | MC_1; // SMCLK e modo 1
    TA2CCR0 = pwm; // MSCLK 1.048.576* 0.01(10 ms) = 10480 e depois * 10% para passo de 1048
    TA2CCTL2 = OUTMOD_6; // saido modo 6
    TA2CCR2 = 5*passo; // inicio em 50%
    P2DIR |= BIT5; // PINO DO OUTRO LADO 2.5 e TA2CCR2 NO GUIA
    P2SEL |= BIT5;

    while(TRUE){
        if(saida1()==TRUE){ // se S1 tiver aberta
            if (TA2CCR2 < TA2CCR0) // se TA2CCR1 = 5240 < TA2CCR0 = 1048
                TA2CCR2 = TA2CCR2 + passo; // AUMENTA 10%
        }
        if(saida2()==TRUE){
            if(TA2CCR2 >0) // se TA2CCR1 = 5240 > 0
                TA2CCR2 = TA2CCR2 - passo; // DIMINUI 10%
        }
    }

    return 0;
}

int saida1(void){
    static int s1 = ABERTO; // STATIC para guardar o valor passado

    if ((P2IN&BIT1) == 0){ // se atual tiver fechado
        if (s1 == ABERTO){ // olha o valor passado
            __delay_cycles(1000);
            s1 = FECHADO;
            return TRUE;
        }
    }
    else{
        if (s1 == FECHADO){ // se tava fechado
            __delay_cycles(1000);
            s1=ABERTO; // aberto agora
            return FALSE;
        }
    }
    return FALSE;
}

int saida2(void){
    static int s2 = ABERTO;

    if ((P1IN&BIT1) == 0){
        if (s2 == ABERTO){
            __delay_cycles(1000);
            s2 = FECHADO;
            return TRUE;
        }
    }
    else{
        if (s2 == FECHADO){
            __delay_cycles(1000);
            s2=ABERTO;
            return FALSE;
        }
    }
    return FALSE;
}
