; Include the standard 8051 register definitions


CSEG AT 0300H
	segmentos1:			DB 0x40, 0x79, 0x24, 0x30, 0x19, 0x12, 0x02, 0x78, 0x00 , 0x10, 0x3F ; 0. , 1. , 2. , 3. , 4. , 5. , 6. , 7. , 8. , 9. , -. 
	segmentos2:			DB	0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90 , 0xBF, 0x88, 0x83, 0xC6, 0XA1;0, 1, 2, 3, 4, 5, 6, 7, 8, 9
; Define constants for timer reload values
TempoH EQU 0D8h
TempoL EQU 0F0h

; Define segment bit locations for display D1 and D2
D1 EQU P1
D2 EQU P2
	
	
bA BIT P3.4
bB BIT P3.5
bC BIT P3.6
bD BIT P3.7



CSEG AT 0050h
INICIO:
    ; Initialize the microcontroller
    MOV R0, #5 ; segundos
    MOV R1, #0 ; decimasSegundos
    MOV R2, #0 ; conta
    MOV R3, #1 ; start
    MOV R4, #0 ; resposta
	MOV R5, #0; GuardaSegundos
	MOV R6, #0; GuardaDecimasSegundos
    
    MOV IE, #8Bh   ; Enable interrupts EA, ET0, ET1, EX0
    MOV IP, #00h   ; Set default interrupt priorities
    MOV TMOD, #11h ; Set Timer 0 and Timer 1 in Mode 1
    
    ; Set Timer 0 initial values for 5ms
    MOV TH0, #TempoH
    MOV TL0, #TempoL

    ; Set Timer 1 initial values for 50ms
    MOV TH1, #TempoH
    MOV TL1, #TempoL
    
    CLR TR0
    CLR TR1
    
    SETB IT0
    SETB IT1

    ; Start the main loop
MAIN_LOOP:
    CALL DISPLAY
    SJMP MAIN_LOOP

; External Interrupt 0 ISR
EX0_ISR:
    MOV A, R3 ; A <- Start
    JZ EX0_ISR_s0
    
    SETB TR0  ; Start Timer 0
    SETB TR1  ; Start Timer 1
    CLR EX0   ; Disable external interrupt 0
	SETB EX1;


    JMP Fim_EXO_ISR
    
EX0_ISR_s0:
    MOV R3, #1 ;Start = 1
    MOV R4, #0 ; resposta = 0
    MOV R0, #5 ; Segundos = 5
    MOV R1, #0; DecimasSegundos = 0
    
    MOV TH0, TempoH
    MOV TL0, TempoL
    
    MOV TH1, TempoH
    MOV TL1, TempoL
    
    SETB TR0
    CLR TR1
    CLR EX0
    MOV R2, #0 ; Start = 0 
    
Fim_EXO_ISR:  
    RETI

; Timer 0 ISR - Debounce handling
TIMER0_ISR:
    CLR TR0   ; Stop Timer 0
    
    CLR IE0   ; Clear External Interrupt 0 flag
    CLR IE1
    
    SETB EX0

    ; Reset Timer 0
    MOV TH0, #TempoH
    MOV TL0, #TempoL

    RETI

; External Interrupt 1 ISR - Response handling
EX1_ISR:
    CLR EX1   ; Disable external interrupt 1

    ; Check which response is selected
    JNB bA, SELECT_A
    JNB bB, SELECT_B
    JNB bC, SELECT_C
    JNB bD, SELECT_D

    JMP HANDLE_RESPONSE

SELECT_A:
    MOV R4, #1
    SJMP HANDLE_RESPONSE

SELECT_B:
    MOV R4, #2
    SJMP HANDLE_RESPONSE

SELECT_C:
    MOV R4, #3
    SJMP HANDLE_RESPONSE

SELECT_D:
    MOV R4, #4

HANDLE_RESPONSE:
    MOV R3, #0   ; Start = 0 
	MOV A, R0   
	MOV R5, A; GuardaSegundos = Segundos
	MOV A, R1
	MOV R6, A ;GuardaDecimasSegundos = DecimasSegundos

    RETI


; Timer 1 ISR - Counting time and display control
TIMER1_ISR:
    ; Check if still counting down
	
	
	
    MOV A, R3
    CJNE A, #1, NOT_COUNTING ; IF(start==0)
	
	MOV A, R2
	CJNE A, #10, CONTINUE_COUNTING ; If(conta ==10){
	MOV R2, #0; conta = 0
	
	
	MOV A , R1
    JNZ DecrementaDecimas ;IF(DecimasSegundos==0)
	
	MOV A, R0
	JNZ SegundosNaoZero ;if(Segundos ==0){
	CLR EX1
	MOV A , R0
    MOV R5, A
	MOV A, R1
	MOV R6, A
	MOV R3, #0
	MOV R2, #0
	JMP return
	
SegundosNaoZero:
    MOV R1, #9
    DEC R0
	
	JMP return

	;}
	
	

	

    JMP Return
	;}
	;}
DecrementaDecimas:
	DEC R1; DecimasSegundos --
	JMP Return

	
NOT_COUNTING:
    MOV A, R2
	CJNE A, #100, CONTINUE_COUNTING ; If(conta ==100){
	MOV R2, #0; conta = 0
	MOV A, R0
	CJNE A, #10, NotSecond; if(segundos = 10)
	MOV A, R5; Segundos = GuardaSegundos
	MOV R0, A 
	MOV A, R6
	MOV R1,A; DecimasSegundos = GuardaDecimasSegundos
	JMP Return

NotSecond:

	MOV R0, #10
	CJNE R4, #1, RespostaB
	MOV R1, #11
	JMP Return
RespostaB:
	CJNE R4, #2,RespostaC
	MOV R1, #12
	JMP Return
RespostaC:
	CJNE R4, #3,RespostaD
	MOV R1, #13
	JMP Return
RespostaD:
	CJNE R4, #4,RespostaDefault
	MOV R1, #14
	JMP Return
	
RespostaDefault:	
	MOV R1, #10
	JMP Return	


CONTINUE_COUNTING:
    INC R2 ; CONTA++
	JMP Return
Return:
    MOV TH1, #TempoH
    MOV TL1, #TempoL		
	RETI	

; Display subroutine - Control the 7-segment displays
DISPLAY:
    ; Display digit D1 based on seconds
    MOV A, R0
    ACALL DISPLAY_DIGIT_D1

    ; Display digit D2 based on tenths of seconds
    MOV A, R1
    ACALL DISPLAY_DIGIT_D2
    
    
    RET

DISPLAY_DIGIT_D1:
    ; Lookup segment values for display D1
    MOV DPTR, #segmentos1
    MOVC A, @A+DPTR

    ; Update segments for D1
    MOV D1, A
    RET

DISPLAY_DIGIT_D2:
    ; Lookup segment values for display D2
    MOV DPTR, #segmentos2
    MOVC A, @A+DPTR
    ; Update segments for D2
    MOV D2, A
    RET
    


; Segmento de interrupção
CSEG AT 0000h
    LJMP INICIO
CSEG AT 0003h 
    LJMP EX0_ISR
CSEG AT 000Bh
    LJMP TIMER0_ISR
CSEG AT 0013h
    LJMP EX1_ISR
CSEG AT 001Bh
    LJMP TIMER1_ISR

END
