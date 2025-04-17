/*
 * pwm_manual.c
 *
 * Created: 7/04/2025 22:37:07
 *  Author: Luis Sactic
 */ 

#include "pwm_manual.h"

#define SMOOTHING_FACTOR 0.1f  // Factor de suavizado
#define UPDATE_INTERVAL 10     // Intervalo de actualización en ms

static volatile uint8_t pwm_pin_mask;
static volatile uint8_t target_brightness = 0;
static float current_brightness = 0;
static volatile uint16_t last_update_time = 0;

void pwm_manual_init(uint8_t pin) {
	// Configurar el pin como salida
	DDRD |= (1 << pin);
	pwm_pin_mask = (1 << pin);
	
	// Configurar Timer0 para PWM manual
	TCCR0A = 0x00; // Modo normal
	TCCR0B = (1 << CS01); // Prescaler 8
	TIMSK0 = (1 << TOIE0); // Habilitar interrupción
	
	// Inicializar valores
	current_brightness = 0;
	target_brightness = 0;
	last_update_time = 0;
}

void pwm_manual_set(uint8_t value) {
	target_brightness = value;
}

void pwm_manual_update(void) {
	static uint16_t timer_counter = 0;
	
	// Actualizar cada UPDATE_INTERVAL ms usando un contador de tiempo
	if (TIFR0 & (1 << TOV0)) {  // Si ocurrió overflow del Timer0
		TIFR0 |= (1 << TOV0);   // Limpiar flag
		timer_counter++;
		
		if (timer_counter >= UPDATE_INTERVAL) {
			// Filtro de suavizado
			current_brightness += (target_brightness - current_brightness) * SMOOTHING_FACTOR;
			timer_counter = 0;
		}
	}
}

ISR(TIMER0_OVF_vect) {
	static uint8_t pwm_counter = 0;
	uint8_t threshold = (uint8_t)current_brightness;
	
	// Manejo de valores bajos
	if (threshold < 2) {
		PORTD &= ~pwm_pin_mask;
		return;
	}
	
	// PWM normal
	if (pwm_counter < threshold) {
		PORTD |= pwm_pin_mask;
		} else {
		PORTD &= ~pwm_pin_mask;
	}
	
	pwm_counter++;
}