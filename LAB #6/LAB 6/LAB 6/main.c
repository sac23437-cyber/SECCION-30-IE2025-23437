/*
 * LAB 6.c
 * 
 * Created: 22/04/2025 14:10:31
 * Author : Luis Sactic
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

// Definiciones
#define F_CPU 16000000UL
#define BAUD 9600
#define MYUBRR F_CPU/16/BAUD-1

// Variables globales
volatile uint8_t received_char = 0;
volatile uint8_t char_received_flag = 0;

/************************/
// Prototipos de función
void setup();
void usart_init(void);
void usart_transmit(uint8_t data);
uint8_t usart_receive(void);
void cadena(char txt[]);

/************************/
// Función principal
int main(void) {
    setup();
    cadena("Hector es el mejor del mundo :D\n\r");
    
    while (1) {
        if(char_received_flag) {
            usart_transmit(received_char);
            PORTB = received_char;
            char_received_flag = 0;
        }
    }
}

/************************/
// Configuración inicial
void setup() {
    cli();
    DDRB = 0xFF;
    PORTB = 0x00;
    usart_init();
    sei();
}

/************************/
// Funciones USART
// USART initialization
void usart_init(void){
	// Configuramos la velocidad de baudios (9600 a 16MHz)
	UBRR0H = 0;
	UBRR0L = 103;
	
	// Habilitamos receptor y transmisor así como interrupción por recepción
	UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);
	
	// Configuramos el formato - 8 bits de datos, 1 bit de parada, sin paridad
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}


void usart_transmit(uint8_t data) {
    while(!(UCSR0A & (1 << UDRE0)));
    UDR0 = data;
}

uint8_t usart_receive(void) {
    while(!(UCSR0A & (1 << RXC0)));
    return UDR0;
}

void cadena(char txt[]) {
    uint8_t i = 0;
    while(txt[i] != '\0') {
        usart_transmit(txt[i]);
        i++;
    }
}

/************************/
// Rutina de interrupción
ISR(USART_RX_vect) {
    received_char = UDR0;
    char_received_flag = 1;
}