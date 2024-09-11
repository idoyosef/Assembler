; file error1.as

.entry DATA
.extern func1
START:  mov r2, DATA
        jsr func1, func2
LOOP1:  prn #24
        macro my_macro1
                add r5, r2, r3
                bne EXIT
        endmacro
        lea MSG, r3, r4
        inc r3, r5
        mov *r3, L2, L3
        sub r4, r5, r2
        my_macro1
.entry START
        jmp LOOP1
EXIT:   stop
MSG:    .string "hello"
DATA:   .data 8, -5
        .data -50
L2:     .data 42
.extern L2
