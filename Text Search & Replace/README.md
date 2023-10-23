In this assignment you will be writing a main function that takes command-line arguments that represent text-manipulation commands to search for text in an input file and generate a copy of the input file with the search-text replaced with the provided replacement-text. The options (-l, -s, -r, -w) and other arguments given at the command line include:


-s search_text: the text to search for in the input file. This option is required.
-r replace_text: the text that will replace the search text in the output file. This option is required.
-w: enable wildcard searching of words, explained later. This option is optional.
-l start_line,end_line: which lines of the input file to process. The range is inclusive. Lines outside this range are simply copied from the input file to the output file unmodified. If end_line is greater than the number of lines in the file, simply read to the end of the file. Note that line numbers start at 1, not 0. This option is optional.
infile: pathname of the input file. This argument is required.
outfile: pathname of the output file. This argument is required.


(I realize that it is strange to talk about “optional options” and “required options”, but terms like “option”, “flag”, “parameter”, “argument” and the like do not have official definitions in programming.)


For example, consider the command-line arguments 


-s hello -r wonder -l 4,10 dir1/input1.txt dir2/output1.txt


These arguments indicate that we should search for occurrences of "hello" anywhere in the lines 4 through 10 (inclusive) in the file input1.txt (located in the directory named "dir1") and replace the occurrences of "hello" with "wonder" in the file output1.txt (which we must create in the directory "dir2"). All other text is simply copied from input1.txt to output1.txt. In other words, you are implementing a simple search/replace feature found in basically every text editor available. The search text may appear anywhere in the file and need not begin or end a word. With the exception of infile and outfile, which must appear in that order at the end of the command-line arguments, the command-line arguments may appear in any order.


Assuming argc is at least 7, we will assume that argv[argc-2] provides infile, and argv[argc-1] provides outfile. This assumption will make your implementation of Part 1 a little easier. You may also assume that infile and outfile refer to different files. Please remember that argv[0] stores the name of the executable itself. The “real” arguments begin at argv[1].
