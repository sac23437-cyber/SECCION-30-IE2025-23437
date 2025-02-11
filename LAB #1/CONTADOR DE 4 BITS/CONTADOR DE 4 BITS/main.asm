; CONTADOR DE 4 BITS CON ANTIREBOTE.asm

// Universidad del Valle de Guatemala
// Programación de Microcontroladores
// Autor: Luis Sactic
// Creado: 3/02/2025 21:35:39

// Encabezado
.include "M328PDEF.inc"
.cseg
.org	0x0000

// Configuración de pila // 0x00FF
LDI		R16, LOW(RAMEND)		// cargar 0xFF a R16
OUT		SPL, R16				// cargar 0xFF a SPL
LDI		R17, HIGH(RAMEND)		
OUT		SPH, R17				// cargar 0x08 a SPH

// Colocamos el reloj a 1MHz
LDI		R24, (1 << CLKPCE)
STS		CLKPR, R24
LDI		R24, 0b00000100
STS		CLKPR, R24

// Configuración de Entradas y Salidas

// PRIMER CONTADOR
LDI		R16, 0b00001111			// Se configuran los 4 bits (PB0 - PB3) como salida
OUT		DDRB, R16				// Si DDRB = 1 es salida, si DDRB = 0 es entrada


// SEGUNDO CONTADOR
LDI		R17, 0b00001111			// Se configuran los 4 bits (PC0 - PC3) como salida para el contador
OUT		DDRC, R17				// Si DDRC = 1 es salida, si DDRC = 0 es entrada


// SUMADOR
LDI		R18, 0b01111111			// Se configuran los 4 bits (PD0 - PD5) como salida para el sumador
OUT		DDRD, R18				// Si DDRD = 1 es salida, si DDRD = 0 es entrada



LDI		R19, 0					// Se inicia el contador #1 en 0 (PB2 - PB5)
LDI		R20, 0					// Se inicia el contador #2 en 0 (PC2 - PC5) 
LDI		R21, 0					// Se inicia el sumador en 0 (PD2 - PD5)

LOOP:
	OUT		PORTB, R19			// Mostramos en los LED's el valor que está actualmente en R19
	OUT		PORTC, R20			// Mostramos en los LED's el valor que está actualmente en R20
	OUT		PORTD, R21			// Mostramos en los LED's el valor que está actualmente en R21


	// Primer Contador (PB2 - PB5)			
	SBIC	PINB, PB5			// Verifica si fue presionado el pulsador para incrementar
	CALL	INCREMENTAR1		// Si se oprimió se incrementa
	SBIC	PINB, PB4			// Verifica si fue presionado el pulsador para decrementar
	CALL	DECREMENTAR1		// Si se oprimió se decrementa

	// Segundo Contador (PC2 - PC5)
	SBIC	PINC, PC4			// Verifica si fue presionado el pulsador para incrementar
	CALL	INCREMENTAR2		// Si se oprimió se incrementa
	SBIC	PINC, PC5			// Verifica si fue presionado el pulsador para decrementar
	CALL	DECREMENTAR2		// Si se oprimió se decrementa

	// Sumador
	SBIC	PIND, PD7			// Verifica si fue presionado el pulsador de suma
	CALL	SUMA				// Si se oprimió se suman

	RJMP	LOOP


// SUBRUTINAS (DE INTERRUPCIÓN)
INCREMENTAR1:
	CALL	DELAY				// Llamamos al antirrebote
	INC		R19					// Incrementamos el valor de R19
	OUT		PORTB, R19			// Actualizamos el PORTB
	RJMP	LOOP

DECREMENTAR1:
	CALL	DELAY				// Llamamos al antirrebote
	DEC		R19					// Decrementamos el valor de R19
	OUT		PORTB, R19			// Actualizamos el PORTB
	RJMP	LOOP


INCREMENTAR2:
	CALL	DELAY				// Llamamos al antirrebote
	INC		R20					// Incrementamos el valor de R20
	OUT		PORTC, R20			// Actualizamos el PORTC
	RJMP	LOOP

DECREMENTAR2:
	CALL	DELAY				// Llamamos al antirrebote
	DEC		R20					// Decrementamos el valor de R20
	OUT		PORTC, R20			// Actualizamos el PORTC
	RJMP	LOOP

SUMA:
	CALL    DELAY				// Llamamos al antirrebote
    MOV     R21, R19			// Copiamos R20 a R21
    ADD     R21, R20			// Sumamos R21 + R20
	LSL		R21
	LSL		R21
    OUT     PORTD, R21			// Mostrar la suma en los LEDs (PD2 - PD5)
    RJMP    LOOP


// SUBRUTINAS (NO DE INTERRUPCIÓN) "El DEC hace que no sea necesario usar CPI (Comparación)"
DELAY:
	LDI		R22, 255			// Ciclo externo (largo)
SUBDELAY1:
	LDI		R23, 255			// Ciclo interno (corto)
SUBDELAY2:
	DEC		R23				
	BRNE	SUBDELAY2			// Esperamos hasta que R22 sea 0
	DEC		R22				
	BRNE	SUBDELAY1			// Esperamos hasta que R23 sea 0
	RET