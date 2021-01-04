
;The interrupt service routine must traverse the entire page table, 
;clearing the reference bits of each PTE. 
;You may assume when writing this code that the start address of the page table is fixed.


.orig x1200
	;Store user values
	STW R0,R6,#-1
	STW R1,R6,#-2
	STW R2,R6,#-3
	STW R3,R6,#-4
	STW R4,R4,#-5

	;Start ISR
	LEA R0,A
	LDW R1,R0,#1
	LDW R2,R0,#2
	LDW R0,R0,#0

B	LDW R3,R1,#0
	AND R3,R3,R2
	STW R3,R1,#0
	ADD R1,R1,#-2
	ADD R4,R1,R0
	BRzp B

	;Restore user values

	LDW R0,R6,#-1
	LDW R1,R6,#-2
	LDW R2,R6,#-3
	LDW R3,R6,#-4
	LDW R4,R6,#-5

	RTI


A 	.FILL xf000 	;start of page table (R0)
	.FILL x10FE		;end of page table   (R1)
	.FILL xFFFE		;mask				 (R2)

.END
