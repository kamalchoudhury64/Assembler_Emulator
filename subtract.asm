;test7.asm 
; subtract 2 numbers from memory & store it somewhere else in memory (difference label)

ldc arr ; loading address of first number in A register
ldnl 0  ; loading the first number in A register
ldc arr ; loading address of first number in A register & B register stores the first number
ldnl 1  ; loading second number in A register
sub	; A stores (B-A) (First number - second number)
ldc difference ; Loading address in A where difference needs to be stored & B stores the difference
stnl 0 ; Value of B is written in memory & difference of two numbers is stored in memory
HALT	;Halting the program

; data segment
arr:	data	0x0008 ;first number
	data	0x0001 ;second number 

difference: data 0x0000 