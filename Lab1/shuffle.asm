.ORIG x3000
	LEA R1, A
	LDW R1,R1,#0
S	LEA R2, B
	LDW R2,R2,#0
	ADD R2,R0,R2
	BRz DONE
	LDB R3,R1,#0
	LEA R5, C
	LDW R5,R5,#0
	AND R3,R3,R5
	LDB R4,R1,#1
	AND R4,R4,R5
	LSHF R4,R4,#8
	ADD R3,R3,R4
	STW R3,R1,#0
	ADD R1,R1,#2
	ADD R0,R0,#-1
	BR S
DONE HALT
A 	.FILL x3050
B 	.FILL x0000
C 	.FILL x00FF
.END