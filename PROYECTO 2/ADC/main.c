#define F_CPU 16000000UL  // Debe ser lo primero en el archivo
#define FILTER_SAMPLES 3

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

// Variables globales
volatile uint16_t adc_values[4];
volatile uint8_t current_channel = 0;

void setup() {
	
	//--------------------------------------------------------------------------------
	ADCSRA = (1 << ADEN) | (1 << ADIE) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Prescaler 128 (más lento pero estable)
	DIDR0 = 0x0F; // Deshabilitar entradas digitales en A0-A3 (si no se usan)
	//--------------------------------------------------------------------------------
	
	// 1. Configurar todos los pines PWM como salidas
	DDRB |= (1 << PB1) | (1 << PB2) | (1 << PB3); // OC1A, OC1B, OC2A
	DDRD |= (1 << PD3) | (1 << PD5) | (1 << PD6);  // OC2B, OC0B, OC0A

	// 2. Timer1 (16-bit) para servos de precisión (PB1 y PB2)
	TCCR1A = (1 << COM1A1) | (1 << COM1B1) | (1 << WGM11);
	TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11);
	ICR1 = 39999; // 20ms period
	OCR1A = 3000; // 1.5ms inicial PB1
	OCR1B = 3000; // 1.5ms inicial PB2

	// Configuración mejorada para Timer0 (PD5 - OC0B)
	TCCR0A = (1 << COM0B1) | (1 << WGM01) | (1 << WGM00);  // Fast PWM
	TCCR0B = (1 << CS02) | (1 << CS00);  // Prescaler 1024 (mayor precisión)
	OCR0B = 38;  // 1.5ms inicial

	// Configuración mejorada para Timer2 (PD3 - OC2B)
	TCCR2A = (1 << COM2B1) | (1 << WGM21) | (1 << WGM20);  // Fast PWM
	TCCR2B = (1 << CS22) | (1 << CS21) | (1 << CS20);  // Prescaler 1024
	OCR2B = 38;  // 1.5ms inicial

	// 5. Configurar ADC
	ADMUX = (1 << REFS0);
	ADCSRA = (1 << ADEN) | (1 << ADIE) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
	ADCSRB = 0;
	PORTC |= 0xF0; // Pull-ups en A4-A7
	ADCSRA |= (1 << ADSC);
	
	sei();
}

uint16_t filtered_adc(uint8_t channel) {
	static uint16_t samples[4][FILTER_SAMPLES];
	static uint8_t index = 0;
	samples[channel][index] = ADC;
	index = (index + 1) % FILTER_SAMPLES;
	uint32_t sum = 0;
	for(uint8_t i=0; i<FILTER_SAMPLES; i++) sum += samples[channel][i];
	return sum / FILTER_SAMPLES;
}

ISR(ADC_vect) {
	adc_values[current_channel] = filtered_adc(current_channel);
	
	current_channel = (current_channel + 1) % 4;
	
	ADMUX = (1 << REFS0) | (current_channel + 4); // Versión optimizada para A4-A7


	// Asignación directa con protección
	OCR1A = 2000 + (adc_values[3] * 3250UL / 1023); // A7 ? PB1
	OCR1B = 2000 + (adc_values[2] * 3250UL / 1023); // A6 ? PB2
	// Mapeo perfecto para rango completo (0-180°)
	OCR0B = 20 + (adc_values[1] * 35 / 1023);  // A5 ? PD5 (0.8ms-2.2ms)
	OCR2B = 20 + (adc_values[0] * 35 / 1023);  // A4 ? PD3 (0.8ms-2.2ms)

	// Límites estrictos
	if(OCR0B < 20) OCR0B = 20;   // Mínimo 0.8ms (~0°)
	if(OCR0B > 55) OCR0B = 55;   // Máximo 2.2ms (~180°)
	if(OCR2B < 20) OCR2B = 20;
	if(OCR2B > 55) OCR2B = 55;

	ADCSRA |= (1 << ADSC);
}


int main(void) {
	setup();
	
	while(1) {
		
	}
}