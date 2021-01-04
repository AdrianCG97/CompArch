;This program should calculate the sum of the first 20 bytes stored in memory starting at address xC000. 
;This sum should then be stored as a 16-bit word at xC014. 
;Then the program should jump to the address given by the value of the sum.

.ORIG x3000
	LEA R5,A
	LDW R0,R5,#0	;Ptr to list of values
	LDW R4,R5,#1 	;R4 holds xC014
	LDW R1,R5,#2	;index


B	ADD R2,R0,R1
	LDB R2,R2,#0
	ADD R3,R3,R2
	ADD R1,R1,#-1
	BRzp b

	STW R3,R4,#0
	JMP R3


A 	.FILL xC000	;R0
 	.FILL xC014	;R4
 	.FILL x0013 ;R1

.END 