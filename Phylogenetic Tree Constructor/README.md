Implemented a command-line program (called philo) whose purpose is to construct a phylogenetic tree from genetic distance data given as input. Using C programming, with a focus on input/output, bitwise manipulations, and the use of pointers.

This program handles command-line options:

- If the -h flag is provided, you will display the usage for the program and exit with an EXIT_SUCCESS return code.
  
- If the -h flag is not provided, then the program will proceed to read genetic distance data from standard input ( stdin ), construct a
  phylogenetic tree based on this data, and produce some output related to this tree on standard output ( stdout ). If the program is
  successful, it will exit with an EXIT_SUCCESS return code, otherwise, the program will exit with an EXIT_FAILURE return code. In the latter
  case, the program will print to standard error ( stderr ) an error message or message describing the error(s) that were discovered.

- If no arguments are provided, then the default behavior of the program is to output edge data for the synthesized tree on stdout.
  
- If the -m (matrix) flag is provided, then instead of the edge data, the program will output to stdout the matrix of the estimated node
  distances constructed during the run.
  
- If the -n (newick) flag is provided, then instead of the edge data, the program will output to stdout a representation, in "Newick" format,
  of the phylogenetic tree synthesized from the input data. As discussed below, the production of this output requires that an "outlier" node
  be selected to orient the edges of the tree. The -o option provides the user to specify the name of a leaf node to serve as the
  outlier. If this option is not provided, then the program uses a default strategy to choose an outlier.
