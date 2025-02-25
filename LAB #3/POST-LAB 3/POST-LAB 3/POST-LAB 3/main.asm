// Universidad del Valle de Guatemala
// IE2023 - Programación de Microcontroladores
// Post-lab 3
// Autor: Luis Fernando Sactic Ordoñez
// Hardware: ATmega328P
// Creado: 20/02/2025 08:12:41

.include "M328PDEF.inc"
.cseg
.org 0x00
	JMP SETUP								// Vector de reset

.org 0x0020									// Vector de interrupción
	JMP TIMER0

TABLA7SEG: 
	.DB 0x3F, 0x06, 0x5B, 0x4F, 0X66, 0X6D, 0X7D, 0X07, 0X7F, 0X6F	// Valores de 0-9

SETUP:
	// Configuración del STACK
	LDI		R16, LOW(RAMEND)
	OUT		SPL, R16
	LDI		R17, HIGH(RAMEND)
	OUT		SPH, R17

	// Configuración de entradas y salidas
	LDI		R16, 0b000011000				// Colocamos PB3 y PB4 como salidas
	OUT		DDRB, R16						// Mostramos el valor de R16
	LDI		R16, 0b01111111					// Comolocamos como salida (PD0 - PD6)
	OUT		DDRD, R16						// Mostramos el valor de R16
	LDI		R16, 0b10000011					// Colocamos PB3 y PB4 como control
	OUT		DDRB, R16						// Mostramos el valor de R16
	CBI		PORTB, PB3						// Colocamos PB3 como bajo
	CBI		PORTB, PB4						// Colocamos PB4 como bajo

	LDI		R17, 0							// Iniciamos R17 en 0 para la tabla de 7 segmentos
	LDI		R18, 0							// Iniciamos R18 en 0 para el contador auxiliar
	LDI		R19, 0							// Iniciamos R19 en 0 para el display de unidades de segundos
	LDI		R20, 0							// Iniciamos R20 en 0 para el display de decenas de segundos
	
	LDI		R21, 9							// Colocamos el límite para las unidades de segundos
	LDI		R22, 5							// Colocamos el límite para las decenas de segundos
	LDI		ZH, HIGH(TABLA7SEG << 1)
	LDI		ZL, LOW(TABLA7SEG << 1)
	ADD		ZL, R17
	LPM		R17, Z							// Cargamos el valor en R17
	CALL	INICIO							// Llamamos a la función INICIO
	LDI		R23, 0							// Iniciamos R23 en 0 para el contador de tiempo

	SEI										// Habilitamos las interrupciones globales

LOOP:
	CALL	SEGUNDOS						// Llamamos a la función SEGUNDOS
	CALL	DECENAS							// Llamamos a la función DECENAS
	CPI		R23, 100						// Verificamos si R23 es 100
	BRNE	LOOP							// Si no es 100 vamos al LOOP
	CLR		R23								// Limpiamos R23
	CPSE	R19, R21						// Comparamos R19 con R21
	CALL	AUMENTARSEG						// Llamamos a la función AUMENTARSEG
	LDI		R19, 0							// Colocamos R19 en 0
	CPSE	R20, R22						// Comparamos R20 con R22
	CALL	AUMENTARDEC						// Llamamos a la función AUMENTARDEC
	LDI		R20, 0							// Colocamos R20 en 0
	RJMP	LOOP							//

INICIO:
	LDI		R16, (1 << CS02)|(1 << CS00)	// Prescaler en 1024
	OUT		TCCR0B, R16						 
	LDI		R16, 100						// Colocamos el valor de desbordamiento inicial
	OUT		TCNT0, R16						// Colocamos el valor del temporizador
	LDI		R16, (1 << TOIE0)				// Habilitamos la interrupción de desbordamiento
	STS		TIMSK0, R16						// Habilidamos la interrupción de R16
	RET

TIMER0:
	PUSH	R16								// Guardamos R16
	IN		R16, SREG						// Guardamos el estado de R16
	PUSH	R16								// Guardamos SREG

	LDI		R16, 100						// Cargamos un nuevo valor de desbordamiento
	OUT		TCNT0, R16						// Quitamos la bandera del desbordamiento
	INC		R23								// Incrementamos R23

	POP		R16								// Restauramos el SREG
	OUT		SREG, R16						// Restauramos el SREG
	POP		R16								// Restauramos el SREG
	RETI


AUMENTARSEG:
	INC		R19								// Aumentamos las unidades segundos
	RJMP	LOOP							
AUMENTARDEC:
	INC		R20								// Aumentamos las decenas de segundos
	RJMP	LOOP							

SEGUNDOS:
	CBI		PORTB, PB3						// Apagamos el PB3
	CALL	DELAY							// Llamamos a la función DELAY
	MOV		R17, R19						// Movemos R17 a R19
	LDI		ZH, HIGH(TABLA7SEG << 1)		
	LDI		ZL, LOW(TABLA7SEG << 1)				
	ADD		ZL, R17							
	LPM		R17, Z							// Leemos el valor

	CBI		PORTB, PB3						//

	OUT		PORTD, R17						// Mostramos el valor del display
	CBI		PORTB, PB3						// Apagamos PB3
	SBI		PORTB, PB4						// Encendemos PB4
	RET

DECENAS:
	CBI		PORTB, PB4						// Apagamos el PB4
	CALL	DELAY							// Llamamos a la función DELAY
	MOV		R18, R20						// Movemos R18 a R20
	LDI		ZH, HIGH(TABLA7SEG << 1)		
	LDI		ZL, LOW(TABLA7SEG << 1)			
	ADD		ZL, R18							
	LPM		R18, Z							// Leemos el valor

	CBI		PORTB, PB4						//

	OUT		PORTD, R18						// mostrar en el display	
	CBI		PORTB, PB4						// Apagamos PB4
	SBI		PORTB, PB3						// Encendemos PB3
	RET

DELAY:
    LDI R16, 20								// Cargamos un número de iteraciones
DELAY2:
    DEC R16									// Decrementamos el contador
    BRNE DELAY2								// Si no es 0 repetimos
    RET										
