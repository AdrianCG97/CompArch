.orig x1200
	STW R0,R6,#-1	;Push R0 and R1
	STW R1,R6,#-2

	LEA R0,A 		;Get value from x4000
	LWD R1,R0,#0
	LDW R0,R1,#0

	ADD R0,R0,#1 	;Add 1 and store back
	STW R0,R1,#0
	LDW R1,R6,#-2	;Pop R0 and R1
	LDW R0,R6,#-1
	RTI
A	.FILL x4000
.end