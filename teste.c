#include <msp430.h>
#include <string.h>

#define TRUE    1
#define FALSE   0

// Definição do endereço do PCF_8574
#define PCF_ADR1 0x3F
#define PCF_ADR2 0x27
#define PCF_ADR  PCF_ADR2

#define BR_100K    11  //SMCLK/100K = 11
#define BR_50K     21  //SMCLK/50K  = 21
#define BR_10K    105  //SMCLK/10K  = 105

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
void simple_hash(char *input, char *output);
void simple_hash_encrypt(char *input, char *output);
void lcd_str_n(char *pt, int n);
int analyze_text(char *text);  // Função de análise do texto
void start_timer(void);
void stop_timer(void);

volatile int good_password = 0;

int main(void){
    WDTCTL = WDTPW | WDTHOLD; // stop watchdog timer

    gpio_config();
    i2c_config();

    if (pcf_teste(PCF_ADR) == FALSE) {
        led_VM(); // Indicar que não houve ACK
        while(TRUE); // Travar
    } else {
        led_VD(); // Houve ACK, tudo certo
    }

    lcd_inic(); // Inicializar LCD
    pcf_write(8); // Acender Back Light

    char text[] = "seraquen"; // Texto para criptografar Str0ng!Passw0rdS
    char hash_output[33]; // Buffer para hash do texto

    // Criptografa o texto usando o hash simplificado
    simple_hash_encrypt(text, hash_output);

    lcd_clear();
    lcd_str("Projeto Sismic");
    lcd_cursor(0x40);
    lcd_str("Henrique OR.");
    __delay_cycles(4000000);                // Aproximadamente 5 segundos

    lcd_clear();
    lcd_str("Criptografia");
    lcd_clear();
    lcd_str("Gerador senha MD5");
    lcd_cursor(0x40);
    lcd_str("Analise de Hash");
    __delay_cycles(2000000);                // Aproximadamente 5 segundos

    lcd_clear();
    lcd_str("senha escolhida..");
    __delay_cycles(2000000);                // Aproximadamente 5 segundos
    lcd_clear();
    lcd_str(text);
    __delay_cycles(2000000);                // Aproximadamente 5 segundos
    lcd_clear();


    lcd_clear();
    if (analyze_text(text)) {
        lcd_str("Padrão NIST: ");
        lcd_cursor(0x40);
        lcd_str("Senha Forte.");
        good_password = 1;
        
    } else {
        lcd_str("Padrão NIST: ");
        lcd_cursor(0x40);                       // Segunda linha, primeira coluna
        lcd_str("Senha Fraca.");
        good_password = 0;
    }
    __delay_cycles(5000000);  // Aproximadamente 5 segundos

    // Configurar Timer A0 para gerar interrupções a cada 50 ms
    TA0CCR0 = 524; // 50 ms = 524 * (1/1048576 Hz) * 8
    TA0CCTL0 = CCIE; // Habilitar interrupção para CCR0
    TA0CTL = TASSEL_2 + MC_1 + ID_3; // SMCLK, modo up, divisor 8

    // Exibe a mensagem inicial no LCD
    lcd_clear();
    lcd_str("Hash processando..");
    __delay_cycles(5000000);  // Aproximadamente 5 segundos

    // Exibe o hash completo no LCD
    lcd_clear();
    // Exibe a primeira metade do hash na primeira linha.
    lcd_str(hash_output);
    // Exibe a segunda metade do hash na segunda linha
    lcd_cursor(0x40);  // Segunda linha, primeira coluna
    lcd_str(hash_output + 9);

    __enable_interrupt(); // Habilitar interrupções gerais

    while(TRUE); // Travar execução

    return 0;
}

// Função de hash simplificada (pseudo-MD5)
void simple_hash_encrypt(char *input, char *output) {
    unsigned int hash[4] = { 0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476 };
    unsigned int i, j;
    unsigned int input_len = strlen(input);
    unsigned char *buffer = (unsigned char*)input;

    for (i = 0; i < input_len; i++) {
        hash[i % 4] ^= buffer[i];
        hash[i % 4] = (hash[i % 4] << (i % 4 + 1)) | (hash[i % 4] >> (32 - (i % 4 + 1)));
        hash[i % 4] += buffer[i];
    }

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 8; j++) {
            unsigned char c = (hash[i] >> (j * 4)) & 0xF;
            if (c < 10) c += '0';
            else c += 'A' - 10;
            output[i * 8 + j] = c;
        }
    }
    output[32] = '\0';
}

int analyze_text(char *text) {
    int length = strlen(text);
    int has_upper = 0, has_lower = 0, has_digit = 0, has_special = 0;
    int i;

    // Verificar comprimento mínimo
    if (length < 8) return 0;

    // Verificar tipos de caracteres
    for (i = 0; i < length; i++) {
        char c = text[i];
        if (isupper((unsigned char)c)) {
            has_upper = 1;
        } else if (islower((unsigned char)c)) {
            has_lower = 1;
        } else if (isdigit((unsigned char)c)) {
            has_digit = 1;
        } else {
            has_special = 1;
        }
    }

    // Verificar se atende aos requisitos do NIST
    if (has_upper && has_lower && has_digit && has_special) return 1;
    return 0;
}


void lcd_clear(void) {
    lcd_cmd(0x01);
    delay(2000);
}

// Função para enviar um comando ao LCD
void lcd_cmd(char cmd) {
    char upper, lower;
    upper = cmd & 0xF0;
    lower = (cmd << 4) & 0xF0;
    pcf_write(upper | 0x0C);
    pcf_write(upper | 0x08);
    pcf_write(lower | 0x0C);
    pcf_write(lower | 0x08);
    delay(1000);
}
// Inicializar LCD modo 4 bits
void lcd_inic(void){
    // Preparar I2C para operar
    UCB0I2CSA = PCF_ADR; // Endereço Escravo
    UCB0CTL1 |= UCTR | // Mestre TX
            UCTXSTT; // Gerar START
    while ( (UCB0IFG & UCTXIFG) == 0); // Esperar TXIFG=1
    UCB0TXBUF = 0; // Saída PCF = 0;
    while ( (UCB0CTL1 & UCTXSTT) == UCTXSTT); // Esperar STT=0
    if ( (UCB0IFG & UCNACKIFG) == UCNACKIFG) // NACK?
        while(1);

    // Começar inicialização
    lcd_aux(0); // RS=RW=0, BL=1
    delay(20000);
    lcd_aux(3); // 3
    delay(10000);
    lcd_aux(3); // 3
    delay(10000);
    lcd_aux(3); // 3
    delay(10000);
    lcd_aux(2); // 2

    // Entrou em modo 4 bits
    lcd_aux(2); lcd_aux(8); // 0x28
    lcd_aux(0); lcd_aux(8); // 0x08
    lcd_aux(0); lcd_aux(1); // 0x01
    lcd_aux(0); lcd_aux(6); // 0x06
    lcd_aux(0); lcd_aux(0xF); // 0x0F

    while ( (UCB0IFG & UCTXIFG) == 0) ; // Esperar TXIFG=1
    UCB0CTL1 |= UCTXSTP; // Gerar STOP
    while ( (UCB0CTL1 & UCTXSTP) == UCTXSTP) ; // Esperar STOP
    delay(50);
}
// Auxiliar inicialização do LCD (RS=RW=0)
// *** Só serve para a inicialização ***
void lcd_aux(char dado){
    while ( (UCB0IFG & UCTXIFG) == 0); // Esperar TXIFG=1
    UCB0TXBUF = ((dado<<4)&0XF0) | BIT3; // PCF7:4 = dado;
    delay(50);
    while ( (UCB0IFG & UCTXIFG) == 0); // Esperar TXIFG=1
    UCB0TXBUF = ((dado<<4)&0XF0) | BIT3 | BIT2; // E=1
    delay(50);
    while ( (UCB0IFG & UCTXIFG) == 0); // Esperar TXIFG=1
    UCB0TXBUF = ((dado<<4)&0XF0) | BIT3; // E=0;
}

// Ler a porta do PCF
int pcf_read(void){
    int dado;
    UCB0I2CSA = PCF_ADR; // Endereço Escravo
    UCB0CTL1 &= ~UCTR; // Mestre RX
    UCB0CTL1 |= UCTXSTT; // Gerar START
    while ( (UCB0CTL1 & UCTXSTT) == UCTXSTT);
    UCB0CTL1 |= UCTXSTP; // Gerar STOP + NACK
    while ( (UCB0CTL1 & UCTXSTP) == UCTXSTP) ; // Esperar STOP
    while ( (UCB0IFG & UCRXIFG) == 0); // Esperar RX
    dado = UCB0RXBUF;
    return dado;
}
// Escrever dado na porta
void pcf_write(char dado){
    UCB0I2CSA = PCF_ADR; // Endereço Escravo
    UCB0CTL1 |= UCTR | // Mestre TX
            UCTXSTT; // Gerar START
    while ( (UCB0IFG & UCTXIFG) == 0) ; // Esperar TXIFG=1
    UCB0TXBUF = dado; // Escrever dado
    while ( (UCB0CTL1 & UCTXSTT) == UCTXSTT) ; // Esperar STT=0
    if ( (UCB0IFG & UCNACKIFG) == UCNACKIFG) // NACK?
        while(1); // Escravo gerou NACK
    UCB0CTL1 |= UCTXSTP; // Gerar STOP
    while ( (UCB0CTL1 & UCTXSTP) == UCTXSTP) ; // Esperar STOP
}

// Testar endereço I2C
// TRUE se recebeu ACK
int pcf_teste(char adr){
    UCB0I2CSA = adr; // Endereço do PCF
    UCB0CTL1 |= UCTR | UCTXSTT; // Gerar START, Mestre transmissor
    while ( (UCB0IFG & UCTXIFG) == 0); // Esperar pelo START
    UCB0CTL1 |= UCTXSTP; // Gerar STOP
    while ( (UCB0CTL1 & UCTXSTP) == UCTXSTP); // Esperar pelo STOP
    if ((UCB0IFG & UCNACKIFG) == 0) return TRUE;
    else return FALSE;
}

// Configurar UCSB0 e Pinos I2C
// P3.0 = SDA e P3.1=SCL
void i2c_config(void){
    UCB0CTL1 |= UCSWRST; // UCSI B0 em ressete
    UCB0CTL0 = UCSYNC | // Síncrono
            UCMODE_3 | // Modo I2C
            UCMST; // Mestre
    UCB0BRW = BR_100K; // 100 kbps
    P3SEL |= BIT1 | BIT0; // Use dedicated module
    UCB0CTL1 = UCSSEL_2; // SMCLK e remove ressete
}
void led_vd(void) {P4OUT &= ~BIT7;} // Apagar verde
void led_VD(void) {P4OUT |= BIT7;} // Acender verde
void led_vm(void) {P1OUT &= ~BIT0;} // Apagar vermelho
void led_VM(void) {P1OUT |= BIT0;} // Acender vermelho

// Configurar leds
void gpio_config(void){
    P1DIR |= BIT0; // Led vermelho
    P1OUT &= ~BIT0; // Vermelho Apagado
    P4DIR |= BIT7; // Led verde
    P4OUT &= ~BIT7; // Verde Apagado
}

void delay(long limite){
    volatile long cont = 0;
    while (cont++ < limite);
}

void led_char(char X){
    char msn, lsn;
    msn = X & 0xf0;
    lsn = (X & 0xf) << 4;
    pcf_write(msn | 9);
    pcf_write(msn | 0xD);
    pcf_write(msn | 9);
    pcf_write(lsn | 9);
    pcf_write(lsn | 0xD);
    pcf_write(lsn | 9);
}

void lcd_str(char *pt){
    char i = 0;
    while (pt[i] != '\0'){
        led_char(pt[i]);
        i++;
    }
}

void lcd_cursor(char x) {
    x = x | 0x80; // Comando para mover o cursor
    char msn = x & 0xF0;
    char lsn = (x & 0x0F) << 4;

    pcf_write(msn | 0x0C); // Enviar bits altos
    pcf_write(msn | 0x08); // Pulso de habilitação
    delay(50);
    pcf_write(lsn | 0x0C); // Enviar bits baixos
    pcf_write(lsn | 0x08); // Pulso de habilitação
    delay(50);
}

void lcd_str_n(char *pt, int n) {
    int i;
    for (i = 0; i < n && pt[i] != '\0'; i++) {
        led_char(pt[i]);
    }
}


#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A(void)
{
    if (good_password) {
        P4OUT ^= BIT7; // Alternar estado do LED verde
        P1OUT &= ~BIT0; // Garantir que o LED vermelho esteja apagado
    } else {
        P1OUT ^= BIT0; // Alternar estado do LED vermelho
        P4OUT &= ~BIT7; // Garantir que o LED verde esteja apagado
    }
}
