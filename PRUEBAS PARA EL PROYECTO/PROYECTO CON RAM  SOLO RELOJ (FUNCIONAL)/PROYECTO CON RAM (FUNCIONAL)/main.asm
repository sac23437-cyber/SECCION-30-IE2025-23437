.include "M328PDEF.inc"

//------------------------------------------------------------Secci�n de datos (RAM)------------------------------------------------------------
.DSEG																	
UNIDADES_MINUTOS_RAM: .BYTE 1   ; Almacena las unidades de minutos
DECENAS_MINUTOS_RAM:  .BYTE 1   ; Almacena las decenas de minutos
UNIDADES_HORAS_RAM:   .BYTE 1   ; Almacena las unidades de horas
DECENAS_HORAS_RAM:    .BYTE 1   ; Almacena las decenas de horas
CONTADOR_INTERRUPCIONES: .BYTE 1 ; Almacena el contador de interrupciones
CONTADOR_SEGUNDOS:    .BYTE 1   ; Almacena el contador de segundos
//------------------------------------------------------------Secci�n de datos (RAM)------------------------------------------------------------

//--------------------------------------------------------------Secci�n de c�digo--------------------------------------------------------------
.CSEG																	
.org 0x00
    JMP SETUP                   ; Vector de reset

.org 0x0020                     ; Vector de interrupci�n del Timer0
    JMP TIMER0_ISR

TABLA7SEG: 
    .DB 0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F ; Valores de 0-9 para el display de 7 segmentos

SETUP:
    ; Configuraci�n del STACK
    LDI R16, LOW(RAMEND)
    OUT SPL, R16
    LDI R16, HIGH(RAMEND)
    OUT SPH, R16

    ; Configuraci�n de Entradas y Salidas
    LDI R16, 0b01111111         ; Colocamos (PD0 - PD6) como salidas
    OUT DDRD, R16               ; Mostramos el valor de R16 (Display)
    LDI R16, 0b00111111         ; Colocamos (PB0 - PB5) como salidas (PB4 y PB5 para LEDs)
    OUT DDRB, R16               ; Colocamos (PB0 - PB5) como salidas de control

    ; Inicializaci�n de variables en RAM
    LDI R16, 0
    STS UNIDADES_MINUTOS_RAM, R16
    STS DECENAS_MINUTOS_RAM, R16
    STS UNIDADES_HORAS_RAM, R16
    STS DECENAS_HORAS_RAM, R16
    STS CONTADOR_INTERRUPCIONES, R16
    STS CONTADOR_SEGUNDOS, R16

    ; Inicializaci�n de Registros
    LDI R17, 0                  ; Iniciamos R17 para la tabla de 7 segmentos
    LDI R18, 0                  ; Iniciamos R18 para el contador auxiliar
    LDI R19, 0                  ; Iniciamos R19 para el contador de interrupciones
    LDI R20, 0                  ; Iniciamos R20 para el contador de segundos

    ; Configuraci�n del Timer0 (para el reloj)
    CALL INICIO_TIMER0


    SEI                         ; Habilitamos las interrupciones globales

LOOP:
    CALL UNIDADES_MINUTOS       ; Mostramos las unidades de minutos
    CALL DECENAS_MINUTOS        ; Mostramos las decenas de minutos
    CALL UNIDADES_HORAS         ; Mostramos las unidades de horas
    CALL DECENAS_HORAS          ; Mostramos las decenas de horas

    LDS R20, CONTADOR_SEGUNDOS  ; Cargamos el contador de segundos desde RAM
    CPI R20, 2                 ; Verificamos si R20 es 60 (60 segundos = 1 minuto)
    BRNE LOOP                   ; Si no es 60, volvemos al LOOP
    CLR R20                     ; Reiniciamos R20 a 0
    STS CONTADOR_SEGUNDOS, R20  ; Guardamos el contador de segundos en RAM

    ; Incrementa minutos
    LDS R16, UNIDADES_MINUTOS_RAM ; Cargamos las unidades de minutos desde RAM
    CPI R16, 9                  ; Verificamos si el valor de las unidades de minutos es 9
    BREQ INC_DECENAS_MINUTOS    ; Si es 9, incrementamos las decenas de minutos
    INC R16                     ; Incrementa las unidades de minutos
    STS UNIDADES_MINUTOS_RAM, R16 ; Guardamos las unidades de minutos en RAM
    RJMP LOOP

INC_DECENAS_MINUTOS:
    LDI R16, 0                  ; Reiniciamos las unidades de minutos
    STS UNIDADES_MINUTOS_RAM, R16 ; Guardamos las unidades de minutos en RAM

    LDS R16, DECENAS_MINUTOS_RAM ; Cargamos las decenas de minutos desde RAM
    CPI R16, 5                  ; Verificamos si el valor de las decenas de minutos es 5
    BREQ INCREMENTAR_HORAS      ; Si es 5, incrementamos las horas
    INC R16                     ; Incrementamos las decenas de minutos
    STS DECENAS_MINUTOS_RAM, R16 ; Guardamos las decenas de minutos en RAM
    RJMP LOOP

INCREMENTAR_HORAS:
    LDI R16, 0                  ; Reinicia las decenas de minutos
    STS DECENAS_MINUTOS_RAM, R16 ; Guardamos las decenas de minutos en RAM

    LDS R16, DECENAS_HORAS_RAM  ; Cargamos las decenas de horas desde RAM
    CPI R16, 2                  ; Verifica si el valor de las decenas de horas es 2
    BRNE INC_UNIDADES_HORAS     ; Si no es 2, incrementamos las unidades de horas
    LDS R16, UNIDADES_HORAS_RAM ; Cargamos las unidades de horas desde RAM
    CPI R16, 3                  ; Si es 2, verificamos si el valor de las unidades de horas es 3
    BREQ REINICIAR              ; Si valor es 23:59, reiniciamos todo (00:00)
    INC R16                     ; Incrementamos las unidades de horas
    STS UNIDADES_HORAS_RAM, R16 ; Guardamos las unidades de horas en RAM
    RJMP LOOP

INC_UNIDADES_HORAS:
    LDS R16, UNIDADES_HORAS_RAM ; Cargamos las unidades de horas desde RAM
    CPI R16, 9                  ; Verificamos si las unidades de horas son 9
    BREQ INC_DECENAS_HORAS      ; Si es 9, incrementa las decenas de horas
    INC R16                     ; Incrementamos las unidades de horas
    STS UNIDADES_HORAS_RAM, R16 ; Guardamos las unidades de horas en RAM
    RJMP LOOP

INC_DECENAS_HORAS:
    LDI R16, 0                  ; Reiniciamos las unidades de horas
    STS UNIDADES_HORAS_RAM, R16 ; Guardamos las unidades de horas en RAM
    LDS R16, DECENAS_HORAS_RAM  ; Cargamos las decenas de horas desde RAM
    CPI R16, 1                  ; Verificamos si las decenas de horas son 1
    BREQ SET_DECENAS_HORAS_2    ; Si es 1, establecemos las decenas de horas a 2
    INC R16                     ; Incrementamos las decenas de horas
    STS DECENAS_HORAS_RAM, R16  ; Guardamos las decenas de horas en RAM
    RJMP LOOP

SET_DECENAS_HORAS_2:
    LDI R16, 2                  ; Establecemos las decenas de horas a 2
    STS DECENAS_HORAS_RAM, R16  ; Guardamos las decenas de horas en RAM
    RJMP LOOP

REINICIAR:
    LDI R16, 0                  ; Reiniciamos las unidades de horas
    STS UNIDADES_HORAS_RAM, R16 ; Guardamos las unidades de horas en RAM
    LDI R16, 0                  ; Reiniciamos las decenas de horas
    STS DECENAS_HORAS_RAM, R16  ; Guardamos las decenas de horas en RAM
    RJMP LOOP

INICIO_TIMER0:
    LDI R16, (1 << CS02) | (1 << CS00) ; Configuramos el prescaler en 1024
    OUT TCCR0B, R16
    LDI R16, 0                  ; Valor inicial del contador
    OUT TCNT0, R16
    LDI R16, (1 << TOIE0)       ; Habilitamos la interrupci�n por desbordamiento
    STS TIMSK0, R16
    RET


TIMER0_ISR:
    PUSH R16                    ; Guardamos R16
    IN R16, SREG                ; Guardamos el estado de SREG
    PUSH R16

    LDI R16, 0                  ; Colocamos el valor inicial del contador
    OUT TCNT0, R16

    LDS R16, CONTADOR_INTERRUPCIONES ; Cargamos el contador de interrupciones desde RAM
    INC R16                     ; Incrementamos el contador de interrupciones
    STS CONTADOR_INTERRUPCIONES, R16 ; Guardamos el contador de interrupciones en RAM

    CPI R16, 2                 ; Verificamos si R16 es 61 (61 interrupciones = 1 segundo)
    BRNE FIN_TIMER0             ; Si no es 61, vamos a FIN_TIMER0

    LDI R16, 0                  ; Reiniciamos el contador de interrupciones
    STS CONTADOR_INTERRUPCIONES, R16 ; Guardamos el contador de interrupciones en RAM

    LDS R16, CONTADOR_SEGUNDOS  ; Cargamos el contador de segundos desde RAM
    INC R16                     ; Incrementamos el contador de segundos
    STS CONTADOR_SEGUNDOS, R16  ; Guardamos el contador de segundos en RAM

FIN_TIMER0:
    POP R16                     ; Restauramos SREG
    OUT SREG, R16
    POP R16                     ; Restauramos R16
    RETI


UNIDADES_MINUTOS:
    CBI PORTB, PB3              ; Colocamos PB3 como bajo
    CBI PORTB, PB2              ; Colocamos PB2 como bajo
    CBI PORTB, PB1              ; Colocamos PB1 como bajo
    CBI PORTB, PB0              ; Colocamos PB0 como bajo

    LDS R17, UNIDADES_MINUTOS_RAM ; Cargamos las unidades de minutos desde RAM
    LDI ZH, HIGH(TABLA7SEG << 1)
    LDI ZL, LOW(TABLA7SEG << 1)
    ADD ZL, R17
    LPM R17, Z                  ; Obtenemos el valor de la tabla del display
    OUT PORTD, R17              ; Mostramos R17

    SBI PORTB, PB0              ; Activamos el display de unidades de minutos
    RET

DECENAS_MINUTOS:
    CBI PORTB, PB3              ; Colocamos PB3 como bajo
    CBI PORTB, PB2              ; Colocamos PB2 como bajo
    CBI PORTB, PB1              ; Colocamos PB1 como bajo
    CBI PORTB, PB0              ; Colocamos PB0 como bajo

    LDS R18, DECENAS_MINUTOS_RAM ; Cargamos las decenas de minutos desde RAM
    LDI ZH, HIGH(TABLA7SEG << 1)
    LDI ZL, LOW(TABLA7SEG << 1)
    ADD ZL, R18
    LPM R18, Z                  ; Obtenemos el valor de la tabla del display
    OUT PORTD, R18              ; Mostramos R18

    SBI PORTB, PB1              ; Activamos el display de decenas de minutos
    RET

UNIDADES_HORAS:
    CBI PORTB, PB3              ; Colocamos PB3 como bajo
    CBI PORTB, PB2              ; Colocamos PB2 como bajo
    CBI PORTB, PB1              ; Colocamos PB1 como bajo
    CBI PORTB, PB0              ; Colocamos PB0 como bajo

    LDS R17, UNIDADES_HORAS_RAM ; Cargamos las unidades de horas desde RAM
    LDI ZH, HIGH(TABLA7SEG << 1)
    LDI ZL, LOW(TABLA7SEG << 1)
    ADD ZL, R17
    LPM R17, Z                  ; Obtenemos el valor de la tabla del display
    OUT PORTD, R17              ; Mostramos R17

    SBI PORTB, PB2              ; Activamos el display de unidades de horas
    RET

DECENAS_HORAS:
    CBI PORTB, PB3              ; Colocamos PB3 como bajo
    CBI PORTB, PB2              ; Colocamos PB2 como bajo
    CBI PORTB, PB1              ; Colocamos PB1 como bajo
    CBI PORTB, PB0              ; Colocamos PB0 como bajo

    LDS R18, DECENAS_HORAS_RAM  ; Cargamos las decenas de horas desde RAM
    LDI ZH, HIGH(TABLA7SEG << 1)
    LDI ZL, LOW(TABLA7SEG << 1)
    ADD ZL, R18
    LPM R18, Z                  ; Obtenemos el valor de la tabla del display
    OUT PORTD, R18              ; Mostramos R18

    SBI PORTB, PB3              ; Activamos el display de decenas de horas
    RET