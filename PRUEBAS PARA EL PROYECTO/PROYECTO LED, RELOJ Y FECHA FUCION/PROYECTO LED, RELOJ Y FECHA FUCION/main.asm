.include "M328PDEF.inc"

//------------------------------------------------------------Sección de datos (RAM)------------------------------------------------------------
.DSEG																	
DIA:                   .BYTE 1   ; Almacena el día
MES:                   .BYTE 1   ; Almacena el mes
UNIDADES_MINUTOS_RAM:  .BYTE 1   ; Almacena las unidades de minutos
DECENAS_MINUTOS_RAM:   .BYTE 1   ; Almacena las decenas de minutos
UNIDADES_HORAS_RAM:    .BYTE 1   ; Almacena las unidades de horas
DECENAS_HORAS_RAM:     .BYTE 1   ; Almacena las decenas de horas
CONTADOR_INTERRUPCIONES: .BYTE 1 ; Almacena el contador de interrupciones
CONTADOR_SEGUNDOS:     .BYTE 1   ; Almacena el contador de segundos
MODO_VISUALIZACION:    .BYTE 1   ; 0 = Fecha, 1 = Horaa
LED_STATE:				.BYTE 1
LED_BLINK_COUNT:       .BYTE 1   ; Contador para el parpadeo de LEDs
//------------------------------------------------------------Sección de datos (RAM)------------------------------------------------------------

//--------------------------------------------------------------Sección de código--------------------------------------------------------------
.CSEG																	
.org 0x00
    JMP SETUP                   ; Vector de reset

.org 0x0020                     ; Vector de interrupción del Timer0
    JMP TIMER0_ISR

TABLA7SEG: 
    .DB 0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F ; Valores de 0-9 para el display de 7 segmentos

TABLADIAS:
	.DB 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 ; Días de cada mes

SETUP:
    ; Configuración del STACK
    LDI R16, LOW(RAMEND)
    OUT SPL, R16
    LDI R16, HIGH(RAMEND)
    OUT SPH, R16

    ; Configuración de Entradas y Salidas
    LDI R16, 0b01111111         ; Colocamos (PD0 - PD6) como salidas
    OUT DDRD, R16               ; Mostramos el valor de R16 (Display)
    LDI R16, 0b00011111         ; Colocamos (PB0 - PB4) como salidas (PB4 para LEDs)
    OUT DDRB, R16               ; Colocamos (PB0 - PB4) como salidas de control
    LDI R16, 0b00000000         ; Configuramos (PC2 - PC5) como entrada (botón)
    OUT DDRC, R16

    ; Inicialización de variables en RAM
    LDI R16, 1                  ; Día inicial (1)
    STS DIA, R16
    LDI R16, 1                  ; Mes inicial (1)
    STS MES, R16
    LDI R16, 0
    STS UNIDADES_MINUTOS_RAM, R16
    STS DECENAS_MINUTOS_RAM, R16
    STS UNIDADES_HORAS_RAM, R16
    STS DECENAS_HORAS_RAM, R16
    STS CONTADOR_INTERRUPCIONES, R16
    STS CONTADOR_SEGUNDOS, R16
    LDI R16, 1                  ; Modo inicial: Hora
    STS MODO_VISUALIZACION, R16
    STS LED_STATE, R16
    STS LED_BLINK_COUNT, R16    ; Inicializar el contador de parpadeo de LEDs

    ; Inicialización de Registros
    LDI R17, 0                  ; Iniciamos R17 para la tabla de 7 segmentos
    LDI R18, 0                  ; Iniciamos R18 para el contador auxiliar
    LDI R19, 0                  ; Iniciamos R19 para el contador de interrupciones
    LDI R20, 0                  ; Iniciamos R20 para el contador de segundos

    ; Configuración del Timer0 (para el reloj y el parpadeo de LEDs)
    CALL INICIO_TIMER0

    SEI                         ; Habilitamos las interrupciones globales

LOOP:
    ; Verificar el estado del botón en PC2
    IN R16, PINC
    SBRS R16, PC2               ; Si el botón está presionado (PC2 en alto), cambiar el modo
    RJMP MOSTRAR_DISPLAY        ; Si no está presionado, continuar mostrando el display actual

    ; Cambiar el modo de visualización
    LDS R16, MODO_VISUALIZACION
    CPI R16, 0
    BREQ CAMBIAR_A_HORA
    LDI R16, 0
    STS MODO_VISUALIZACION, R16
    RJMP MOSTRAR_DISPLAY

CAMBIAR_A_HORA:
    LDI R16, 1
    STS MODO_VISUALIZACION, R16

MOSTRAR_DISPLAY:
    LDS R16, MODO_VISUALIZACION
    CPI R16, 0
    BREQ MOSTRAR_FECHA
    CALL MOSTRAR_HORA
    RJMP LOOP

MOSTRAR_FECHA:
    CALL MOSTRAR_FECHA_SUB
    RJMP LOOP

MOSTRAR_FECHA_SUB:
    ; Decenas del día
    LDS R16, DIA                ; Cargamos el día desde RAM
    CALL DIVIDIR_DECENAS        ; Separamos decenas y unidades
    MOV R19, R16                ; Guardamos las decenas en R19

    ; Unidades del día
    LDS R16, DIA                ; Cargamos el día desde RAM
    CALL DIVIDIR_UNIDADES       ; Separamos decenas y unidades
    MOV R20, R16                ; Guardamos las unidades en R20

    ; Decenas del mes
    LDS R16, MES                ; Cargamos el mes desde RAM
    CALL DIVIDIR_DECENAS        ; Separamos decenas y unidades
    MOV R21, R16                ; Guardamos las decenas en R21

    ; Unidades del mes
    LDS R16, MES                ; Cargamos el mes desde RAM
    CALL DIVIDIR_UNIDADES       ; Separamos decenas y unidades
    MOV R22, R16                ; Guardamos las unidades en R22

    ; Mostrar decenas del día
    MOV R16, R19
    CALL MOSTRAR_DIGITO
    SBI PORTB, PB3              ; Activamos el display de decenas de día
    CALL DELAY
    CBI PORTB, PB3              ; Desactivamos el display

    ; Mostrar unidades del día
    MOV R16, R20
    CALL MOSTRAR_DIGITO
    SBI PORTB, PB2              ; Activamos el display de unidades de día
    CALL DELAY
    CBI PORTB, PB2              ; Desactivamos el display

    ; Mostrar decenas del mes
    MOV R16, R21
    CALL MOSTRAR_DIGITO
    SBI PORTB, PB1              ; Activamos el display de decenas de mes
    CALL DELAY
    CBI PORTB, PB1              ; Desactivamos el display

    ; Mostrar unidades del mes
    MOV R16, R22
    CALL MOSTRAR_DIGITO
    SBI PORTB, PB0              ; Activamos el display de unidades de mes
    CALL DELAY
    CBI PORTB, PB0              ; Desactivamos el display

    RET

MOSTRAR_HORA:
    CALL UNIDADES_MINUTOS       ; Mostramos las unidades de minutos
    CALL DECENAS_MINUTOS        ; Mostramos las decenas de minutos
    CALL UNIDADES_HORAS         ; Mostramos las unidades de horas
    CALL DECENAS_HORAS          ; Mostramos las decenas de horas
    RET

MOSTRAR_DIGITO:
    ; Cargar el valor del dígito desde la tabla de 7 segmentos
    LDI ZH, HIGH(TABLA7SEG << 1)
    LDI ZL, LOW(TABLA7SEG << 1)
    ADD ZL, R16
    LPM R16, Z                  ; Obtenemos el valor de la tabla del display
    OUT PORTD, R16              ; Mostramos R16 en el display
    RET

DIVIDIR_DECENAS:
    ; Divide un número en decenas (R16 / 10)
    LDI R17, 10
    CLR R18
DIVIDE_LOOP:
    INC R18
    SUB R16, R17
    BRCC DIVIDE_LOOP            ; Si no hay acarreo, continuar
    DEC R18
    ADD R16, R17
    MOV R16, R18                ; Guardamos las decenas en R16
    RET

DIVIDIR_UNIDADES:
    ; Obtiene las unidades de un número (R16 % 10)
    LDI R17, 10
DIVIDE_UNIDADES_LOOP:
    CPI R16, 10
    BRLO FIN_DIVIDE_UNIDADES    ; Si R16 < 10, terminar
    SUB R16, R17
    RJMP DIVIDE_UNIDADES_LOOP
FIN_DIVIDE_UNIDADES:
    RET

DELAY:
    ; Retardo para la multiplexación
    LDI R16, 50
DELAY_LOOP:
    DEC R16
    BRNE DELAY_LOOP
    RET

INICIO_TIMER0:
    LDI R16, (1 << CS02) | (1 << CS00) ; Configuramos el prescaler en 1024
    OUT TCCR0B, R16
    LDI R16, 0                  ; Valor inicial del contador
    OUT TCNT0, R16
    LDI R16, (1 << TOIE0)       ; Habilitamos la interrupción por desbordamiento
    STS TIMSK0, R16
    RET

TIMER0_ISR:
    PUSH R16                    ; Guardamos R16
    IN R16, SREG                ; Guardamos el estado de SREG
    PUSH R16

    LDI R16, 0                  ; Colocamos el valor inicial del contador
    OUT TCNT0, R16

    ; Incrementar el contador de interrupciones
    LDS R16, CONTADOR_INTERRUPCIONES
    INC R16
    STS CONTADOR_INTERRUPCIONES, R16

    ; Verificar si han pasado 61 interrupciones (1 segundo)
    CPI R16, 61 //---------------------------------------------------------------------------------> CAMBIAR PARA QUE VAYA MÁS RÁPIDO 
    BRNE CHECK_LED_BLINK        ; Si no es 61, verificar el parpadeo de LEDs

    ; Reiniciar el contador de interrupciones
    LDI R16, 0
    STS CONTADOR_INTERRUPCIONES, R16

    ; Incrementar el contador de segundos
    LDS R16, CONTADOR_SEGUNDOS
    INC R16
    STS CONTADOR_SEGUNDOS, R16

    ; Verificar si han pasado 60 segundos (1 minuto)
    CPI R16, 60 //---------------------------------------------------------------------------------> CAMBIAR PARA QUE VAYA MÁS RÁPIDO
    BRNE CHECK_LED_BLINK        ; Si no es 60, verificar el parpadeo de LEDs

    ; Reiniciar el contador de segundos
    LDI R16, 0
    STS CONTADOR_SEGUNDOS, R16

    ; Incrementar el tiempo (minutos y horas)
    CALL INCREMENTAR_TIEMPO

CHECK_LED_BLINK:
    ; Verificar si han pasado 30 interrupciones (500ms)
    LDS R16, CONTADOR_INTERRUPCIONES
    CPI R16, 30
    BRNE FIN_TIMER0             ; Si no es 30, salir de la interrupción

    ; Alternar el estado de los LEDs
    LDS R16, LED_STATE
    CPI R16, 0
    BREQ ENCENDER_LEDS
    CBI PORTB, PB4              ; Apagar LED en PB4
    LDI R16, 0
    STS LED_STATE, R16
    RJMP FIN_TIMER0

ENCENDER_LEDS:
    SBI PORTB, PB4              ; Encender LED en PB4
    LDI R16, 1
    STS LED_STATE, R16          ; Guardar el nuevo estado de los LEDs

FIN_TIMER0:
    POP R16                     ; Restauramos SREG
    OUT SREG, R16
    POP R16                     ; Restauramos R16
    RETI

INCREMENTAR_TIEMPO:
    ; Incrementa minutos
    LDS R16, UNIDADES_MINUTOS_RAM ; Cargamos las unidades de minutos desde RAM
    CPI R16, 9                  ; Verificamos si el valor de las unidades de minutos es 9
    BREQ INC_DECENAS_MINUTOS    ; Si es 9, incrementamos las decenas de minutos
    INC R16                     ; Incrementa las unidades de minutos
    STS UNIDADES_MINUTOS_RAM, R16 ; Guardamos las unidades de minutos en RAM
    RET

INC_DECENAS_MINUTOS:
    LDI R16, 0                  ; Reiniciamos las unidades de minutos
    STS UNIDADES_MINUTOS_RAM, R16 ; Guardamos las unidades de minutos en RAM

    LDS R16, DECENAS_MINUTOS_RAM ; Cargamos las decenas de minutos desde RAM
    CPI R16, 5                  ; Verificamos si el valor de las decenas de minutos es 5
    BREQ INCREMENTAR_HORAS      ; Si es 5, incrementamos las horas
    INC R16                     ; Incrementamos las decenas de minutos
    STS DECENAS_MINUTOS_RAM, R16 ; Guardamos las decenas de minutos en RAM
    RET

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
    RET

INC_UNIDADES_HORAS:
    LDS R16, UNIDADES_HORAS_RAM ; Cargamos las unidades de horas desde RAM
    CPI R16, 9                  ; Verificamos si las unidades de horas son 9
    BREQ INC_DECENAS_HORAS      ; Si es 9, incrementa las decenas de horas
    INC R16                     ; Incrementamos las unidades de horas
    STS UNIDADES_HORAS_RAM, R16 ; Guardamos las unidades de horas en RAM
    RET

INC_DECENAS_HORAS:
    LDI R16, 0                  ; Reiniciamos las unidades de horas
    STS UNIDADES_HORAS_RAM, R16 ; Guardamos las unidades de horas en RAM
    LDS R16, DECENAS_HORAS_RAM  ; Cargamos las decenas de horas desde RAM
    CPI R16, 1                  ; Verificamos si las decenas de horas son 1
    BREQ SET_DECENAS_HORAS_2    ; Si es 1, establecemos las decenas de horas a 2
    INC R16                     ; Incrementamos las decenas de horas
    STS DECENAS_HORAS_RAM, R16  ; Guardamos las decenas de horas en RAM
    RET

SET_DECENAS_HORAS_2:
    LDI R16, 2                  ; Establecemos las decenas de horas a 2
    STS DECENAS_HORAS_RAM, R16  ; Guardamos las decenas de horas en RAM
    RET

REINICIAR:
    LDI R16, 0                  ; Reiniciamos las unidades de horas
    STS UNIDADES_HORAS_RAM, R16 ; Guardamos las unidades de horas en RAM
    LDI R16, 0                  ; Reiniciamos las decenas de horas
    STS DECENAS_HORAS_RAM, R16  ; Guardamos las decenas de horas en RAM

	CALL INCREMENTAR_FECHA

    RET

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

INCREMENTAR_FECHA:
    ; Cargar el mes actual
    LDS R16, MES
    DEC R16                     ; Ajustar el mes para usarlo como índice (0-11)

    ; Cargar la cantidad de días del mes actual desde la tabla
    LDI ZH, HIGH(TABLADIAS << 1)
    LDI ZL, LOW(TABLADIAS << 1)
    ADD ZL, R16                 ; Sumar el índice del mes
    LPM R17, Z                  ; Cargar los días del mes en R17

    ; Verificar si es febrero (mes 2) y si es año bisiesto (opcional)
    CPI R16, 1                  ; Febrero es el mes 2 (índice 1)
    BRNE CHECK_LAST_DAY         ; Si no es febrero, saltar a verificar el último día

    ; Aquí puedes agregar lógica para años bisiestos si es necesario
    ; Por ahora, asumimos que febrero tiene 28 días

CHECK_LAST_DAY:
    ; Verificar si el día actual es igual al último día del mes
    LDS R16, DIA
    CP R16, R17
    BRLO INCREMENTAR_DIA        ; Si el día es menor que los días del mes, incrementar el día

    ; Si el día es igual al último día del mes, reiniciar el día a 1 e incrementar el mes
    LDI R16, 1
    STS DIA, R16

    ; Incrementar el mes
    LDS R16, MES
    INC R16
    STS MES, R16

    ; Verificar si el mes supera 12
    CPI R16, 13
    BRLO FIN_INCREMENTAR_FECHA  ; Si el mes es menor que 13, terminar

    ; Reiniciar el mes a 1 (enero)
    LDI R16, 1
    STS MES, R16
    RJMP FIN_INCREMENTAR_FECHA

INCREMENTAR_DIA:
    ; Incrementar el día
    LDS R16, DIA
    INC R16
    STS DIA, R16

FIN_INCREMENTAR_FECHA:
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