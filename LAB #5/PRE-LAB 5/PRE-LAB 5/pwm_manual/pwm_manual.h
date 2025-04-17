/*
 * pwm_manual.h
 *
 * Created: 7/04/2025 22:36:58
 *  Author: Luis Sactic
 */ 


#ifndef PWM_MANUAL_H
#define PWM_MANUAL_H

#include <avr/io.h>
#include <avr/interrupt.h>

// Inicializa el PWM manual en el pin especificado
void pwm_manual_init(uint8_t pin);

// Establece el valor de PWM (0-255)
void pwm_manual_set(uint8_t value);

// Nueva función para actualización suave
void pwm_manual_update(void);

#endif