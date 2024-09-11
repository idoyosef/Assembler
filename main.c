#include <stdio.h>
#include <string.h>
#include "definitions.h"
#include "passes.h"

char file_name[TOTAL_LEN]; /* Buffer for input/output file names */

int main(int argc, char *argv[])
{
    const char *name;
    FILE *fileas;
    FILE *fileam;
    FILE *fileent;
    FILE *filewrong;
    FILE *fileext;
    FILE *fileob;
    int total_invalid;
    int del_entry;
    int del_extern;
    struct passes passes;

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <file-name>\n", argv[0]); /* Print how to use message */
        return (-1); /* Exit if incorrect number of arguments */
    }

    name = argv[1];
    initialize_passes(&passes); /* Initialize passes structure */
    strcpy(file_name, name); /* Set base file name */
    strcat(file_name, ".as"); /* Add .as extension */
    fileas = fopen(file_name, "r"); /* Open assembly file for reading */
    strcpy(file_name, name); /* Reset file name */
    strcat(file_name, ".am"); /* Add .am extension */
    fileam = fopen(file_name, "w+"); /* Open macro file for writing */

    filewrong = stderr; /* Set error output to stderr */
    del_entry = FALSE; /* Initialize entry deletion flag */
    del_extern = FALSE; /* Initialize extern deletion flag */

    total_invalid = assembler_first_pass(
        &passes,
        fileas,
        fileam,
        filewrong); /* Perform first assembler pass */

    if (total_invalid == 0)
    {
        fseek(fileam, 0, SEEK_SET); /* Reset file pointer for .am file */

        strcpy(file_name, name); /* Reset file name */
        strcat(file_name, ".ent"); /* Add .ent extension */
        fileent = fopen(file_name, "w+"); /* Open entry file for writing */

        strcpy(file_name, name); /* Reset file name */
        strcat(file_name, ".ext"); /* Add .ext extension */
        fileext = fopen(file_name, "w+"); /* Open extern file for writing */

        strcpy(file_name, name); /* Reset file name */
        strcat(file_name, ".ob"); /* Add .ob extension */
        fileob = fopen(file_name, "w+"); /* Open object file for writing */

        assembler_second_pass(
            &passes,
            fileam,
            fileent,
            fileext,
            fileob); /* Perform second assembler pass */

        if (ftell(fileent) == 0)
        {
            del_entry = TRUE; /* Mark entry file for deletion if empty */
        }

        if (ftell(fileext) == 0)
        {
            del_extern = TRUE; /* Mark extern file for deletion if empty */
        }

        fclose(fileent); /* Close entry file */
        fclose(fileext); /* Close extern file */
        fclose(fileob); /* Close object file */
    }

    fclose(fileam); /* Close macro file */
    fclose(fileas); /* Close assembly file */

    release_passes_memory(&passes); /* Free memory used by passes */

    if (del_entry)
    {
        strcpy(file_name, name); /* Reset file name */
        strcat(file_name, ".ent"); /* Add .ent extension */
        remove(file_name); /* Remove entry file */
    }

    if (del_extern)
    {
        strcpy(file_name, name); /* Reset file name */
        strcat(file_name, ".ext"); /* Add .ext extension */
        remove(file_name); /* Remove extern file */
    }

    return 0; /* Exit successfully */
}
