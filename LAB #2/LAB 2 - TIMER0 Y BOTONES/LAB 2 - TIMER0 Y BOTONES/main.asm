//***************************************************************************
.include "M328PDEF.inc"
.cseg
.org 0x00
RJMP CONTADOR								// Vamos a CONTADOR
RJMP TIMER0									// Vamos a la rutina de interrupción
//***************************************************************************
// Stack
LDI R16, LOW(RAMEND)
OUT SPL, R16
LDI R17, HIGH(RAMEND)
OUT SPH, R17
//***************************************************************************
// CONFIGURACION
//***************************************************************************

TABLA7SEG:
	.DB   0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07  // Valores de 0-7  
    .DB   0x7F, 0x6F, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71  // Valores de 8-F 


CONTADOR:
	LDI R16, (1 << CLKPCE)
	STS CLKPR, R16				//	prescaler
	LDI R16, 0b0000_1000		// 1000
	STS CLKPR, R16				//	definido a 8 fcpu = 2MHz

// CONTADOR AUTÓNOMO--------------------------------------------------------------------------------------
	// Configuración de Entradas y Salidas para el contador binario
	LDI		R16, 0xFF						// Colocamos como salida (PC0 - PC3)
	OUT		DDRC, R16						// Colocamos PORTC como salida

	LDI		R16, 0x00						// Se inicia el contador en 0
	OUT		PORTC, R16						// Mostramos el valor de R16 en PORTC

	// TIMER	
	LDI		R16, (1 << CS02) | (1 << CS00)	// Colocamos el prescale a 1024
	OUT		TCCR0B, R16						// Se escribe el valor para aactivar el TIMER

	LDI		R16, (1 << TOIE0)				// Colocamos la interrupción del Timer0
	STS		TIMSK0, R16						// Colocamos la interrupción en TIMSK0

	SEI										// Colocamos las interrupciones globales
// CONTADOR AUTÓNOMO--------------------------------------------------------------------------------------
	
SETUP:
	// Configuración de los botones para el display (pull-up)
	CBI		DDRB, PB4						// PB4 como entrada
	CBI		DDRB, PB5						// PB5 como entrada
	SBI		PORTB, PB4						// Activamos el pull-up
	SBI		PORTB, PB5						// Activamos el pull-up

	// Configuración de salidas para el display
	LDI		R17, 0b11111111					// Colocamos como salidas PD0 - PD7
	OUT		DDRD, R17						// Mostramos el valor de R17

	RCALL	ACTDISPLAY7						// Llamos a la rutina para el display


LOOP:
	RCALL	BOTONES	
	RJMP	LOOP


ACTDISPLAY7:
	MOV		ZL, R18
	LDI		ZH, HIGH(TABLA7SEG << 1)
	LDI		ZL, LOW(TABLA7SEG << 1)
	ADD		ZL, R18
	LPM		R17, Z
	OUT		PORTD, R17
	RET

BOTONES:
	SBIC	PINB, PB5
	RJMP	DECREMENTAR
	RCALL	DELAY
	SBIC	PINB, PB5
	RJMP	DECREMENTAR
	
	INC		R18
	ANDI	R18, 0x0F
	RCALL	ACTDISPLAY7

DECREMENTAR:
	SBIC	PINB, PB4
	RET
	RCALL	DELAY
	SBIC	PINB, PB4
	RET

	DEC		R18
	ANDI	R18, 0x0F
	RCALL	ACTDISPLAY7

ESPERARINCREMENTAR:
	SBIS	PINB, PB5
	RJMP	ESPERARINCREMENTAR

ESPERARDECREMENTAR:
	SBIS	PINB, PB4
	RJMP	ESPERARDECREMENTAR

DELAY:
	LDI		R29, 100
	DELAY2:
	DEC		R29
	BRNE	DELAY2
	RET

TIMER0:
	IN		R30, PORTC					// Leemos el valor de PORTC
	INC		R30							// Incrementamos el valor de R18
	OUT		PORTC, R30					// Se muestra el valor de R18 en PORTC
	RETI								// Regresamos a la interrupción
