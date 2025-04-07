/*
 * POST-LAB 4.cpp
 *
 * Created: 3/04/2025 22:37:19
 * Author : Luis Sactic
 */ 


/************************/
// Encabezado (Libraries)
#include <avr/io.h>
#include <avr/interrupt.h>

// Variables para contador binario
uint8_t counter_10ms;
uint8_t counter_value = 0;
uint8_t boton1 = 0;
uint8_t boton2 = 0;
uint8_t delay_boton1 = 0;
uint8_t delay_boton2 = 0;

// Variables para display 7 segmentos
uint8_t display_digit = 0;
volatile uint16_t adc_value = 0;
uint8_t hex_digits[2] = {0,0};

// Tabla de valores para el display (cátodo común)
const uint8_t seg7_table[16] = {
    0x3F, // 0
    0x06, // 1
    0x5B, // 2
    0x4F, // 3
    0x66, // 4
    0x6D, // 5
    0x7D, // 6
    0x07, // 7
    0x7F, // 8
    0x6F, // 9
    0x77, // A
    0x7C, // B
    0x39, // C
    0x5E, // D
    0x79, // E
    0x71  // F
};

/************************/
// Function prototypes
void setup();
void update_leds();
void update_displays();
void read_adc();
void check_adc_compare();
/************************/
// Main Function
int main(void){
    setup();
    
    while (1){
       
    }
}

/************************/
// NON-Interrupt subroutines
void setup(){
	cli();
	
	hex_digits[0] = 0;
	hex_digits[1] = 0;
	counter_value = 0;
	adc_value = 0;
	display_digit = 0;
	
	// Configuramos el contador de 8 bits
	DDRB |= (1 << PB4) | (1 << PB5);  // PB4 y PB5 como salidas
	DDRC |= (1 << PC0) | (1 << PC1) | (1 << PC2) | (1 << PC3) | (1 << PC4) | (1 << PC5); // PC0-PC5 como salidas
	
	// Configuramos el displays
	DDRD = 0xFF; // PD0-PD7 como salidas
	DDRB |= (1 << PB0) | (1 << PB1); // PB0 y PB1 como salidas de control para el displays
	
	// Configuramos los botones
	PORTB &= ~((1 << PB2) | (1 << PB3));
	
	// Configuramos el Timer0
	TCCR0A = 0x00;
	TCCR0B = (1 << CS02) | (1 << CS00); // Colocamos un Prescaler 1024
	TCNT0 = 100;
	TIMSK0 = (1 << TOIE0);
	
	// Configuramos el ADC
	ADMUX = (1 << REFS0) | (1 << MUX2) | (1 << MUX1); // AVcc ref, ADC6 (A6)
	ADCSRA = (1 << ADEN) | (1 << ADIE) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // ADC enable, interrupt enable, prescaler 128
	
	// Inicializamos el contador y los display's
	counter_10ms = 0;
	display_digit = 0;
	
	// Mostramos los valores iniciales
	update_leds();
	update_displays();
	
	// Iniciamos la conversión ADC
	ADCSRA |= (1 << ADSC);
	
	sei();
}

void check_adc_compare() {
    // Comparamos el valor del ADC (10 bits) con el contador de 8 bits
    // Como el ADC es de 10 bits, lo convertimos a 8 bits para comparar
    uint8_t adc_8bit = adc_value >> 2; // Convertimos de 10 bits a 8 bits
    
    if(adc_8bit > counter_value) {
        PORTD |= (1 << PD7); // Encendemos el LED en PD7
    } else {
        PORTD &= ~(1 << PD7); // Apagamos el LED en PD7
    }
}

// CONTADOR DE 8 BITS------------------------------------------>
void update_leds(){
	// Actualizamos PB4 (bit 0)
	if(counter_value & 0x01)
	PORTB |= (1 << PB4);
	else
	PORTB &= ~(1 << PB4);
	
	// Actualizamos PB5 (bit 1)
	if(counter_value & 0x02)
	PORTB |= (1 << PB5);
	else
	PORTB &= ~(1 << PB5);
	
	// Actualizamos PC0 (bit 2)
	if(counter_value & 0x04)
	PORTC |= (1 << PC0);
	else
	PORTC &= ~(1 << PC0);
	
	// Actualizamos PC1 (bit 3)
	if(counter_value & 0x08)
	PORTC |= (1 << PC1);
	else
	PORTC &= ~(1 << PC1);
	
	// Actualizamos PC2 (bit 4)
	if(counter_value & 0x10)
	PORTC |= (1 << PC2);
	else
	PORTC &= ~(1 << PC2);
	
	// Actualizamos PC3 (bit 5)
	if(counter_value & 0x20)
	PORTC |= (1 << PC3);
	else
	PORTC &= ~(1 << PC3);
	
	// Actualizamos PC4 (bit 6)
	if(counter_value & 0x40)
	PORTC |= (1 << PC4);
	else
	PORTC &= ~(1 << PC4);
	
	// Actualizamos PC5 (bit 7)
	if(counter_value & 0x80)
	PORTC |= (1 << PC5);
	else
	PORTC &= ~(1 << PC5);
}
// CONTADOR DE 8 BITS------------------------------------------>

// DISPLAY'S--------------------------------------------------->
void update_displays(){
	// Apagamos ambos display's
	PORTB &= ~((1 << PB0) | (1 << PB1));
	
	if(display_digit == 0){
		// Mostramos el dígito menos significativo
		PORTD = seg7_table[hex_digits[0]];
		PORTB |= (1 << PB0);
	}
	else{
		// Mostramos el dígito más significativo
		PORTD = seg7_table[hex_digits[1]];
		PORTB |= (1 << PB1);
	}
	
	display_digit = !display_digit;
}
// DISPLAY'S--------------------------------------------------->


void read_adc(){
	// Convertimos el valor ADC a 2 dígitos (HEX)
	uint8_t hex_value = adc_value >> 2;
	hex_digits[0] = hex_value & 0x0F;						// Dígito menos significativo
	hex_digits[1] = (hex_value >> 4) & 0x0F;				// Dígito más significativo
}


/************************/
// Interrupt routines
ISR(TIMER0_OVF_vect){
	TCNT0 = 100;
	counter_10ms++;
	
	update_displays();										// Multiplexación de displays
	
	// Antirebote para contador (delay)
	if(counter_10ms >= 1){
		counter_10ms = 0;
		
		// Botón de incremento (PB3)
		if((PINB & (1 << PB3)) && (delay_boton1 < 3)){
			delay_boton1++;
			if(delay_boton1 == 3){
				boton1 = 1;
			}
		}
		else if(!(PINB & (1 << PB3)) && (delay_boton1 > 0)){
			delay_boton1--;
			if(delay_boton1 == 0){
				boton1 = 0;
			}
		}
		
		// Botón de decremento (PB2)
		if((PINB & (1 << PB2)) && (delay_boton2 < 3)){
			delay_boton2++;
			if(delay_boton2 == 3){
				boton2 = 1;
			}
		}
		else if(!(PINB & (1 << PB2)) && (delay_boton2 > 0)){
			delay_boton2--;
			if(delay_boton2 == 0){
				boton2 = 0;
			}
		}
		
		if(boton1){
			counter_value++;
			update_leds();
			boton1 = 0;
		}
		else if(boton2){
			counter_value--;
			update_leds();
			boton2 = 0;
		}
	}
}


ISR(ADC_vect){
	// Leemos el valor del ADC
	adc_value = ADC;
	
	// Convertimos los dígitos hexadecimales
	read_adc();
	
	uint8_t adc_8bit = adc_value >> 2;
	if (adc_8bit > counter_value) {
		PORTD |= (1 << PD7);   // Enciende LED si ADC > contador
		} else {
		PORTD &= ~(1 << PD7);  // Apaga LED si ADC <= contador
	}
	
	// Iniciamos una nueva conversión
	ADCSRA |= (1 << ADSC);
}
