// Universidad del Valle de Guatemala
// IE2023 - Programación de Microcontroladores
// Lab 3
// Autor: Luis Fernando Sactic Ordoñez
// Hardware: ATmega328P
// Creado: 18/02/2025 14:12:41

; Incluye el archivo de definiciones del ATmega328PB
.include "M328PDEF.inc"
.cseg

.org 0x0000																// Vector de reset
    RJMP SETUP															// Vamos al SETUP
.org OVF0addr															// Vector de interrupción
    RJMP TIMER															// Vamos al TIMER


TABLA7SEG:
    .DB 0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F		// Valores de 0-9


SETUP:	
	// Configuración del STACK
    LDI		R16, HIGH(RAMEND)  
    OUT		SPH, R16           
    LDI		R16, LOW(RAMEND)   
    OUT		SPL, R16				

    // Configuración del Display
    LDI		R16, 0b1111111												// Comolocamos como salida (PD0 - PD7)
    OUT		DDRD, R16													// Mostramos el valor de R16
    LDI		R16, 0b11111111						
    OUT		PORTD, R16													// Colocamos las salidas apagadas

	// Configuración del TIMER
	LDI		R16, (0 << WGM01) | (0 << WGM00)							// Colocamos el TIMER en modo normal
	OUT		TCCR0A, R16
	LDI		R16, (1 << CS02)|(1 << CS00)								// Colocamos un prescaler de 1024
	OUT		TCCR0B, R16
	LDI		R16, (1 << TOIE0)											// Habilitamos la interrupción para overflow
	STS		TIMSK0, R16

	// Configuración de interrupciones
	LDI		R16, (1 << PCINT1)|(1 << PCINT0)							// Configuramos las interrupciones
	STS		PCMSK0, R16													// Habilitamos PCINT en PCINT0 y PCINT1
	LDI		R16, (1 << PCIE0)
	STS		PCICR, R16													// Habilitamos el registro de control

    // Contadores
    LDI		R17, 0														// Colocamos el contador del TIMER en 0
    LDI		R18, 0														// Colocamos el contador de milisegundos en 0
    LDI		R19, 0														// Colocamos el Display en 0
	
	SEI																	// Habilita interrupciones globales.

LOOP:
    // Cargamos los valores de la TABLA7SEG
    LDI		ZH, HIGH(TABLA7SEG << 1)									 
    LDI		ZL, LOW(TABLA7SEG << 1)										 
    ADD		ZL, R19														// Obtenemos el valor de R19
    LPM		R16, Z														// Cargamos el valor de TABLA7SEG
    OUT		PORTD, R16													// Mostramos el valor

    RJMP	LOOP														

INTERR:
    POP		R16															// Recuperamos el valor de R16
    OUT		SREG, R16													// Colocamos el valor de R16
    POP		R16															// Recuperamos el valor de R16
    RETI																

TIMER:
    PUSH	R16															// Guardamos el valor de R16
    IN		R16, SREG													// Leemos el registro
    PUSH	R16															// Guardamos el valor de SREG

    INC		R18															// Incrementamos el contador de milisegundos
    CPI		R18, 50														// Verificamos si hay 50ms
    BRNE	SALIDA														// Si no hay 50ms vamos a SALIDA

    CLR		R18															// Reiniciamos contador de milisegundos
    INC		R19															// Incrementa valor

    CPI		R19, 10														// Vemos si R19 llego a 10
    BRNE	SALIDA														// Vamos a SALIDA si no es 10
    CLR		R19															// Reiniciamos el valor de R19

SALIDA:
    POP		R16															// Recuperamos el valor de R16
    OUT		SREG, R16													// Colocamos el valor de R16
    POP		R16															// Recuperamos el valor
    RETI																