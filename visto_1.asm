;-------------------------------------------------------------------------------
; tem que ver se o erro é em 16 bits ou não, na parte mov e inc
;-------------------------------------------------------------------------------

main:
NUM .equ 2024                   ;Indicar número a ser convertido
mov     #NUM,R5                 ;R5 = número a ser convertido
mov     #RESP,R6                ;R6 = ponteiro para escrever a resposta
call    #ALG_ROM
jmp $
nop

ALG_ROM:
mov     #0,R6                   ;inicia o acumulador

LOOP:
mov.b   @R6+,R7
cmp.b   #0,R7                   ;testa se der 0, se for igual, fim
jz      FIM

CASE_M:
sub     #0x03E8,R7              ;R7 - 1000
jn      CASE_CM                 ;Caso seja negativo, vai pro proximo
mov.b   #'M', 0(R6)
inc.b   R6                      ; Atualiza R6 para apontar para o próximo byte livre
jmp     LOOP

CASE_CM:
sub     #0x0384, R7             ; R7 - 900
jn      CASE_D                  ; Se for negativo, vai para o próximo caso
mov.b   #'C', 0(R6)             ; Adiciona 'C'
mov.b   #'M', 1(R6)             ; Adiciona 'M'
inc.b   R6                      ; Atualiza R6 para apontar para o próximo byte livre
inc.b   R6                      ; Atualiza R6 para apontar para o próximo byte livre
jmp     LOOP

CASE_D:
sub     #0x01F4, R7             ; R7 - 500
jn      CASE_CD                 ; Caso seja negativo, vai pro proximo
mov.b   #'D', 0(R6)
inc.b   R6                      ; Atualiza R6 para apontar para o próximo byte livre
jmp     LOOP

CASE_CD:
sub     #0x0190,R7              ;R7 - 400
jn      CASE_C                  ;Caso seja negativo, vai pro proximo
mov.b   #'C', 0(R6)             ; Adiciona 'C'
mov.b   #'D', 1(R6)             ; Adiciona 'D'
inc.b   R6                      ; Atualiza R6 para apontar para o próximo byte livre
inc.b   R6                      ; Atualiza R6 para apontar para o próximo byte livre
jmp     LOOP

CASE_C:
sub     #0x0064,R7              ;R7 - 100
jn      CASE_XC                 ;Caso seja negativo, vai pro proximo
mov.b   #'C', 0(R6)
inc.b   R6                      ; Atualiza R6 para apontar para o próximo byte livre
jmp     LOOP

CASE_XC:
sub     #0x005A,R7              ;R7 - 90
jn      CASE_L                  ;Caso seja negativo, vai pro proximo
mov.b   #'X', 0(R6)             ; Adiciona 'X'
mov.b   #'C', 1(R6)             ; Adiciona 'C'
inc.b   R6                      ; Atualiza R6 para apontar para o próximo byte livre
jmp     LOOP

CASE_L:
sub     #0x0032,R7              ;R7 - 50
jn      CASE_XL                 ;Caso seja negativo, vai pro proximo
mov.b   #'L', 0(R6)
inc.b   R6                      ; Atualiza R6 para apontar para o próximo byte livre
jmp     LOOP

CASE_XL:
sub     #0x0028,R7              ;R7 - 40
jn      CASE_X                  ;Caso seja negativo, vai pro proximo
mov.b   #'X', 0(R6)             ; Adiciona 'X'
mov.b   #'L', 1(R6)             ; Adiciona 'L'
inc.b   R6                      ; Atualiza R6 para apontar para o próximo byte livre
inc.b   R6                      ; Atualiza R6 para apontar para o próximo byte livre
jmp     LOOP

CASE_X:
sub     #0x000A,R7              ;R7 - 10
jn      CASE_IX                 ;Caso seja negativo, vai pro proximo
mov.b   #'X', 0(R6)
inc.b   R6                      ; Atualiza R6 para apontar para o próximo byte livre
jmp     LOOP

CASE_IX:
sub     #0x0009,R7              ;R7 - 9
jn      CASE_V                  ;Caso seja negativo, vai pro proximo
mov.b   #'I', 0(R6)             ; Adiciona 'I'
mov.b   #'X', 1(R6)             ; Adiciona 'X'
inc.b   R6                      ; Atualiza R6 para apontar para o próximo byte livre
inc.b   R6                      ; Atualiza R6 para apontar para o próximo byte livre
jmp     LOOP


CASE_V:
sub     #0x0005,R7              ;R7 - 5
jn      CASE_IV                 ;Caso seja negativo, vai pro proximo
mov.b       #'V', 0(R6)
inc.b   R6                      ; Atualiza R6 para apontar para o próximo byte livre
jmp     LOOP

CASE_IV:
sub     #0x0004,R7              ;R7 - 4
jn      CASE_I                  ;Caso seja negativo, vai pro proximo
mov.b   #'I', 0(R6)             ; Adiciona 'C'
mov.b   #'V', 1(R6)             ; Adiciona 'M'
inc.b   R6                      ; Atualiza R6 para apontar para o próximo byte livre
inc.b   R6                      ; Atualiza R6 para apontar para o próximo byte livre
jmp     LOOP

CASE_I:
sub     #0x0001,R7              ;R7 - 1
jn      FIM                     ;Caso seja negativo, vai pro proximo
mov.b   #'I', 0(R6)
inc.b   R6                      ; Atualiza R6 para apontar para o próximo byte livre
jmp     LOOP
FIM:
ret

;-------------------------------------------------------------------------------
; Stack Pointer definition
;-------------------------------------------------------------------------------
.global __STACK_END
.sect   .stack

;-------------------------------------------------------------------------------
; Interrupt Vectors
;-------------------------------------------------------------------------------
.sect   ".reset"                ; MSP430 RESET Vector
.short  RESET

.data
RESP:       .byte "RRRRRRRRRRRRRRRRRR",0
