/*
 * PRE-LAB 5.c
 *
 * Created: 7/04/2025 22:36:03
 * Author : Luis Sactic
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "pwm_manual/pwm_manual.h"

// Variables globales
volatile uint16_t adc_values[3]; // Almacena valores ADC [A5, A6, A7]
volatile uint8_t current_channel = 0;

void setup() {
	// 1. Configurar PWM manual para LED en PD7
	pwm_manual_init(PD7);
	
	// 2. Configurar pines de servos como salidas
	DDRB |= (1 << PB1) | (1 << PB2);
	
	// 3. Configurar Timer1 para PWM modo 14 (ICR1 como TOP) - Servos
	TCCR1A = (1 << COM1A1) | (1 << COM1B1) | (1 << WGM11);
	TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11); // Prescaler 8
	
	// Periodo PWM 20ms (50Hz) para servos
	ICR1 = 39999; // (16MHz/8)/50Hz - 1
	
	// Valores iniciales servos (1ms)
	OCR1A = 3000; // Servo1 en PB1
	OCR1B = 3000; // Servo2 en PB2
	
	// 4. Configurar ADC con autoscan de 3 canales (A5, A6, A7)
	ADMUX = (1 << REFS0); // AVcc como referencia
	ADCSRA = (1 << ADEN) | (1 << ADIE) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
	ADCSRB = 0; // Modo libre
	
	// Habilitar resistencias de pull-up en entradas ADC
	PORTC |= (1 << PC5);
	
	// Iniciar primera conversión ADC
	ADCSRA |= (1 << ADSC);
	
	// Habilitar interrupciones globales
	sei();
}

int main(void) {
	setup();
	
	while (1) {
		
		// Actualización suave del brillo del LED
		pwm_manual_update();
		
	}
}

// Interrupción ADC para lectura de potenciómetros
ISR(ADC_vect) {
	

	
	// Guardar valor del canal actual
	adc_values[current_channel] = ADC;
	
	// Cambiar al siguiente canal (0:A5, 1:A6, 2:A7)
	current_channel = (current_channel + 1) % 3;
	
	// Configurar próximo canal
	ADMUX = (1 << REFS0) | (current_channel == 0 ? (1 << MUX2) | (1 << MUX0) :
	current_channel == 1 ? (1 << MUX2) | (1 << MUX1) :
	(1 << MUX2) | (1 << MUX1) | (1 << MUX0));
	
	// Actualizar PWM del LED con valor de A5 (convertido de 10bit a 8bit)
	pwm_manual_set(adc_values[0] >> 2);
	
	// Actualizar servos según los valores leídos
	OCR1A = 2000 + ((uint32_t)adc_values[1] * 3250 / 1023); // A6 controla Servo1 (PB1)
	OCR1B = 2000 + ((uint32_t)adc_values[2] * 3250 / 1023); // A7 controla Servo2 (PB2)
	
	// Limitar valores para proteger servos
	if(OCR1A < 2000) OCR1A = 2000;  // Límite inferior (0°)
	if(OCR1A > 5250) OCR1A = 5250;  // Límite superior (180°)
	if(OCR1B < 2000) OCR1B = 2000;
	if(OCR1B > 5250) OCR1B = 5250;
	
	// Iniciar nueva conversión
	ADCSRA |= (1 << ADSC);
}
