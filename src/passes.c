#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "definitions.h"
#include "passes.h"
#include "tokens.h"

static char line_buffer[TOTAL_LEN]; /* Buffer for current line */
static int current_line_number; /* Line number tracker */
static int total_functions; /* Total functions count */
static int total_code_lines; /* Total code lines count */
static int total_data_lines; /* Total data lines count */
static int total_errors_found; /* Total errors found */

#define GROUP0 (0) /* Group 0 for instruction types */
#define GROUP1_CODE (1) /* Group 1 for code instructions */
#define GROUP2_DATA (2) /* Group 2 for data instructions */

#define NO_OPERANDS_GROUP (0) /* No operands */
#define ONE_OPERAND_GROUP (1) /* One operand */
#define TWO_OPERANDS_GROUP (2) /* Two operands */

#define IMMEDIATE_GROUP_OPERAND (1) /* Immediate operand */
#define DIR_GROUP_OPERAND (2) /* Direct operand */
#define INDIR_GROUP_OPERAND (4) /* Indirect operand */
#define REGISTER_GROUP_OPERAND (8) /* Register operand */

#define FIRST_OPERAND (0) /* First operand index */
#define SECOND_OPERAND (1) /* Second operand index */

#define ABSOLUTE_FLAG (0x4) /* Absolute address flag */
#define RELOCATABLE_FLAG (0x2) /* Relocatable address flag */
#define EXTERNAL_FLAG (0x1) /* External address flag */

#define MAX_GUIDE_DIRECTIVES (4) /* Maximum guide directives count */

const char *guide_directives[] = {
    ".data",
    ".entry",
    ".extern",
    ".string"};

struct LabelStruct
{
    char name[TOTAL_LEN]; /* Label name */
    int address; /* Label address */
};

struct LabelStruct *labels; /* Array of labels */
int total_labels; /* Total number of labels */
int max_labels; /* Maximum number of labels */

struct LabelStruct *val_arr; /* Array for entries */
int total_input; /* Total number of entries */
int limit; /* Limit of entries */

struct instruction
{
    const char *name; /* Instruction name */
    int command_opcode; /* Opcode for instruction */
    int group; /* Group for instruction */
};

#define TOTAL_COMMANDS (16) /* Total number of commands */

const struct instruction valid_commands_sizes[] = {
    {"mov", 0, 2},
    {"cmp", 1, 2},
    {"add", 2, 2},
    {"sub", 3, 2},
    {"lea", 4, 2},
    {"clr", 5, 1},
    {"not", 6, 1},
    {"inc", 7, 1},
    {"dec", 8, 1},
    {"jmp", 9, 1},
    {"bne", 10, 1},
    {"red", 11, 1},
    {"prn", 12, 1},
    {"jsr", 13, 1},
    {"rts", 14, 0},
    {"stop", 15, 0}};

#define MAX_RESERVED_WORD (20) /* Max number of reserved words */

const char *valid_commands[] = {
    ".data",
    ".entry",
    ".extern",
    ".string",
    "mov",
    "cmp",
    "add",
    "sub",
    "lea",
    "clr",
    "not",
    "inc",
    "dec",
    "jmp",
    "bne",
    "red",
    "prn",
    "jsr",
    "rts",
    "stop"};

void initialize_passes(struct passes *passes)
{
    val_arr = NULL; /* Initialize entry array */
    total_input = 0; /* Reset total entries */
    limit = 0; /* Reset entry limit */

    labels = NULL; /* Initialize labels array */
    total_labels = 0; /* Reset total labels */
    max_labels = 0; /* Reset max labels */
}

void release_passes_memory(struct passes *passes)
{
    free(val_arr); /* Free entry array memory */
    free(labels); /* Free labels memory */
}

static void insert_entry(
    struct passes *passes, 
    const char *name
)
{
    int id;
    struct LabelStruct *entry; /* Pointer to new entry */

    for (id = 0; id < total_input; id++)
    {
        if (strcmp(val_arr[id].name, name) == 0)
        {
            return; /* Entry already exists */
        }
    }

    if (total_input == limit)
    {
        limit += MEMORY_BLOCK_SIZE; /* Increase limit */
        val_arr = realloc(val_arr, sizeof(*val_arr) * limit); /* Resize entry array */
    }

    entry = &val_arr[total_input]; /* Point to new entry */
    strcpy(entry->name, name); /* Copy entry name */
    entry->address = 0; /* Initialize address */

    total_input++; /* Increment total entries */
}

static int insert_label(
    struct passes *passes, 
    const char *name
)
{
    int id;
    struct LabelStruct *LabelStruct; /* Pointer to new label */

    for (id = 0; id < total_labels; id++)
    {
        if (strcmp(labels[id].name, name) == 0)
        {
            return FALSE; /* Label already exists */
        }
    }

    if (total_labels == max_labels)
    {
        max_labels += MEMORY_BLOCK_SIZE; /* Increase max labels */
        labels = realloc(labels, sizeof(*labels) * max_labels); /* Resize labels array */
    }

    LabelStruct = &labels[total_labels]; /* Point to new label */
    strcpy(LabelStruct->name, name); /* Copy label name */
    LabelStruct->address = total_functions; /* Set label address */

    total_labels++; /* Increment total labels */
    return TRUE; /* Label inserted successfully */
}

static int confirm_label(const char *name)
{
    int id;

    for (id = 0; id < TOTAL_COMMANDS; id++)
    {
        if (strcmp(valid_commands_sizes[id].name, name) == 0)
        {
            return FALSE; /* Name matches a command */
        }
    }

    for (id = 0; id < MAX_GUIDE_DIRECTIVES; id++)
    {
        if (strcmp(guide_directives[id], name) == 0)
        {
            return FALSE; /* Name matches a directive */
        }
    }

    return TRUE; /* Name is not a valid command or directive */
}

static int confirm_guide_keyword(
    struct passes *passes, 
    const char *line,            
    int index_base,              
    int total_words_in_row       
)
{
    char *word;
    word = get_word_number(line, index_base);

    /* Check for data section directive */
    if (strcmp(word, DIRECTIVE_DATA_SECTION) == 0)
    {
        /* Ensure even number of words for data section */
        if (total_words_in_row % 2 != 0)
        {
            return FALSE;
        }

        /* Validate comma separators */
        while (TRUE)
        {
            total_words_in_row -= 2;
            if (total_words_in_row <= 0)
            {
                break;
            }

            word = get_word_number(line, index_base + 2);
            
            if (strcmp(word, SEPARATOR_COMMA) != 0)
            {
                return FALSE;
            }

            index_base += 2;
        }

        return TRUE; 
    }

    /* Check for entry point directive */
    if (strcmp(word, DIRECTIVE_ENTRY_POINT) == 0)
    {
        /* Ensure exactly two words for entry point */
        if (total_words_in_row != 2)
        {
            return FALSE;
        }

        word = get_word_number(line, index_base + 1);
        insert_entry(passes, word);

        return TRUE; 
    }

    /* Check for external reference directive */
    if (strcmp(word, DIRECTIVE_EXTERNAL_REFERENCE) == 0)
    {
        /* Ensure exactly two words for external reference */
        if (total_words_in_row != 2)
        {
            return FALSE;
        }

        return TRUE; 
    }

    /* Check for string literal directive */
    if (strcmp(word, DIRECTIVE_STRING_LITERAL) == 0)
    {
        /* Ensure exactly two words and valid string format */
        if (total_words_in_row != 2)
        {
            return FALSE;
        }

        word = get_word_number(line, index_base + 1);
        if (word[0] != '"' || word[strlen(word) - 1] != '"')
        {
            return FALSE;
        }

        return TRUE; 
    }

    return FALSE; 
}


static int allocate_op_group(const char *operand)
{
    char letter;

    letter = operand[0];

    /* Determine operand group based on the first character */
    if (letter == 'r')
    {
        return REGISTER_GROUP_OPERAND;
    }

    if (letter == '#')
    {
        return IMMEDIATE_GROUP_OPERAND;
    }

    if (letter == '*')
    {
        return INDIR_GROUP_OPERAND;
    }

    return DIR_GROUP_OPERAND; 
}


static int confirm_command(
    struct passes *passes, 
    const char *line,            
    int index_base,              
    int token_counter            
)
{
    char *word;

    int id;

    const struct instruction *instruction; 

    instruction = NULL;
    word = get_word_number(line, index_base);

    /* Check if the command is valid */
    for (id = 0; id < TOTAL_COMMANDS; id++)
    {
        if (strcmp(valid_commands_sizes[id].name, word) == 0)
        {
            instruction = &valid_commands_sizes[id];
            break; 
        }
    }

    if (instruction == NULL)
    {
        return FALSE;
    }

    /* Validate number of tokens for the command */
    switch (instruction->group)
    {
    case NO_OPERANDS_GROUP:
    {
        /* No operands should be present */
        if (token_counter != 1)
        {
            return FALSE;
        }
        break;
    }
    case ONE_OPERAND_GROUP:
    {
        /* One operand should be present */
        if (token_counter != 2)
        {
            return FALSE;
        }
        break;
    }
    case TWO_OPERANDS_GROUP:
    {
        /* Two operands and a comma should be present */
        if (token_counter != 4)
        {
            return FALSE;
        }
        
        word = get_word_number(line, index_base + 2);
        if (strcmp(SEPARATOR_COMMA, word) != 0)
        {
            return FALSE;
        }
        break;
    }
    }

    return TRUE; 
}


static int check_guide_length(
    struct passes *passes, 
    const char *line,            
    int index_base,              
    int total_words_in_row       
)
{
    char *word;

    int totalen;

    totalen = 0;
    word = get_word_number(line, index_base);

    /* Calculate length for data section or string literal */
    if (strcmp(word, DIRECTIVE_DATA_SECTION) == 0)
    {
        totalen = total_words_in_row / 2;
    }

    if (strcmp(word, DIRECTIVE_STRING_LITERAL) == 0)
    {
        word = get_word_number(line, index_base + 1);
        totalen = strlen(word) - 1; 
    }

    return totalen; 
}

static int check_command_length(
    struct passes *passes, 
    const char *line,            
    int index_base,              
    int counts                   
)
{
    char *word;
    int id;
    int totalen;
    const struct instruction *instruction; 

    totalen = 0;
    word = get_word_number(line, index_base);

    instruction = NULL;

    /* Find the instruction based on the command */
    for (id = 0; id < TOTAL_COMMANDS; id++)
    {
        if (strcmp(valid_commands_sizes[id].name, word) == 0)
        {
            instruction = &valid_commands_sizes[id];
            break; 
        }
    }

    if (instruction != NULL)
    {
        /* Determine length based on the instruction group */
        switch (instruction->group)
        {
        case NO_OPERANDS_GROUP:
        {
            /* Validate if only one token is present */
            if (counts == 1)
            {
                totalen = 1;
            }
            break;
        }
        case ONE_OPERAND_GROUP:
        {
            /* Validate if two tokens are present */
            if (counts == 2)
            {
                totalen = 2;
            }
            break;
        }
        case TWO_OPERANDS_GROUP:
        {
            /* Validate if four tokens are present and check operands */
            if (counts == 4)
            {
                char *token_a;
                char *token_b;
                int test_group1;
                int test_indirect;
                int test_operand;
                int test_group2;
                int test_indirect_group2;
                int test_operand_group2;

                totalen = 1;

                token_a = get_word_number(line_buffer, index_base + 1);
                test_indirect = token_a[0] == 'r';
                test_operand = token_a[0] == '*' && token_a[1] == 'r';
                test_group1 = test_indirect || test_operand;

                token_b = get_word_number(line_buffer, index_base + 3);
                test_indirect_group2 = token_b[0] == 'r';
                test_operand_group2 = token_b[0] == '*' && token_b[1] == 'r';
                test_group2 = test_indirect_group2 || test_operand_group2;

                if (test_group1 && test_group2)
                {
                    totalen += 1;
                }
                else
                {
                    totalen += 2;
                }
            }
            break;
        }
        }
    }

    return totalen; 
}


static void process_one_line(
    struct passes *passes, 
    const char *line,            
    FILE *assembly_file_error    
)
{
    char *word;
    int group;
    int total_words;
    int index_base;
    int totalen;
    int lbl_diff;
    int lbl_correct;

    index_base = 0;
    total_words = total_words_in_row(line);

    /* Process labels in the line */
    while (total_words >= 2)
    {
        word = get_word_number(line, index_base + 1);

        if (strcmp(word, LABEL_DEFINITION_SEPARATOR) == 0)
        {
            word = get_word_number(line, index_base);
            total_words -= 2;
            index_base += 2;

            /* Check for label errors */
            lbl_correct = confirm_label(word);
            if (lbl_correct == FALSE)
            {
                fprintf(
                    assembly_file_error,
                    "There is an error in line number%d: duplicate labels defined \"%s\"\n",
                    current_line_number,
                    word);
                total_errors_found++;
                continue;
            }
            lbl_diff = insert_label(passes, word);
            if (lbl_diff == FALSE)
            {
                fprintf(
                    assembly_file_error,
                    "There is an error in line number%d: duplicate labels defined \"%s\"\n",
                    current_line_number,
                    word);
                total_errors_found++;
                continue;
            }
        }
        else
        {
            break;
        }
    }

    if (total_words == 0)
    {
        return;
    }

    group = GROUP0;

    /* Check if the command or guide keyword is valid */
    if (confirm_command(passes, line, index_base, total_words))
    {
        group = GROUP1_CODE;
    }

    if (confirm_guide_keyword(passes, line, index_base, total_words))
    {
        group = GROUP2_DATA;
    }

    totalen = 0;

    /* Update counts based on the group type */
    switch (group)
    {
    case GROUP1_CODE:
    {
        totalen = check_command_length(passes, line, index_base, total_words);
        total_code_lines += totalen;
        break;
    }
    case GROUP2_DATA:
    {
        totalen = check_guide_length(passes, line, index_base, total_words);
        total_data_lines += totalen;
        break;
    }
    default:
    {
        fprintf(
            assembly_file_error,
            "There is an error in line number%d: invalid syntax detected\n",
            current_line_number);
        total_errors_found++;
        break;
    }
    }

    total_functions += totalen; 
}

int assembler_first_pass(
    struct passes *passes, 
    FILE *assembly_fileas,   
    FILE *assembly_file_output,  
    FILE *assembly_file_error    
)
{
    int id;
    int saved_keyword;
    char *word;
    int total_words;
    struct Macro *currently_in_macro_block;      
    struct Macro *macroPtr;           
    struct MacrosList macros; 

    current_line_number = 0;
    total_code_lines = 0;
    total_data_lines = 0;
    total_errors_found = 0;
    currently_in_macro_block = NULL;

    MacrosList_init(&macros); /* Initialize macro list */
    total_functions = 100; 

    while (fgets(line_buffer, TOTAL_LEN, assembly_fileas) != NULL)
    {
        current_line_number++; /* Increment line number */
        total_words = total_words_in_row(line_buffer); /* Count words in line */

        if (total_words == 1)
        {
            word = get_word_number(line_buffer, 0);
            
            if (strcmp(word, MACRO_TERMINATION) == 0)
            {
                if (currently_in_macro_block == NULL)
                {
                    /* Error: endmacro outside macro */
                    fprintf(
                        assembly_file_error,
                        "There is an error in line number%d: \"endmacro keyword\" is outside the macro\n",
                        current_line_number);
                    total_errors_found++;
                    continue;
                }
                currently_in_macro_block = NULL; /* End macro block */
                continue;
            }

            saved_keyword = FALSE;

            for (id = 0; id < MAX_RESERVED_WORD; id++)
            {
                if (strcmp(valid_commands[id], word) == 0)
                {
                    saved_keyword = TRUE;
                    break;
                }
            }

            if (!saved_keyword)
            {
                macroPtr = MacrosList_find(&macros, word);

                if (macroPtr == NULL)
                {
                    /* Error: undefined macro usage */
                    fprintf(
                        assembly_file_error,
                        "There is an error in line number%d: undefined macro usage \"%s\"\n",
                        current_line_number,
                        word);
                    total_errors_found++;
                    continue;
                }

                for (id = 0; id < macroPtr->counter_line; id++)
                {
                    process_one_line(passes, macroPtr->lines[id], assembly_file_error);
                    fputs(macroPtr->lines[id], assembly_file_output); 
                }
                continue;
            }
        }

        if (total_words == 2)
        {
            word = get_word_number(line_buffer, 0);
            
            if (strcmp(word, MACRO_DEFINITION) == 0)
            {
                word = get_word_number(line_buffer, 1);
                currently_in_macro_block = MacrosList_register(&macros, word); 

                if (currently_in_macro_block == NULL)
                {
                    /* Error: duplicate macro name */
                    fprintf(
                        assembly_file_error,
                        "There is an error in line number%d: duplicate macro name definition \"%s\"\n",
                        current_line_number,
                        word);
                    total_errors_found++;
                    continue;
                }
                continue;
            }
        }

        if (currently_in_macro_block == NULL)
        {
            process_one_line(passes, line_buffer, assembly_file_error); 
            fputs(line_buffer, assembly_file_output);                                 
        }
        else
        {
            Macro_append(currently_in_macro_block, line_buffer); 
        }
    }

    MacrosList_free(&macros); /* Free macro list */

    return total_errors_found; 
}

static void generate_objects_output(
    struct passes *passes, 
    FILE *output_file_pointer,   
    int counter,                 
    int value                    
)
{
    /* Print formatted output to file */
    fprintf(
        output_file_pointer,
        "%d %05o\n", 
        counter,
        value & 0x7FFF 
    );
}

static int parse_register_value(const char *operand)
{
    int register_val;
    register_val = (-1); 

    /* Check if operand starts with 'r' */
    if (operand[0] == 'r')
    {
        sscanf(&operand[1], "%d", &register_val);
    }

    /* Check if operand starts with '*' */
    if (operand[0] == '*')
    {
        sscanf(&operand[2], "%d", &register_val);
    }

    return register_val; 
}

static void out_object_operand_file(
    struct passes *passes, 
    FILE *fileext,       
    FILE *output_file_pointer,   
    int counter,                 
    const char *operand,         
    int order                    
)
{
    int group;
    int imd_value;
    int id;
    int register_val;
    int value;

    group = allocate_op_group(operand); /* Determine operand group */

    switch (group)
    {
    case IMMEDIATE_GROUP_OPERAND:
    { 
        sscanf(&operand[1], "%d", &imd_value); /* Read immediate value */
        value = (imd_value << 3) | ABSOLUTE_FLAG; /* Prepare value */
        generate_objects_output(
            passes,
            output_file_pointer,
            counter,
            value);
        total_functions++; /* Increment function count */
        break;
    }
    case INDIR_GROUP_OPERAND: 
    case REGISTER_GROUP_OPERAND:
    { 
        register_val = parse_register_value(operand); /* Parse register value */
        value = ABSOLUTE_FLAG;
        switch (order)
        {
        case FIRST_OPERAND:
        {                                                  
            value = value | (register_val << 6); /* Set value for first operand */
            break;
        }
        case SECOND_OPERAND:
        {                                                  
            value = value | (register_val << 3); /* Set value for second operand */
            break;
        }
        }
        generate_objects_output(
            passes,
            output_file_pointer,
            counter,
            value);
        total_functions++; /* Increment function count */
        break;
    }
    case DIR_GROUP_OPERAND:
    { 
        struct LabelStruct *LabelStruct;
        LabelStruct = NULL;
        for (id = 0; id < total_labels; id++)
        {
            if (strcmp(labels[id].name, operand) == 0)
            {
                LabelStruct = &labels[id]; /* Find label */
                break;
            }
        }
        if (LabelStruct == NULL)
        {                    
            value = EXTERNAL_FLAG; 
            generate_objects_output(
                passes,
                output_file_pointer,
                counter,
                value);
            fprintf(
                fileext,
                "%s %04d\n",
                operand,
                counter); /* Write external label */
            total_functions++; /* Increment function count */
        }
        else
        {                                               
            value = (LabelStruct->address << 3) | RELOCATABLE_FLAG; 
            generate_objects_output(
                passes,
                output_file_pointer,
                counter,
                value);
            total_functions++; /* Increment function count */
        }
        break;
    }
    }
}

static void generate_guides_output(
    struct passes *passes, 
    const char *line,            
    int index_base,              
    int total_words,            
    FILE *output_file_pointer    
)
{
    char *word; 
    int id;   
    int limit;   
    int value;   

    word = get_word_number(line, index_base); /* Get directive */

    if (strcmp(word, DIRECTIVE_DATA_SECTION) == 0)
    { 
        while (total_words > 0)
        {                                                          
            word = get_word_number(line, index_base + 1); /* Get data word */

            value = 0;                   
            sscanf(word, "%d", &value); /* Convert word to integer */

            generate_objects_output(
                passes,
                output_file_pointer,
                total_functions,
                value);

            total_functions++; /* Increment function count */

            index_base += 2;   /* Move to next word */
            total_words -= 2; /* Decrease word count */
        }
    }

    if (strcmp(word, DIRECTIVE_STRING_LITERAL) == 0)
    {                                                          
        word = get_word_number(line, index_base + 1); /* Get string literal */

        limit = strlen(word) - 1; 
        for (id = 1; id < limit; id++)
        {                               
            char letter = word[id]; 

            value = letter & 0xFF; /* Convert char to value */

            generate_objects_output(
                passes,
                output_file_pointer,
                total_functions,
                value);

            total_functions++; /* Increment function count */
        }

        generate_objects_output(
            passes,
            output_file_pointer,
            total_functions,
            0); /* Add null terminator */

        total_functions++; /* Increment function count */
    }
}

static void generate_commands_output(
    struct passes *passes, 
    const char *line,            
    int index_base,              
    int total_words,            
    FILE *fileext,       
    FILE *output_file_pointer    
)
{
    char *word;          
    int id;            
    int value;            
    int category_operand; 

    const struct instruction *instruction; 

    word = get_word_number(line, index_base); /* Get command */
    instruction = NULL;                                

    for (id = 0; id < TOTAL_COMMANDS; id++)
    { 
        if (strcmp(valid_commands_sizes[id].name, word) == 0)
        {                                        
            instruction = &valid_commands_sizes[id]; /* Find command instruction */
            break;                               
        }
    }

    if (instruction != NULL)
    { 
        switch (instruction->group)
        { 
        case NO_OPERANDS_GROUP:
        {                                                        
            value = (instruction->command_opcode << 11) | ABSOLUTE_FLAG; 
            generate_objects_output(                                 
                                passes,
                                output_file_pointer,
                                total_functions,
                                value);
            total_functions++; /* Increment function count */
            break;
        }

        case ONE_OPERAND_GROUP:
        {                                                          
            value = (instruction->command_opcode << 11) | ABSOLUTE_FLAG;   
            word = get_word_number(line, index_base + 1); /* Get operand */
            category_operand = allocate_op_group(word);  

            value = value | (category_operand << 3); 

            generate_objects_output(
                                passes,
                                output_file_pointer,
                                total_functions,
                                value);
            total_functions++; /* Increment function count */

            out_object_operand_file(
                                    passes,
                                    fileext,
                                    output_file_pointer,
                                    total_functions,
                                    word,
                                    SECOND_OPERAND); /* Process operand */

            break;
        }

        case TWO_OPERANDS_GROUP:
        {                   
            int group1; 
            int group2; 
            int test_group1;     
            int test_indirect;    
            int test_operand;    
            int test_group2;     
            int test_indirect_group2;    
            int test_operand_group2;    

            word = get_word_number(line, index_base + 1); /* Get first operand */
            group1 = allocate_op_group(word);        

            word = get_word_number(line, index_base + 3); /* Get second operand */
            group2 = allocate_op_group(word);        

            value = (instruction->command_opcode << 11) | ABSOLUTE_FLAG; 
            value = value | (group1 << 7);                   
            value = value | (group2 << 3);                   

            generate_objects_output(
                                passes,
                                output_file_pointer,
                                total_functions,
                                value);
            total_functions++; /* Increment function count */

            test_indirect = group1 == INDIR_GROUP_OPERAND;    
            test_operand = group1 == REGISTER_GROUP_OPERAND; 
            test_group1 = test_indirect || test_operand;                   

            test_indirect_group2 = group2 == INDIR_GROUP_OPERAND;    
            test_operand_group2 = group2 == REGISTER_GROUP_OPERAND; 
            test_group2 = test_indirect_group2 || test_operand_group2;                   

            if (test_group1 && test_group2)
            {                          
                int number_register_1; 
                int number_register_2; 

                word = get_word_number(line, index_base + 1); /* Get register 1 */
                number_register_1 = parse_register_value(word);    

                word = get_word_number(line, index_base + 3); /* Get register 2 */
                number_register_2 = parse_register_value(word);    

                value = ABSOLUTE_FLAG;                          
                value = value | (number_register_1 << 6); 
                value = value | (number_register_2 << 3); 

                generate_objects_output(
                                    passes,
                                    output_file_pointer,
                                    total_functions,
                                    value);
                total_functions++; /* Increment function count */
            }
            else
            {                                                          
                word = get_word_number(line, index_base + 1); /* Process first operand */
                out_object_operand_file(                               
                                        passes,
                                        fileext,
                                        output_file_pointer,
                                        total_functions,
                                        word,
                                        FIRST_OPERAND);

                word = get_word_number(line, index_base + 3); /* Process second operand */
                out_object_operand_file(                               
                                        passes,
                                        fileext,
                                        output_file_pointer,
                                        total_functions,
                                        word,
                                        SECOND_OPERAND);
            }

            break;
        }
        }
    }
}

static void second_phase_process_line(
    struct passes *passes, 
    const char *line,            
    FILE *fileext,       
    FILE *output_file_pointer    
)
{
    char *word; 

    int group;     
    int total_words; 
    int index_base;   

    index_base = 0;                          
    total_words = total_words_in_row(line); /* Count total words in the line */

    while (total_words >= 2)
    {                                                          
        word = get_word_number(line, index_base + 1); /* Get next word */
        if (strcmp(word, LABEL_DEFINITION_SEPARATOR) == 0)
        {                      
            total_words -= 2; /* Decrease words count and adjust base index */
            index_base += 2;   
        }
        else
        { 
            break;
        }
    }

    if (total_words == 0)
    { 
        return; /* Exit if no words left */
    }

    group = GROUP0; 

    if (confirm_command(passes, line, index_base, total_words))
    {                         
        group = GROUP1_CODE; /* Set group for code commands */
    }

    if (confirm_guide_keyword(passes, line, index_base, total_words))
    {                         
        group = GROUP2_DATA; /* Set group for data guides */
    }

    switch (group)
    { 
    case GROUP1_CODE:
    {                                
        generate_commands_output(
                                     passes,
                                     line,
                                     index_base,
                                     total_words,
                                     fileext,
                                     output_file_pointer); /* Process command output */
        break;
    }
    case GROUP2_DATA:
    {                          
        generate_guides_output(
                               passes,
                               line,
                               index_base,
                               total_words,
                               output_file_pointer); /* Process guide output */
        break;
    }
    }
}

void assembler_second_pass(
    struct passes *passes, 
    FILE *assembly_fileas,   
    FILE *fileent,       
    FILE *fileext,       
    FILE *output_file_pointer    
)
{
    int id;   
    int index_a; 
    int index_b; 

    struct LabelStruct *entry; 
    struct LabelStruct *LabelStruct; 

    total_functions = 100; /* Initialize function count */

    fprintf(
            output_file_pointer,
            "%d %d\n",
            total_code_lines,
            total_data_lines); /* Write totals to output file */

    while (fgets(line_buffer, TOTAL_LEN, assembly_fileas) != NULL)
    {                                
        second_phase_process_line(
                                     passes,
                                     line_buffer,
                                     fileext,
                                     output_file_pointer); /* Process each line */
    }

    for (index_a = 0; index_a < total_input; index_a++)
    {                              
        entry = &val_arr[index_a]; 
        for (index_b = 0; index_b < total_labels; index_b++)
        {                              
            LabelStruct = &labels[index_b]; 
            if (strcmp(entry->name, LabelStruct->name) == 0)
            {                                          
                entry->address = LabelStruct->address; /* Update entry address */
                break;                                 
            }
        }
    }

    while (TRUE)
    {              
        int swaps; 
        swaps = 0; 
        for (id = 1; id < total_input; id++)
        {                                        
            struct LabelStruct temp_storage_a;         
            struct LabelStruct temp_storage_b;         
            temp_storage_a = val_arr[id - 1]; 
            temp_storage_b = val_arr[id - 0]; 
            if (temp_storage_a.address > temp_storage_b.address)
            {                                        
                val_arr[id - 1] = temp_storage_b; 
                val_arr[id - 0] = temp_storage_a; 
                swaps++; /* Count swaps for sorting */
            }
        }
        if (swaps == 0)
        { 
            break; /* Exit if no swaps needed */
        }
    }

    for (id = 0; id < total_input; id++)
    {                            
        entry = &val_arr[id]; 
        fprintf(                 
                fileent,
                "%s %d\n",
                entry->name,
                entry->address); /* Write sorted labels to file */
    }
}

