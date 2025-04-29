/*
 * POST-LAB 6.c
 *
 * Created: 24/04/2025 22:33:32
 * Author : Luis Sactic
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

// Definiciones
#define F_CPU 16000000UL
#define BAUD 9600
#define MYUBRR F_CPU/16/BAUD-1
#define POT_CHANNEL 6  // Canal ADC6 para el potenciómetro

// Variables globales
volatile uint8_t received_char = 0;
volatile uint8_t char_received_flag = 0;
volatile uint8_t menu_option = 0;

/************************/
// Prototipos de función
void setup();
void usart_init(void);
void usart_transmit(uint8_t data);
uint8_t usart_receive(void);
void cadena(char txt[]);
void adc_init(void);
uint16_t adc_read(uint8_t channel);
void show_menu(void);
void process_menu_option(void);
void read_potentiometer(void);

/************************/
// Función principal
int main(void) {
    setup();
    
    while (1) {
        show_menu();
        process_menu_option();
    }
}

/************************/
// Configuración inicial
void setup() {
    cli();
    DDRB = 0xFF;        // Puerto B como salida para LEDs
    PORTB = 0x00;       // LEDs inicialmente apagados
    usart_init();       // Inicializar USART
    adc_init();         // Inicializar ADC
    sei();              // Habilitar interrupciones globales
}

/************************/
// Funciones USART
void usart_init(void){
    // Configurar velocidad de baudios (9600 a 16MHz)
    UBRR0H = 0;
    UBRR0L = 103;
    
    // Habilitar receptor y transmisor + interrupción por recepción
    UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);
    
    // Configurar formato - 8 bits de datos, 1 bit de parada, sin paridad
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
// Funciones ADC
void adc_init(void) {
    // Referencia AVcc, ajuste a la izquierda
    ADMUX = (1 << REFS0);
    // Habilitar ADC y preescaler de 128 (16MHz/128 = 125kHz)
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

uint16_t adc_read(uint8_t channel) {
    // Seleccionar canal (0-7) y mantener bits de referencia
    ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);
    // Iniciar conversión
    ADCSRA |= (1 << ADSC);
    // Esperar hasta que la conversión se complete
    while (ADCSRA & (1 << ADSC));
    // Retornar resultado de 10 bits (ajuste a izquierda)
    return ADC;
}

/************************/
// Funciones de menú
void show_menu(void) {
    cadena("\n\r===== MENU PRINCIPAL =====\n\r");
    cadena("1. Leer Potenciometro (ADC6)\n\r");
    cadena("2. Enviar caracter ASCII\n\r");
    cadena("Seleccione una opcion (1-2): ");
}

void process_menu_option(void) {
    char_received_flag = 0;
    while(!char_received_flag);  // Esperar selección de usuario
    
    switch(received_char) {
        case '1':
            read_potentiometer();
            break;
        case '2':
            cadena("\n\rIngrese un caracter ASCII: ");
            char_received_flag = 0;
            while(!char_received_flag);  // Esperar caracter
            PORTB = received_char;       // Mostrar en LEDs
            cadena("\n\rCaracter recibido: ");
            usart_transmit(received_char);
            cadena("\n\rValor mostrado en LEDs: ");
            // Mostrar valor en decimal
            char buffer[10];
            sprintf(buffer, "%d", received_char);
            cadena(buffer);
            break;
        default:
            cadena("\n\rOpcion invalida!");
            break;
    }
}

void read_potentiometer(void) {
    cadena("\n\rLeyendo potenciometro en ADC6...\n\r");
    cadena("Gire el potenciometro y presione cualquier tecla para detener\n\r");
    
    while(1) {
        uint16_t adc_value = adc_read(POT_CHANNEL);
        uint8_t led_value = adc_value >> 2;  // Convertir 10 bits a 8 bits
        
        PORTB = led_value;  // Mostrar en LEDs
        
        // Mostrar valor en terminal
        char buffer[20];
        sprintf(buffer, "ADC: %4d  LEDs: 0x%02X\r", adc_value, led_value);
        cadena(buffer);
        
        // Verificar si se presionó una tecla para salir
        if(char_received_flag) {
            char_received_flag = 0;
            break;
        }
        _delay_ms(100);
    }
}

/************************/
// Rutina de interrupción
ISR(USART_RX_vect) {
    received_char = UDR0;
    char_received_flag = 1;
}