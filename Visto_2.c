// bluetooth usando controle
#include <msp430.h> 
#define partm (0.9 * 14051)
#define partM (1.1 * 14051)

#define zerom (0.9 * 1237)
#define zeroM (1.1 * 4051)

#define umm (0.9 * 2359) // novo
#define umM (1.1 * 2359) // novo

#define TRUE 1
#define FALSE 0

void config_leds(void);
char partida(void);
void ler_pulsos(void); // novo
void decodificar(void); // novo
void comando(void); // novo

volatile long codigo = 0;
volatile long vet[32]; // Vetor global para armazenar os pulsos

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD; // stop watchdog timer
    config_leds();

    TA1CTL = TASSEL_2 | MC_2; // SMCLK, modo contínuo
    TA1CCTL1 = CM_2 | SCS | CAP; // Modo captura, SCS
    P2DIR &= ~BIT0; // P2.0 como entrada
    P2SEL |= BIT0; // P2.0 selecionado para TA1.CCI1A

    while(1) {
        TA1CCTL1 = CM_2 | SCS | CAP;
        TA1CCTL1 &= ~CCIFG;
        if(partida() == TRUE) {
            ler_pulsos(); // novo
            decodificar(); // novo
            comando(); // novo
        }
    }

    return 0;
}

void config_leds(void) {
    P4DIR |= BIT7;
    P1DIR |= BIT0;
    P4OUT &= ~BIT7;
    P1OUT &= ~BIT0;
}

char partida(void) {
    long x, y = 0, DIF;

    while(1) {
        TA1CCTL1 &= ~CCIFG;
        while((TA1CCTL1 & CCIFG) == 0);
        x = y;
        y = TA1CCR1;
        DIF = y - x;
        if(DIF < 0)
            DIF += 65536;
        if (DIF > partm && DIF < partM)
            return TRUE;
    }
}

void ler_pulsos(void) { // novo
    long x, y, DIF;
    int i;

    TA1CCTL1 = CM_1 | SCS | CAP;
    TA1CCTL1 &= ~CCIFG;


    while((TA1CCTL1 & CCIFG) == 0);
    x = TA1CCR1;

    for (i = 0; i < 32; i++) {
        TA1CCTL1 &= ~CCIFG;
        while((TA1CCTL1 & CCIFG) == 0);
        y = TA1CCR1;
        DIF = y - x;
        if (DIF < 0)
            DIF += 65536;
        vet[i] = DIF;
        x = y;
    }
}

void decodificar(void) { // novo
    int i;
    codigo = 0;

    for (i = 0; i < 32; i++) {
        codigo >>= 1; // Desloca o código para a esquerda
        if (umm < vet[i] && vet[i] < umM) {
            codigo |= 0x80000000L; // Define o bit como 1
        } else if (zerom < vet[i] && vet[i] < zeroM) {
            // O bit já é 0, então não é necessário fazer nada
        } else {
            return;
        }
    }
}

void comando(void) { // novo
    codigo=codigo>>16;
    switch (codigo) {
    case 0XF30C: // Acende vermelho
        P1OUT |= BIT0;
        break;
    case 0XF708: // Apaga vermelho
        P1OUT &= ~BIT0;
        break;
    case 0XBD42: // Inverte vermelho
        P1OUT ^= BIT0;
        break;
    case 0XE718: // Acende verde
        P4OUT |= BIT7;
        break;
    case 0XE31C: // Apaga verde
        P4OUT &= ~BIT7;
        break;
    case 0XAD52: // Inverte verde
        P4OUT ^= BIT7;
        break;
    default:
        break;
    }
}
