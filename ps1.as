; file ps1.as

.entry ENTRY
.extern fn5
ENTRY:
        add r4, ITEMS
        jsr fn5
LOOP4:
        prn #55
        cmp r4, #-2
        bne BRANCH4
        dec r4
BRANCH4:
        lea ALPHA, r6
        inc r6
        mov *r6, L4
        sub r5, r7
        add r3, r7
.entry ENTRY
        jmp LOOP4
END4:
        stop
ALPHA:
        .string "ijkl"
ITEMS:
        .data 2, -1
        .data -9
L4:      
        .data 30
