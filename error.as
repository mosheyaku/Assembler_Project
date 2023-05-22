; file error.as

.entry LENGTH
.extern W
MAIN:   mov   r3, LENGTH
LOOP:
        abcdef                         ; error
LABEL_A:  abcdef                       ; error
        jmp   L1(#-1, r6)
        jmp   prn(#-1, r6)             ; error
LABEL_B: jmp   prn(#-1, r6)            ; error
        prn   #-5
        bne   W(r4,r5)
        sub   r3,r4
        bne   L3
L1:
        inc   K
.entry LOOP
        bne   LOOP(K, W)
END:
        stop
STR:
        .string "abcdef"
LENGTH:
        .data 6,-9,15
K:
        .data 22
.extern L3