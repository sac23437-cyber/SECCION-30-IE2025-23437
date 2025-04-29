/*
 * PRE-LAB 6.c
 *
 * Created: 19/04/2025 16:22:24
 * Author : Luis Sactic
 */ 


/************************/
// Encabezado (Libraries)
#include <avr/io.h>
#include <avr/interrupt.h>

// Parte 1 - Mostrar carácter
#define CHAR_TO_SEND 'A'									// Puede ser cualquier valor :D

// Global variables
uint8_t counter_10ms;
volatile uint8_t received_char = 0;
volatile uint8_t char_received_flag = 0;

/************************/
// Function prototypes
void setup();
void usart_init(void);
void usart_transmit(uint8_t data);
uint8_t usart_receive(void);

/************************/
// Main Function
int main(void){
    setup();
    
// Parte 1 - Enviamos el carácter una vez
    usart_transmit(CHAR_TO_SEND);
    
	
    while (1){
		
// Parte 2 - Verificamos si se recibió un carácter
        if(char_received_flag){
            PORTB = received_char;							// Mostramos el carácter recibido en PORTB (por medio de LED's)
            char_received_flag = 0;
        }
    }
}

/************************/
// NON-Interrupt subroutines
void setup(){
    cli();													// Deshabilitamos las interrupciones globales
    DDRB = 0xFF;											// Colocamos PORTB como salida
    PORTB = 0x00;											// Iniciamos el PORTB en 0
    
    usart_init();											// Iniciamos USART para la comunicación serial
	
    sei();													// Habilitamos las interrupciones globales
}

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

// Función para transmitir por USART
void usart_transmit(uint8_t data){
    while(!(UCSR0A & (1 << UDRE0)));					    // Esperamos a que el buffer de transmisión esté vacío
    
    UDR0 = data;											// Ponemos el dato en el buffer (envíamos)

// Función para recibir por USART 
uint8_t usart_receive(void){
    while(!(UCSR0A & (1 << RXC0)));							// Esperamos a que se reciban datos
    
    return UDR0;											// Obtenemos y retornamos el dato recibido del buffer
}

/************************/
// Interrupt routines

// Interrupción por Recepción Completa del USART
ISR(USART_RX_vect){
    received_char = UDR0;									// Leemos el carácter recibido
    char_received_flag = 1;									// Activamos la bandera para indicar nuevo carácter recibido
}