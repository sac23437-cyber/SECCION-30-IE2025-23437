; PRE-LAB 2.asm
// Universidad del Valle de Guatemala
// IE2023 - Programación de Microcontroladores
// Contador de 4 bits
// Hardware: ATmega328P
// Created: 11/02/2025 09:29:41
// Author : Luis Fernando Sactic Ordoñez



// Encabezado
.include "M328PDEF.inc"
.cseg
.org	0x0000
RJMP INICIO								// Vamos a INICIO
.org	0x0020
RJMP TIMER0								// Vamos a la rutina de interrupción

INICIO:
	// Configuración de Entradas y Salidasa
	LDI		R16, 0xFF					// Colocamos como salida (PC0 - PC3)
	OUT		DDRC, R16						// Colocamos PORTC como salida

	LDI		R16, 0x00						// Se inicia el contador en 0
	OUT		PORTC, R16						// Mostramos el valor de R17 en PORTC
	// TIMER	
	LDI		R16, (1 << CS02) | (1 << CS00)	// Colocamos el prescale a 1024
	OUT		TCCR0B, R16						// Se escribe el valor para aactivar el TIMER

	LDI		R16, (1 << TOIE0)				// Colocamos la interrupción del Timer0
	STS		TIMSK0, R16						// Colocamos la interrupción en TIMSK0

	SEI										// Colocamos las interrupciones globales

LOOP:
	RJMP	LOOP

TIMER0:
	IN		R18, PORTC					// Leemos el valor de PORTC
	INC		R18							// Incrementamos el valor de R18
	OUT		PORTC, R18					// Se muestra el valor de R18 en PORTC
	RETI								// Regresamos a la interrupción
