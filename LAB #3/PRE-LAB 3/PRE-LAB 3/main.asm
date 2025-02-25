// Universidad del Valle de Guatemala
// IE2023 - Programación de Microcontroladores
// Interrupciones
// Autor: Luis Fernando Sactic Ordoñez
// Hardware: ATmega328P
// Creado: 18/02/2025 07:35:39

.include "M328PDEF.inc"
.cseg
.org 0x0000
JMP		STACK										// Iniciamos el programa en STAK
.org 0x0006		
JMP		INTERR										// Interrupción para el cambio de pin

STACK: // Configuración de pila
	LDI R16, LOW(RAMEND)
	OUT SPL, R16
	LDI R17, HIGH(RAMEND)
	OUT SPH, R17

// Colocamos el reloj a 1MHz
LDI		R24, (1 << CLKPCE)
STS		CLKPR, R24
LDI		R24, 0b00000100
STS		CLKPR, R24


SETUP:
// Configuración de entradas y salidas
	SBI		DDRB, PB2								// Se coloca el PB2 como salida
	CBI		PORTB, PB2								// Se apaga el PB2

	// Configuración de LED'S
	LDI		R16, 0b00001111							// Colocamos como salidas PC0 - PC3
	OUT		DDRC, R16								// Mostramos el valor de R16
	
	// Configuración de botones
	SBI		PORTB, PB0								// Colocamos el PB0 con pull-up
	CBI		DDRB, PB0								// Colocamos el PB0 como entrada
	SBI		PORTB, PB1								// Colocamos el PB0 con pull-up
	CBI		DDRB, PB1								// Colocamos el PB1 como entrada

	// Configuración de interrupciones
	LDI		R16, (1 << PCINT1)|(1 << PCINT0)		// Configuramos las interrupciones
	STS		PCMSK0, R16								// Habilitamos PCINT en PCINT0 y PCINT1
	LDI		R16, (1 << PCIE0)
	STS		PCICR, R16								// Habilitamos el registro de control
	SEI												// Habilitamos las interrupciones globales

	// Configuración del contador
	LDI		R20, 0									// Colocamos el contador en 0
	LDI		R21, 0									// Colocamos R21 en 0

LOOP:
	MOV		R21, R20								// Copiamos el valor de R20 a R21
	OUT		PORTC, R21								// Mostramos el valor en el PORTC (LED's)
	JMP		LOOP

INTERR:
	PUSH	R16										// Guardamos el valor de R16		
	IN		R16, SREG								// Leemos el registro
	PUSH	R16										// Guardamos el valor de SREG 

	IN		R18, PINB								// Leemos el valor de los botones (PB0 Y PB1)
	
	SBRC	R18, PB0								// Comprobamos si está presionado
	JMP		BOTON1									// Si está presionado vamos a BOTON1
	INC		R20										// Incrementamos R20 (contador)
	CPI		R20, 16									// Comparamos R20 con R16
	BRNE	FIN										// Si son iguales vamos a FIN
	LDI		R20, 0									// Colocamos R20 en 0
	JMP		FIN										// Vamos a FIN

BOTON1:
	SBRC	R18, PB1								// Comprobamos si está presionado 
	JMP		FIN										// Si está presionado vamos a FIN
	DEC		R20										// Decrementamos R20
	BRNE	FIN										// Comprobamos que R20 sea 0
	LDI		R20, 15									// Si R20 es 0 colocamos R20 en 15

FIN:
	SBI		PINB, PB2								// Cambiamos el estado de PB2
	SBI		PCIFR, PCIF0							// Borramos la interrupción

	POP		R16										// Recuperamos el valor de R16
	OUT		SREG, R16								// Colocamos el valor de R16
	POP		R16										// Recuperamos el valor de R16
	RETI											