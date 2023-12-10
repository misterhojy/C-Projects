#include <stdlib.h>

#include "global.h"
#include "debug.h"

static int strcmp(char *a, char *b) {   //function to compare two char*. Returns (+) if larger 0 if same (-) if smaller
    char *c = a;
    char *d = b;
    while (*c && *d) 
    {
        if(*c != *d)
            return (*c - *d);
        c++;
        d++;
    }
    return (*c - *d);
}

/*There can be at most one leading zero, which if present must occur immediately before a decimal point.  
One of the sequences of digits must be nonempty; i.e. a decimal point standing alone is not permitted.
return: 1 if it is invalid 0 if it is valid;*/
static int invalid_input(char* input) {
    int comma_count = 0;
    int number_count = 0;
    char *ptr = input; //pointer at start address of input
    
    if (*ptr == '0' && *(ptr + 1) == '0') {
        return 1;
    } else if (*ptr == '.') {
        return 1;
    }
    
    ptr = input;
    while(*ptr != '\0') {
        if (*ptr == '.') {
            comma_count++;
        } 
        else if (*ptr >= '0' && *ptr <= '9') {
            number_count++;
        } else {
            return 1;
        }
        ptr++;
    }
    
    if (comma_count > 1 || number_count < 1) {
        return 1;
    } else {
        return 0;
    }
}

static int strlength(char* input) {
    int length = 0;
    while(*(input + length) != '\0')
    {
        length++;
    }
    return length;
}

static double ten_power(int power) {
    double result = 1.0;
    for (int i = 0; i < power; i++)
    {
        result *= 10.0;
    }
    return result;
}


static double string_to_double(char* input) {
    int length = strlength(input);
    double result = 0.0;
    int decimal_place = 0;

    for (int i = 0; i < length; i++)
    {   //how many decimals
        if (*(input + i) == '.')
        {
            decimal_place = length - i - 1;
            break;
        }
    }

    if (decimal_place > 0)
    {
        length--;
    }
    
    for (int i = 0, j = length - 1; j >= 0; i++, j--)
    {
        if(*(input + i) == '.')
        {
            i++;
        }
        double number = (*(input + i) - '0') * ten_power(j);
        result = result + number;
    }

    if (decimal_place > 0)
    {
        result = result / ten_power(decimal_place);
    }
    return result;  
}
    

/*function to calculate row sum of a matrix*/
static void sum_of_matrix() {
    for (int i = 0; i < num_active_nodes; i++)
    {
        *(row_sums + *(active_node_map + i)) = 0;
        for (int j = 0; j < num_active_nodes; j++)
        {
            *(row_sums + *(active_node_map + i)) += *(*(distances + *(active_node_map + i)) + *(active_node_map + j));
        }
    }
}

//function to make two nodes eachother neighbors storing in the correct index whether it is a child or parent
static void two_neighbors(NODE *parent, NODE *child) {
    int neighbor_limit = 3;

    NODE **ptr = parent->neighbors;
    for (int i = 1; i < neighbor_limit; i++) {
        if (*(ptr + i) == NULL) {
            *(ptr + i) = child;
            break;
        }
    }

    ptr = child->neighbors;
    
    if (*(ptr + 0) == NULL) {
        *(ptr + 0) = parent;
    }
}


//function to sum leaf distances for outlier
static void sum_leaf_distance() {
    for (int i = 0; i < num_taxa; i++)
    {
        *(row_sums + i) = 0;
        for (int j = 0; j < num_taxa; j++)
        {
            *(row_sums + i) += *(*(distances + i) + j);
        }
    }
}

//funciton to tell if it is a leaf node or not
//return 0 if it is internal and 1 if it is leaf
static int is_leaf(NODE* node) {
    int count = 0;
    for (int i = 0; i < 3; i++)
    {
        if (*(node->neighbors + i) != NULL)
        {
            count++;
        }
    }
    if (count == 1)
    {
        return 1;
    }
    return 0;
}

//Function given a name you find the index of it on the disance matrix return 0 if none
static int find_node_index(char* name) {
    int index = -1;

    for (int i = 0; i < num_all_nodes; i++)
    {
        if (strcmp(name, *(node_names + i)) == 0)
        {
            index = i;
            return index;  
        }
    }
    return index;
}

static int find_leaf_node_index(char* name) {
    int index = -1;

    for (int i = 0; i < num_taxa; i++)
    {
        if (strcmp(name, *(node_names + i)) == 0)
        {
            index = i;
            return index;  
        }
    }
    return index;
}

//Function to recursively print the tree
static void recursive_print(FILE* out, NODE* child, NODE* parent) {

    //finding indexes for distance data
    int parent_index = find_node_index(parent->name);
    int child_index = find_node_index(child->name);
    int comma_count = 0;
    if (num_taxa > 2)
    {
        fprintf(out, "(");  //opening PARA
    }
    for (int i = 0; i < 3; i++)
    {   //Loop of neighbors

        if ((*(parent->neighbors + i)) != NULL && strcmp(child->name, (*(parent->neighbors + i))->name) != 0)
        {   //can't be null and can't be outlier or a previously visited node

            if (is_leaf(*(parent->neighbors + i)))
            {   //if leaf print
                int leaf_index = find_node_index((*(parent->neighbors + i))->name);
                fprintf(out, "%s:%.2f",(*(parent->neighbors + i))->name, *(*(distances + leaf_index) + parent_index));
            }
            else
            {   //recursive call
                recursive_print(out, parent,*(parent->neighbors + i));
            }
            if (i < 3)
            {   //comma when it not last one unless we at outlier
                if (comma_count < 1)
                {
                    fprintf(out,",");
                    comma_count++;
                }
            }
        }
    }   //closing PARA and data
    if (num_taxa > 2)
    {
        fprintf(out, ")");  //opening PARA
    }
    fprintf(out,"%s:%.2f",parent->name, *(*(distances + child_index) + parent_index));
}

/**
 * @brief  Read genetic distance data and initialize data structures.
 * @details  This function reads genetic distance data from a specified
 * input stream, parses and validates it, and initializes internal data
 * structures.
 *
 * The input format is a simplified version of Comma Separated Values
 * (CSV).  Each line consists of text characters, terminated by a newline.
 * Lines that start with '#' are considered comments and are ignored.
 * Each non-comment line consists of a nonempty sequence of data fields;
 * each field is terminated either by ',' or else newline for the last
 * field on a line.  The constant INPUT_MAX specifies the maximum number
 * of data characters that may be in an input field; fields with more than
 * that many characters are regarded as invalid input and cause an error
 * return.  The first field of the first data line is empty;
 * the subsequent fields on that line specify names of "taxa", which comprise
 * the leaf nodes of a phylogenetic tree.  The total number N of taxa is
 * equal to the number of fields on the first data line, minus one (for the
 * blank first field).  Following the first data line are N additional lines.
 * Each of these lines has N+1 fields.  The first field is a taxon name,
 * which must match the name in the corresponding column of the first line.
 * The subsequent fields are numeric fields that specify N "distances"
 * between this taxon and the others.  Any additional lines of input following
 * the last data line are ignored.  The distance data must form a symmetric
 * matrix (i.e. D[i][j] == D[j][i]) with zeroes on the main diagonal
 * (i.e. D[i][i] == 0).
 *
 * If 0 is returned, indicating data successfully read, then upon return
 * the following global variables and data structures have been set:
 *   num_taxa - set to the number N of taxa, determined from the first data line
 *   num_all_nodes - initialized to be equal to num_taxa
 *   num_active_nodes - initialized to be equal to num_taxa
 *   node_names - the first N entries contain the N taxa names, as C strings
 *   distances - initialized to an NxN matrix of distance values, where each
 *     row of the matrix contains the distance data from one of the data lines
 *   nodes - the "name" fields of the first N entries have been initialized
 *     with pointers to the corresponding taxa names stored in the node_names
 *     array.
 *   active_node_map - initialized to the identity mapping on [0..N);
 *     that is, active_node_map[i] == i for 0 <= i < N.
 *
 * @param in  The input stream from which to read the data.
 * @return 0 in case the data was successfully read, otherwise -1
 * if there was any error.  Premature termination of the input data,
 * failure of each line to have the same number of fields, and distance
 * fields that are not in numeric format should cause a one-line error
 * message to be printed to stderr and -1 to be returned.
 */

int read_distance_data(FILE *in) {
    int c;
    int num_line = 0;
    num_taxa = 0;

    while ((c = fgetc(in)) != EOF) 
    {    //main loop until end of file
        if(c == '#') 
        {  //skip comments until \n
            while ((c = fgetc(in)) != '\n') {}
        } 
        else if(**node_names == '\0' && c == ',' && num_line == 0) 
        {   //if names not processed and see a comma then it is first line and needs to be processed
            num_line++;
            int in_name_line = 1;

            while (in_name_line) 
            {   //while we are in the name line
                c = fgetc(in);  //skipping ','

                for (int i = 0; i <= INPUT_MAX; i++, (c = fgetc(in)))   
                {   //i = col & max iteration = [input_max+1]; num_taxa = row
                    if (num_taxa > MAX_TAXA || num_taxa > MAX_NODES)   //if num taxa more than limit
                    {
                        fprintf(stderr, "Invalid: Too many taxa\n");
                        return -1;
                    }
                    if (i == INPUT_MAX && (c != ',' || c != '\n')) //if max iteration reached and not ',' or '\n' ERROR
                    { 
                        fprintf(stderr, "Invalid: Input is too large\n");
                        return -1;
                    }
                    if (c == ',' || c == '\n') 
                    {   //if ',' or '\n' end of field. '\n' signifies end of name line
                        if (c == '\n')
                            in_name_line = 0;

                        *(*(node_names + num_taxa) + i) = '\0';
                        (nodes + num_taxa)->name = *(node_names + num_taxa);

                        int count = 0;
                        while (count < num_taxa)
                        {   //checking for duplicate names comparing buffer with elements in node names
                            if (strcmp(*(node_names + num_taxa), *(node_names + count)) == 0)
                            {
                                fprintf(stderr, "Invalid: Duplicate taxa\n");
                                return -1;
                            }
                            count++;
                        }
                        num_taxa++;
                        break;
                    } 
                    else 
                    {   //add character in correct position in 2d Array
                        *(*(node_names + num_taxa) + i) = c;
                    }
                }
            }
            num_all_nodes = num_taxa;
            num_active_nodes = num_taxa;
        } 
        else
        {   //lines after the first name line
            int field = 0;

            while (num_line < num_taxa + 1)
            {   //end after reading N+1 lines

                if(c == '#') 
                {  //skip comments until \n
                    while ((c = fgetc(in)) != '\n') {}
                } 
                else if (c == ',' || c == '\n') 
                {   //ski[ the ',' and '\n'
                    c = fgetc(in);
                }

                if (c != '#' && c != ',' && c != '\n')
                {                
                    for (int i = 0; i <= INPUT_MAX; i++, (c = fgetc(in)))
                    {   //iterate throught each field while mainting count of fields iterated
                        if (i == INPUT_MAX && (c != ',' || c != '\n'))  //if max iteration reached and not ',' or '\n' ERROR
                        {
                            fprintf(stderr, "Invalid: Input is too large\n");
                            return -1;
                        }
                        if (c == ',' || c == '\n')
                        {   //if c is ',' or '\n' signifies end of the field and \n new line 
                            *(input_buffer + i) = '\0';

                            if (field == 0 && (strcmp(input_buffer, *(node_names + (num_line - 1))) != 0))  //name field compare with the correct name 
                            {    
                                fprintf(stderr, "Invalid: Taxa does not match corresponding column\n");
                                return -1;
                            }
                            else if (field > 0 && c == '\n' && field != num_taxa)  //if at \n and field != N+1 ERROR; REMEMBER STARTING FIELDS AT 0
                            {   
                                fprintf(stderr, "Invalid: Incorrect amount of fields\n"); 
                                return -1;
                            }
                            else if (field != 0) //validate number field, convert and store into distance matrix  
                            { 
                                if (invalid_input(input_buffer) == 1) 
                                {
                                    fprintf(stderr, "Invalid Distance Input\n");
                                    return -1;
                                }

                                double field_distance = string_to_double(input_buffer);
                                *(*(distances + (num_line - 1)) + (field - 1)) = field_distance;
                            }
                            
                            if (c == '\n') 
                            {
                                num_line++;
                                field = 0;
                            } 
                            else 
                            {
                                field++;
                            }
                            break;
                        }
                        else
                        {   //add character in buffer
                            *(input_buffer + i) = c;
                        }
                    }   
                }  
            } 
        }
    }

    for (int i = 0; i < num_taxa; i++)
    {   //initialize active node map
        *(active_node_map + i) = i;
    }

    for (int i = 0; i < num_taxa; i++)
    {   //validating a symmetric matrix
        for (int j = 0; j < num_taxa; j++)
        {
            if (*(*(distances + i) + j) != *(*(distances + j) + i) || (i == j && *(*(distances + i) + j) != 0.0))
            {
                fprintf(stderr, "Invalid Matrix not Symmetric\n");
                return -1;
            } 
        }
    }

    return 0;
    abort();
}

/**
 * @brief  Emit a representation of the phylogenetic tree in Newick
 * format to a specified output stream.
 * @details  This function emits a representation in Newick format
 * of a synthesized phylogenetic tree to a specified output stream.
 * See (https://en.wikipedia.org/wiki/Newick_format) for a description
 * of Newick format.  The tree that is output will include for each
 * node the name of that node and the edge distance from that node
 * its parent.  Note that Newick format basically is only applicable
 * to rooted trees, whereas the trees constructed by the neighbor
 * joining method are unrooted.  In order to turn an unrooted tree
 * into a rooted one, a root will be identified according by the
 * following method: one of the original leaf nodes will be designated
 * as the "outlier" and the unique node adjacent to the outlier
 * will serve as the root of the tree.  Then for any other two nodes
 * adjacent in the tree, the node closer to the root will be regarded
 * as the "parent" and the node farther from the root as a "child".
 * The outlier node itself will not be included as part of the rooted
 * tree that is output.  The node to be used as the outlier will be
 * determined as follows:  If the global variable "outlier_name" is
 * non-NULL, then the leaf node having that name will be used as
 * the outlier.  If the value of "outlier_name" is NULL, then the
 * leaf node having the greatest total distance to the other leaves
 * will be used as the outlier.
 *
 * @param out  Stream to which to output a rooted tree represented in
 * Newick format.
 * @return 0 in case the output is successfully emitted, otherwise -1
 * if any error occurred.  If the global variable "outlier_name" is
 * non-NULL, then it is an error if no leaf node with that name exists
 * in the tree.
 */

int emit_newick_format(FILE *out) {
    if (num_taxa == 1)
    {
        return 0;
    }
    
    //Find the outlier
    int outlier_index = 0;
    if (outlier_name == NULL)
    {   //if outlier_name is not assigned outlier will be the leaf node with total distance with other leafs. aka row sum till leaf
        sum_leaf_distance();    //sum rows of leaf - leaf
        
        double largest_distance = 0;
        for (int i = 0; i < num_taxa; i++)
        {
            if (*(row_sums + i) > largest_distance)
            {
                largest_distance = *(row_sums + i);
                outlier_index = i;
            }
        }
        
        outlier_name = *(node_names + outlier_index);   //new outlier        
    }
    else
    {   //check if the name is an existing leaf node
        outlier_index = find_leaf_node_index(outlier_name);
        if (outlier_index == -1)
        {  
            return -1;
        }
        
    }
    
    //find root of tree
    NODE* root = *(nodes + outlier_index)->neighbors;

    recursive_print(out, (nodes + outlier_index), root);
    fprintf(out, "\n");

    if (fflush(out) == EOF){
        return -1;
    }
    return 0;
    abort();
}

/**
 * @brief  Emit the synthesized distance matrix as CSV.
 * @details  This function emits to a specified output stream a representation
 * of the synthesized distance matrix resulting from the neighbor joining
 * algorithm.  The output is in the same CSV form as the program input.
 * The number of rows and columns of the matrix is equal to the value
 * of num_all_nodes at the end of execution of the algorithm.
 * The submatrix that consists of the first num_leaves rows and columns
 * is identical to the matrix given as input.  The remaining rows and columns
 * contain estimated distances to internal nodes that were synthesized during
 * the execution of the algorithm.
 *
 * @param out  Stream to which to output a CSV representation of the
 * synthesized distance matrix.
 * @return 0 in case the output is successfully emitted, otherwise -1
 * if any error occurred.
 */
int emit_distance_matrix(FILE *out) {
    for (int i = 0; i < num_all_nodes; i++)
    {
        fprintf(out, ",%s", *(node_names + i));
    }
    fprintf(out,"\n");

    for (int i = 0; i < num_all_nodes; i++)
    {
        fprintf(out, "%s", *(node_names + i));
        for (int j = 0; j < num_all_nodes; j++)
        {
            fprintf(out, ",%.2f", *(*(distances + i) + j));
        }
        fprintf(out, "\n");
    }
    
    if (fflush(out) == EOF)
    {
        return -1;
    }

    return 0;
    abort();
}

/**
 * @brief  Build a phylogenetic tree using the distance data read by
 * a prior successful invocation of read_distance_data().
 * @details  This function assumes that global variables and data
 * structures have been initialized by a prior successful call to
 * read_distance_data(), in accordance with the specification for
 * that function.  The "neighbor joining" method is used to reconstruct
 * phylogenetic tree from the distance data.  The resulting tree is
 * an unrooted binary tree having the N taxa from the original input
 * as its leaf nodes, and if (N > 2) having in addition N-2 synthesized
 * internal nodes, each of which is adjacent to exactly three other
 * nodes (leaf or internal) in the tree.  As each internal node is
 * synthesized, information about the edges connecting it to other
 * nodes is output.  Each line of output describes one edge and
 * consists of three comma-separated fields.  The first two fields
 * give the names of the nodes that are connected by the edge.
 * The third field gives the distance that has been estimated for
 * this edge by the neighbor-joining method.  After N-2 internal
 * nodes have been synthesized and 2*(N-2) corresponding edges have
 * been output, one final edge is output that connects the two
 * internal nodes that still have only two neighbors at the end of
 * the algorithm.  In the degenerate case of N=1 leaf, the tree
 * consists of a single leaf node and no edges are output.  In the
 * case of N=2 leaves, then no internal nodes are synthesized and
 * just one edge is output that connects the two leaves.
 *
 * Besides emitting edge data (unless it has been suppressed),
 * as the tree is built a representation of it is constructed using
 * the NODE structures in the nodes array.  By the time this function
 * returns, the "neighbors" array for each node will have been
 * initialized with pointers to the NODE structure(s) for each of
 * its adjacent nodes.  Entries with indices less than N correspond
 * to leaf nodes and for these only the neighbors[0] entry will be
 * non-NULL.  Entries with indices greater than or equal to N
 * correspond to internal nodes and each of these will have non-NULL
 * pointers in all three entries of its neighbors array.
 * In addition, the "name" field each NODE structure will contain a
 * pointer to the name of that node (which is stored in the corresponding
 * entry of the node_names array).
 *
 * @param out  If non-NULL, an output stream to which to emit the edge data.
 * If NULL, then no edge data is output.
 * @return 0 in case the output is successfully emitted, otherwise -1
 * if any error occurred.
 */

int build_taxonomy(FILE *out) {
    // Q(a,b) = (n - 2) * D(a,b) - S(a) - S(b)
    
    while (num_active_nodes > 2)
    {
        double q_compare = 0;
        double q = 0;
        int i_q = 0, j_q = 1;

        sum_of_matrix(); //calculated row sum of distance before iteration

        q = (num_active_nodes - 2) * (*(*(distances + *active_node_map) + *(active_node_map + 1))) - *(row_sums + *active_node_map) - *(row_sums + (*active_node_map + 1)); //Q value for [i,j,x, ... N - 1]) Q value for pair (i,j)

        for (int i = 0; i < num_active_nodes; i++)
        {
            for (int j = 0; j < num_active_nodes; j++)
            {
                if (i != j)
                {   //main diagonal we don't care about it
                    q_compare = (num_active_nodes - 2) * (*(*(distances + *(active_node_map + i)) + *(active_node_map + j))) - *(row_sums + *(active_node_map + i)) - *(row_sums + *(active_node_map + j)); //Q value for (i,j) for any matrix
                    if (q_compare < q)
                    {   //find the minimum Q and store the node that will be combined
                        q = q_compare;
                        i_q = i;
                        j_q = j;
                    }  
                }
            }
        }

        //create the new node to connect node i and node j; update node i and j neighbor, update active node map, num all nodes, num_active_nodes
        
        sprintf(*(node_names + num_all_nodes), "#%d", num_all_nodes);
        (nodes + num_all_nodes)->name = *(node_names + num_all_nodes);  //new name for new node  

        two_neighbors((nodes + num_all_nodes), (nodes + *(active_node_map + i_q))); //becoming eachother neighbor
        two_neighbors((nodes + num_all_nodes), (nodes + *(active_node_map + j_q))); //becoming eachother neighbor

        //change the distances
        for (int i = 0; i < num_active_nodes; i++)
        {
            double distance_insert = 0;
            double* distance_row_col = (*(distances + num_all_nodes) + *(active_node_map + i));
            double* distance_col_row = (*(distances + *(active_node_map + i)) + num_all_nodes);

            if (*(active_node_map + i) == num_all_nodes)
            {
                *distance_row_col = 0;
                *distance_col_row = 0;
            }
            else if (*(active_node_map + i) == *(active_node_map + i_q))
            {
                distance_insert = (*(*(distances + *(active_node_map + i_q)) + *(active_node_map + j_q)) + (*(row_sums + *(active_node_map + i_q)) - *(row_sums + *(active_node_map + j_q)))/(num_active_nodes - 2))/2;
                *distance_col_row = distance_insert;
                *distance_row_col = distance_insert;
            }
            else if (*(active_node_map + i) == *(active_node_map + j_q))
            {
                distance_insert = (*(*(distances + *(active_node_map + i_q)) + *(active_node_map + j_q)) + (*(row_sums + *(active_node_map + j_q)) - *(row_sums + *(active_node_map + i_q)))/(num_active_nodes - 2))/2;
                *distance_col_row = distance_insert;
                *distance_row_col = distance_insert;
            }
            else
            {
                distance_insert = (*(*(distances + *(active_node_map + i_q)) + *(active_node_map + i)) + *(*(distances + *(active_node_map + j_q)) + *(active_node_map + i)) - *(*(distances + *(active_node_map + i_q)) + *(active_node_map + j_q))) / 2;
                *distance_col_row = distance_insert;
                *distance_row_col = distance_insert;
            }
        }

        if (global_options == 0)
        {   //only output if it is global 0
            fprintf(out, "%d,%d,%.2f\n",*(active_node_map + i_q), num_all_nodes, *(*(distances + *(active_node_map + i_q)) + num_all_nodes));
            fprintf(out, "%d,%d,%.2f\n",*(active_node_map + j_q), num_all_nodes, *(*(distances + *(active_node_map + j_q)) + num_all_nodes));
        }

        *(active_node_map + i_q) = num_all_nodes;   //index of i_q now is value and index of new node
        *(active_node_map + j_q) = *(active_node_map + num_active_nodes - 1);    //index of j_q now value of right most active node

        num_all_nodes++;    //update
        num_active_nodes--;
         
    }
    if (num_active_nodes == 2)
    {   //last two are eachother parent neighbor[0]
        NODE **ptr;
        ptr = (nodes + *(active_node_map))->neighbors;
        *ptr = (nodes + *(active_node_map + 1));

        ptr = (nodes + *(active_node_map + 1))->neighbors;
        *ptr = (nodes + *(active_node_map));

        if (global_options == 0 && out != NULL)
        {   //only output if it is global 0
            if (*(active_node_map) < *(active_node_map + 1))
            {
                fprintf(out, "%d,%d,%.2f\n",*(active_node_map), *(active_node_map + 1), *(*(distances + *(active_node_map)) + *(active_node_map + 1)));
            }
            else
            {
                fprintf(out, "%d,%d,%.2f\n",*(active_node_map + 1), *(active_node_map), *(*(distances + *(active_node_map)) + *(active_node_map + 1)));
            }
            
        }
    }
    
    if (num_active_nodes == 1 && out != NULL && global_options == 0)
    {
        fprintf(out, "%d,%d,%.2f\n",0,0,0.0);
    }
    
    if (fflush(out) == EOF) 
    {
        return -1;
    }

    return 0;
    abort();
}
