/*
 * PRE-LAB 4.cpp
 *
 * Created: 30/03/2025 18:23:20
 * Author : Luis Sactic
 */ 


/************************/
// Encabezado (Libraries)
#include <avr/io.h>
#include <avr/interrupt.h>

uint8_t counter_10ms;
uint8_t counter_value = 0;
uint8_t boton1 = 0;
uint8_t boton2 = 0;
uint8_t delay_boton1 = 0;
uint8_t delay_boton2 = 0;

/************************/
// Function prototypes
void setup();
void update_leds();
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
	
	// Configuraramos las salidas 
	DDRB |= (1 << PB4) | (1 << PB5);																// Colocamos PB4 y PB5 como salidas (Bit's 0 y 1)
	DDRC |= (1 << PC0) | (1 << PC1) | (1 << PC2) | (1 << PC3) | (1 << PC4) | (1 << PC5);			// Colocamos PC0-PC5 como salidas (Bit's 2 - )
	
	
	PORTB &= ~((1 << PB2) | (1 << PB3));															// Desactivamos los Pull-Up's internos (botones en configuración Pull-Down externo)
	
	
	// Configuramos el Timer0 para antirebote 
	TCCR0A = 0x00;
	TCCR0B = (1 << CS02) | (1 << CS00);																// Colocamos el Prescaler de 1024
	TCNT0 = 100; // 
	TIMSK0 = (1 << TOIE0);
	
	counter_10ms = 0;
	
	
	update_leds();																					// Mostrar valor inicial del contador
	
	sei();
}

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

/************************/
// Interrupt routines
ISR(TIMER0_OVF_vect){
	TCNT0 = 100;
	counter_10ms++;
	
	// Delay (antirrebote)
	if(counter_10ms == 1){																			// Revisamos cada 50ms 
		counter_10ms = 0;
		
		// Botón para incremento (PB3)
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
		
		// Botón para decremento (PB2)
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
			boton1 = 0; // 
		}
		else if(boton2){
			counter_value--;
			update_leds();
			boton2 = 0; //
		}
	}
}