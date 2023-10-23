In this assignment you will be implementing a program to perform simple linear algebra computations: matrix addition, multiplication and transpose. The matrices and formulas containing these operations will be stored in scripts that your program must parse and execute. You’ll need to manage memory carefully, allocating and deallocating data structures as you process the file and evaluate expressions.
Mathematical Preliminaries
Matrices
A matrix is a rectangular array of numbers arranged in rows and columns. The size of a matrix is determined by the number of rows and columns it contains. Matrices are widely used in mathematics, physics, engineering, computer science, and many other fields.
Matrix Addition
Matrix addition is an operation that involves adding the corresponding elements of two matrices of the same size. Mathematically, if A and B are both m x n matrices (with m rows and n columns), then their sum, denoted as C, is an m x n matrix where the entry in the i-th row and j-th column of C is given by: 

Cij=Aij+Bij

In this formula, Aij is the entry in the i-th row and j-th column of matrix A, and Bij is the entry in the i-th row and j-th column of matrix B. Note that in order to add two matrices, they must be of the same size, i.e., they must have the same number of rows and the same number of columns. 

In our scripts, matrix addition will be denoted C = A+B. Any number of spaces may appear before or after symbols and matrix names in our formulas.
Matrix Multiplication
Matrix multiplication is a way of combining two matrices together to produce a new matrix. The resulting matrix has the same number of rows as the first matrix and the same number of columns as the second matrix.

Mathematically, if A is an m x n matrix and B is an n x p matrix, then the product of A and B, denoted as C, is an m x p matrix where the entry in the i-th row and j-th column of C is given by:
Cij=k=1nAikBkj

In this formula, Aik is the entry in the i-th row and k-th column of A, and Bkj is the entry in the k-th row and j-th column of B.

In our scripts, matrix multiplication will be denoted C = A*B.
Matrix Transpose
Matrix transpose is an operation that involves flipping a matrix over its diagonal. Mathematically, if A is an m x n matrix, then its transpose, denoted as AT, is an n x m matrix where the entry in the i-th row and j-th column of AT is given by:

ATij=Aji

In other words, the rows of the original matrix become columns in the transposed matrix, and the columns of the original matrix become rows in the transposed matrix.

In our scripts, matrix transpose will be denoted B=A'. (Single quotation mark.)
Script Format
Your program will process a script containing commands to define matrices and perform operations on them. Every line of a script has one of two forms:
a definition of a new matrix from a list of values
a definition of a new matrix created from a formula containing at least one operator
A new matrix is defined by providing:
a single uppercase character
zero or more spaces
an equals symbol
zero or more spaces
a positive integer containing one or more digits that gives the number of rows of the matrix (call this number NR)
one or more spaces
a positive integer containing one or more digits that gives the number of columns of the matrix (call this number NC)
zero or more spaces
a left square bracket
zero or more spaces
NC integers separated by spaces and terminated by a semicolon. There might or might not be spaces surrounding the semicolon.
NR-1 additional lists of NC integers as described in the previous item
a right square bracket
zero or more spaces
a newline
Example: A =  3   2 [4 5; 19 -34  ;  192 -9110;   ]   \n
A formula is defined by providing:
a single uppercase character that has not appeared in the script on a previous line
zero or more spaces
an equals symbol
zero or more spaces
an expression consisting of + (for addition), * (for multiplication), ' (for transpose), left and right parentheses, and matrix names. The precedence of the operators, from highest to lowest is:
transpose
multiplication
addition
Parentheses can be used to change the order of evaluation. An expression may contain spaces anywhere throughout the expression around any of the symbols.
a newline

Example: Z = (A   +B)’ * C * (D’ + A)’   \n
Once a symbol has been defined, it cannot be redefined (i.e., reassigned). A symbol can appear multiple times in the right-hand side of a formula.
Scripts will always be syntactically valid. Extra spaces (or no spaces) may surround the tokens (e.g., matrix name, equals sign, left and and right square brackets, left and right parentheses, semicolons, operators, numbers). When two integers are next to each other, they will be separated by at least one space, or in the case where we have reached the end of a row, a semicolon. There might or might not be spaces around the semicolons.
Scripts will always be semantically valid, meaning that only valid formulas will be provided. For instance, a script will not contain formulas that cannot be evaluated for matrices of the given dimensions.
To summarize: your main task in parsing the text file is NOT to worry about error cases because there will be no error cases. Focus on implementing the order of operations correctly and evaluating the expressions correctly.

