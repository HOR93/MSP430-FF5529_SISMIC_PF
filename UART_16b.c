#include <msp430.h> 
void usci_a1_config(void);
void uart_char(char x);
void uart_str(char *pt);
void uart_crlf(char z);
void uart_dec8(char x);
void uart_dec16(unsigned int x);

int main(void){
    unsigned int cont = 0;
    WDTCTL = WDTPW | WDTHOLD; // stop watchdog timer
    usci_a1_config();
    while(1){
        uart_dec16(cont);
        uart_crlf(1); // Adiciona uma nova linha após cada número exibido
        cont++;
        __delay_cycles(100000); // Pequeno delay para evitar overflow no terminal
    }
    return 0;
}

// USCI_A1: Configurar Serial 1 (MSP <=> PC)
// P4.4 = TX (MSP ==> PC)
// P4.5 = RX (MSP <== PC)

void usci_a1_config(void){
    UCA1CTL1 = UCSSEL_2 | UCSWRST; // Seleciona SMCLK e mantém UCSI em Reset
    UCA1BRW = 109; // Configura Baud Rate (1048576 / 9600)
    UCA1MCTL = UCBRS_2 | UCBRF_0; // Configuração de Modulação

    P4DIR |= BIT4; // P4.4 como saída (TX)
    P4DIR &= ~BIT5; // P4.5 como entrada (RX)
    P4SEL |= BIT5 | BIT4; // Seleciona função UART para P4.4 e P4.5

    PMAPKEYID = 0X02D52; // Libera mapeamento
    P4MAP4 = PM_UCA1TXD; // TX = Sair por P4.4
    P4MAP5 = PM_UCA1RXD; // RX = Receber por P4.5

    UCA1CTL1 &= ~UCSWRST; // Tira UCSI do Reset
}

void uart_char(char x){
    while (!(UCA1IFG & UCTXIFG)); // Espera o buffer estar disponível
    UCA1TXBUF = x; // Envia o caractere
}

void uart_str(char *pt){
    char i = 0;
    while (pt[i] != '\0') {
        uart_char(pt[i]);
        i++;
    }
}

void uart_crlf(char z){
    char i = 0;
    while (i < z) {
        uart_char('\r'); // Envia o caractere de retorno de carro (carriage return)
        uart_char('\n'); // Envia o caractere de nova linha (line feed)
        i++;
    }
}

void uart_dec8(char x){
    char aux;
    aux = x / 100;
    uart_char(aux + 0x30);
    x = x - 100 * aux;
    aux = x / 10;
    uart_char(aux + 0x30);
    x = x - 10 * aux;
    uart_char(x + 0x30);
}

void uart_dec16(unsigned int x){
    char aux;
    aux = x / 10000;
    uart_char(aux + 0x30);
    x = x - 10000 * aux;
    aux = x / 1000;
    uart_char(aux + 0x30);
    x = x - 1000 * aux;
    aux = x / 100;
    uart_char(aux + 0x30);
    x = x - 100 * aux;
    aux = x / 10;
    uart_char(aux + 0x30);
    x = x - 10 * aux;
    uart_char(x + 0x30);
}
