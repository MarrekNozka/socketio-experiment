#include "settings.h"
#include "lcd.h"
#include <avr/io.h>
/*#include <util/delay.h>*/
#include <stdio.h>
#include <avr/pgmspace.h>       // chceme pracovat s řetězci ve flash paměti
#include <avr/interrupt.h>      // chceme pracovat s řetězci ve flash paměti

int8_t check_status(void);


void setupUART(void)
{
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);       // povolí příjem i vysílání
    UCSR0C |= (1 << UCSZ00) | (1 << UCSZ01);    // 8-b znaky
    UBRR0 = 103;                // nastavení přenosové rychlosti, závisí na frekvenci
    UCSR0B |= (1<< RXCIE0); // povolí přerušení při příjmu
}

void setupTIMER(void)
{
    TIMSK0 = (1 << OCIE0A);     // přerušení nastane při události compare kanálu A
    TCNT0 = 0;                  // hodnota samotného čítače
    OCR0A = 82;                 // kdy nastane compare událost
    TCCR0A = (1 << WGM01);      // timer je v módu CTC
    TCCR0B = (1 << CS02);       //  nastavím zdroj Cloku s předděličkou 256
    // to co se v dokumentaci jmenuje CA02 se ve skutečnosti jmenuje CS02
}



void odesli(uint8_t znak)
{
    while (!(UCSR0A & (1 << UDRE0))) {
    };
    UDR0 = znak;
}

void myputs(uint8_t str[])
{
    for (uint8_t i = 0; str[i]; i++) {
        odesli(str[i]);
    }
}


int myputc(char znak, FILE *stream)
{
    while (!(UCSR0A & (1 << UDRE0))) {
    };
    UDR0 = znak;
    return 0;
}

int8_t check_status(void)
{
    static uint8_t minule = 0;
    if ((PINC & (1 << PC0)) && !minule) {
        // vzestupná hrana
        minule = 1;
        if (PINC & (1 << PC1)) {
            return 1;
        } else {
            return -1;
        }
    } else if (!(PINC & (1 << PC0)) && minule) {
        // sestupná hrana
        minule = 0;
        if (PINC & (1 << PC1)) {
            return -1;
        } else {
            return 1;
        }
    } else if (PINC & (1 << PC0) && minule == 1) {
        // nic se nezměnilo
        return 0;
    } else if (!(PINC & (1 << PC0)) && minule == 0) {
        // nic se nezměnilo
        return 0;
    }
    return 0;
}


static FILE mystdout = FDEV_SETUP_STREAM(myputc, NULL, _FDEV_SETUP_WRITE);

volatile int16_t kolecko = 0;
volatile int8_t kolecko_zmena = 0;
volatile char prijaty_znak = 0;
volatile char prijem = 0;

int main(void)
{
    stdout = &mystdout;

    /*lcd_init(LCD_ON_CURSOR);*/
    /*lcd_clrscr();*/
    /*lcd_home();*/
    /*lcd_puts("ahoj");*/
    sei();                       //gloválně povolí přerušení
    setupUART();
    setupTIMER();
    while (1) {
        /*myputs("ahoj\n"); */
        /*printf("ahoj %d\n", 314); */
        /*printf("nazdar\n"); */
        //printf_P(PSTR("ahoj -- %u\n"), i++); //formátovací řetězec je ve FLASH paměti

        /* 
           int8_t status;
           status = check_status();
           if (status == -1 ) {
           printf_P(PSTR("-- %c\n"), 'A'); //formátovací řetězec je ve FLASH paměti
           }
           if (status == 1 ) {
           printf_P(PSTR("-- %c\n"), 'B'); //formátovací řetězec je ve FLASH paměti
           }
           _delay_ms(1);
         */

        if (kolecko_zmena) {
            kolecko_zmena = 0;
            printf("%i\n", kolecko);
            PORTB ^= (1 << PB5);
        }

        if (prijem) {
            prijem = 0;
            printf("OK %c\n", prijaty_znak);
            PORTB ^= (1 << PB5);
        }
    }

    return 0;
}                               /* main */


ISR(TIMER0_COMPA_vect)
{
    // http://www.nongnu.org/avr-libc/user-manual/group__avr__interrupts.html
    int8_t status;
    status = check_status();
    if (status) {
        kolecko += status;
        kolecko_zmena = 1;
    }
}

ISR(USART_RX_vect)
{
    prijaty_znak = UDR0;        // Vyčíst přijatá data
    prijem = 1;       // dej hlavní smyčce info o příchodu nového znaku 
}
