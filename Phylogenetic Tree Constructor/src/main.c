#include <stdio.h>
#include <stdlib.h>

#include "global.h"
#include "debug.h"

int main(int argc, char **argv)
{
    if(validargs(argc, argv)) {
        USAGE(*argv, EXIT_FAILURE);
    }
    if(global_options == HELP_OPTION) {
        USAGE(*argv, EXIT_SUCCESS);
    }
    if(read_distance_data(stdin) == -1) 
    {
        return EXIT_FAILURE;
    }
    if(global_options == 0) {
        if (build_taxonomy(stdout) == -1)
        {
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }
    else 
    {
        if(build_taxonomy(NULL) == -1)
        {
            return EXIT_FAILURE;
        }
        if (global_options == MATRIX_OPTION)
        {
            if (emit_distance_matrix(stdout) == -1)
            {
                return EXIT_FAILURE;
            }
            return EXIT_SUCCESS;
        }
        else if (global_options == NEWICK_OPTION)
        {
            if (emit_newick_format(stdout) == -1)
            {
                return EXIT_FAILURE;
            }
            return EXIT_SUCCESS;
        }
    }

    return EXIT_FAILURE; 
}

/*
 * Just a reminder: All non-main functions should
 * be in another file not named main.c
 */
