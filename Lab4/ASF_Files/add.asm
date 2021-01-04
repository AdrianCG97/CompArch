.orig x3000
	LEA R0,A

	LDW R1,R0,#0	;Load 1 into x4000
	AND R2,R2,#0
	ADD R2,R2,#1
	STW R2,R1,#0
	And R2,R2,#0

	LDW R1,R0,#2
	LDW R5,R0,#4

B	ADD R4,R5,R1	;Check if we are done
	Bz C 			;sum in R2
	LDB R3,R1,#0	;Address in R1
	ADD R2,R2,R3
	ADD R1,R1,#1
	BRnzp B
C 	STW R2,R1,#0

	;Now check for exceptions
	;Protection exception
	And r0,r0,#0
	STW r2,r0,#0

	;Unaligned Access
	;LDW R0,R0,#3
	;STW R2,R0,#0

	;Unknown Opcode
	;.FILL xA000

	HALT

A	.FILL x4000
	.FILL xC000
	.FILL x3FEC
	.FILL xC017
.END