#pragma once 

#include <stdio.h> 
#include "macros.h" 

/* Structure for assembler passes (we have two passes assembler) */
struct passes {
    int pass_number; /* Current pass number */
};

/* Initialize passes structure */
void initialize_passes(struct passes* passes);

/* Release memory used by passes */
void release_passes_memory(struct passes* passes);

/* First pass of assembler process */
int assembler_first_pass(
    struct passes* passes,
    FILE* fileas, /* Assembly file */
    FILE* fileam, /* Macro file */
    FILE* filewrong /* File for errors */
);

/* Second pass of assembler process */
void assembler_second_pass(
    struct passes* passes,
    FILE* fileas, /* Assembly file */
    FILE* fileent, /* Entry file */
    FILE* fileext, /* External file */
    FILE* fileob /* Object file */
);
