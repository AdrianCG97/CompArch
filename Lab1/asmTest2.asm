.ORIG   x3000
        LEA R1, SIX
        LDW R1, R1, x0
        LEA R2, NUM
        LDW R2, R2, x0
        AND R3, R3, x0

AGAIN   ADD R3, R3, R2
        ADD R1, R1, x-1
        BRP AGAIN

        HALT

NUM     .FILL 0x0009
SIX     .FILL 0x0006

        .END

