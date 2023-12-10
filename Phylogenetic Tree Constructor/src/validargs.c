#include <stdlib.h>

#include "global.h"
#include "debug.h"

/**
 * @brief Validates command line arguments passed to the program.
 * @details This function will validate all the arguments passed to the
 * program, returning 0 if validation succeeds and -1 if validation fails.
 * Upon successful return, the various options that were specified will be
 * encoded in the global variable 'global_options', where it will be
 * accessible elsewhere in the program.  For details of the required
 * encoding, see the assignment handout.
 *
 * @param argc The number of arguments passed to the program from the CLI.
 * @param argv The argument strings passed to the program from the CLI.
 * @return 0 if validation succeeds and -1 if validation fails.
 * @modifies global variable "global_options" to contain an encoded representation
 * of the selected program options.
 */
static int strcmp(char *a, char *b) {   //function to compare two char*. Returns (+) if larger 0 if same (-) if smaller
    while (*a && *b) {
        if(*a != *b)
            return (*a - *b);
        a++;
        b++;
    }
    return (*a - *b);
}

int validargs(int argc, char **argv) {
    if(argc == 1)
        return 0;

    for (char **ptr = argv + 1; *ptr != NULL; ptr++)    //for loop through command line
    {  
        if(strcmp(*ptr, "-h") == 0) //compare -h
        {   
            if(ptr - argv != 1 || (global_options & HELP_OPTION) == HELP_OPTION) {
                return -1;
            } else {
                global_options |= HELP_OPTION;
                return 0;
            }
        
        } else if(strcmp(*ptr, "-m") == 0)  //compare -m
        {    
            if((global_options & (MATRIX_OPTION | NEWICK_OPTION)) != 0) {return -1;}
            global_options |= MATRIX_OPTION;

        } else if(strcmp(*ptr, "-n") == 0)  //compare -n
        {    
            if((global_options & (MATRIX_OPTION | NEWICK_OPTION)) != 0) {return -1;}
            global_options |= NEWICK_OPTION;
            outlier_name = NULL;

        } else if(strcmp(*ptr, "-o") == 0)  //compare -o
        {    
            if((global_options & NEWICK_OPTION) == NEWICK_OPTION) {
                ptr = ptr + 1;  //ptr to next arg in CL
                if (*ptr != NULL) {
                    outlier_name = *ptr;    //set outlier_name = to ptr
                    if (strcmp(outlier_name, "-n") == 0) {  //if outlier_name is duplicate of flag -> error
                        return -1;
                    } else if (strcmp(outlier_name, "-m") == 0) {
                        return -1;
                    } else if (strcmp(outlier_name, "-o") == 0) {
                        return -1;
                    } else if (strcmp(outlier_name, "-h") == 0) {
                        return -1;
                    }
                } else {return -1;}
            } else {return -1;}
        } else {return -1;}

    }
    return 0;
    abort();
}


    

