#include "../include/hw6.h"

void handleWritingToTargetFile(char *word, int issuffix, int isprefix, char *sinput, char *rinput, FILE *target_file) {
    if (word == NULL) {
        return;
    }
    // printf("Reading word: -%s-\n", word);
    int firstIndexOfSinput = -1;
    char *sinputOccurance = strstr(word, sinput);
    if (sinputOccurance != NULL) {
         firstIndexOfSinput = sinputOccurance - word;
    }
    // printf("firstIndexOfSinput: %d\n", firstIndexOfSinput);
    int lastStartingIndexOfSinput = firstIndexOfSinput;
    while(sinputOccurance != NULL) {
        //  printf("sinputOccurance not null\n");
        lastStartingIndexOfSinput = sinputOccurance - word;
        // printf("lastStartingIndexOfSinput: %d\n", lastStartingIndexOfSinput);
        sinputOccurance = strstr(sinputOccurance + strlen(sinput), sinput);
    }
    sinputOccurance = NULL;

    // printf("SUFFIX: %d ", issuffix);
    // printf("Sinput: %s\n", sinput);
    // printf("lastStartingIndexOfSinput: %d\n", lastStartingIndexOfSinput);
    // printf("lastStartingIndexOfSinput + len sinput: %lu\n", lastStartingIndexOfSinput + strlen(sinput));
    // printf("word length: %lu\n", strlen(word));

    if (issuffix &&  (lastStartingIndexOfSinput + strlen(sinput) == strlen(word)) && lastStartingIndexOfSinput > 0) {
        // printf("Handling suffix case for word: %s, will replace with %s\n", word, rinput);
        fputs(rinput, target_file);
    }
    else if(isprefix && firstIndexOfSinput == 0) {
        // printf("Handling prefix case for word: %s, will replace with %s\n", word, rinput);
        fputs(rinput, target_file);
    }
    else if (strcmp(word, sinput) == 0) {
        // printf("Handling case where word is identical: %s, will replace with %s\n", word, rinput);
        fputs(rinput, target_file);
    }
    else if (!isprefix && !issuffix && firstIndexOfSinput >= 0 && strstr(word, sinput) != NULL) {
        // printf("Handling where neither prefix or post fix: %s, will replace with ", word);
        char *previousStartingPoint = word;
        char *sinputOccuranceDuringReplace = strstr(previousStartingPoint, sinput);
        char charactersBeforeSinput[MAX_LINE];
        while(sinputOccuranceDuringReplace != NULL) {
            memset(charactersBeforeSinput, 0, sizeof charactersBeforeSinput);
            if (sinputOccuranceDuringReplace != previousStartingPoint) {
                char *charactersBeforeSinputPointer = strncpy(charactersBeforeSinput, previousStartingPoint, sinputOccuranceDuringReplace - previousStartingPoint);
                // printf("charactersBeforeSinput IS : %s\n", charactersBeforeSinputPointer);
                fputs(charactersBeforeSinputPointer, target_file);
                // printf("%s", charactersBeforeSinputPointer);
                charactersBeforeSinputPointer = NULL;
            }
            fputs(rinput, target_file);
            // printf("%s", rinput);
            previousStartingPoint = sinputOccuranceDuringReplace + strlen(sinput);
            sinputOccuranceDuringReplace = strstr(sinputOccuranceDuringReplace + strlen(sinput), sinput);
        }
        // printf("ABout to do the if case with previous starting point\n");
        // printf("previous starting point: %s\n", previousStartingPoint);
        if (sinputOccuranceDuringReplace == NULL && previousStartingPoint != NULL && previousStartingPoint - word < (signed) strlen(word)) {
            fputs(previousStartingPoint, target_file);
            // printf("%s", previousStartingPoint);
        }
        // printf("\n");
    }
    else {
        // printf("Handling case where word does not match so just gonna write the word: %s, will replace with %s\n", word, word);
        fputs(word, target_file);
    }
}

int main(int argc, char *argv[]) {

    //return error if less than 8 arguments. argc[0] is name nad last argc[] is terminator
    if (argc < 7) {
        fprintf(stderr, "MISSING_ARGUMENT\n");
        return MISSING_ARGUMENT;
    }

    //using getopt in while loop to get each command argument and switch cases
    extern char *optarg;
    extern int optind, opterr, optopt;
    FILE *source_file, *target_file;
    char *sinput = "\0", *rinput = "\0", *linput = "\0", *inputxt = "\0", *outputxt = "\0";
    int opt, sflag = 0, rflag = 0, lflag = 0, wflag = 0;
    opterr = 0;

    //getopt going to each command line option
    while((opt = getopt(argc,argv, "s:r:l:w")) != -1){
        //case to know which option -s -r -l -w and do actions accordingly
        switch (opt)
        {
        /*in each case we will be checking if that flag has already set and if it has than it is duplicate 
        we also going to set the flag if we didnt and also give the parameter a pointer to access
        has to ignore unknown options*/
        case 's':
            if(sflag == 1){
                fprintf(stderr, "DUPLICATE_ARGUMENT\n");
                return DUPLICATE_ARGUMENT;
            }
            sflag = 1;
            sinput = optarg;
            break;
        case 'r':
            if(rflag == 1){
                fprintf(stderr, "DUPLICATE_ARGUMENT\n");
                return DUPLICATE_ARGUMENT;
            }
            rflag = 1;
            rinput = optarg;
            break;
        case 'l':
            if(lflag == 1){
                fprintf(stderr, "DUPLICATE_ARGUMENT\n");
                return DUPLICATE_ARGUMENT;
            }
            lflag = 1;
            linput = optarg;
            break;
        case 'w':
            if(wflag == 1){
                fprintf(stderr, "DUPLICATE_ARGUMENT\n");
                return DUPLICATE_ARGUMENT;
            }
            wflag = 1;
            break;
       case '?':
            if (argv[optind - 1][0] != '-'){
                optind++;
            }
            if (argv[optind][0] != '-'){
                optind++;
            }
            break;
        }
    }
    // printf("sflag = %d\n", sflag);
	// printf("rflag = %d\n", rflag);
    // printf("lflag = %d\n", lflag);
    // printf("wflag = %d\n", wflag);
    // printf("input text = \"%s\"\n", inputxt);
    // printf("output text = \"%s\"\n", outputxt);

    /*checking if the input can be opened if not throwing an error. Doing the same with the ouput and to make sure it can be created*/
    inputxt = argv[argc - 2];
    source_file = fopen(inputxt, "r");
    if (source_file == NULL) {
        fprintf(stderr, "INPUT_FILE_MISSING\n");
        return INPUT_FILE_MISSING;
    }
    fclose(source_file);
    outputxt = argv[argc - 1];
    target_file = fopen(outputxt, "w");
    if (target_file== NULL) {
        fprintf(stderr, "OUTPUT_FILE_UNWRITEABLE\n");
        return OUTPUT_FILE_UNWRITABLE;
    }
    fclose(target_file);

    /*checking if we are missing the mandetory -s and -r options, and checking in order as given 
    and the argument that immediately follows the -s/-r/-l option on the command line must not begin with a "-" character.*/
    if (sflag == 0 || sinput[0] == '-') {
        fprintf(stderr, "S_ARGUMENT_MISSING\n");
        return S_ARGUMENT_MISSING;
    }
    if (rflag == 0 || rinput[0] == '-') {
        fprintf(stderr, "R_ARGUMENT_MISSING\n");
        return R_ARGUMENT_MISSING;
    }

    /*Checks if -l is valid and that the input numbers are correct inputs
    what if -s the -r end -l 43,32.txt src/output.txt
    what if input.txt is empty no lines and we have index for lines*/
    long startind, endind;
    if(lflag){
        if (linput[0] == '-') {
                fprintf(stderr, "L_ARGUMENT_INVALID\n");
                return L_ARGUMENT_INVALID;
            }
        char *strstart, *strend;
        strstart = strtok(linput, ",");
        strend = strtok(NULL, ",");
        if (strend == NULL || strstart == NULL) {
            fprintf(stderr, "L_ARGUMENT_INVALID\n");
            return L_ARGUMENT_INVALID;
        }
        startind = strtol(strstart, NULL, 10);
        endind = strtol(strend, NULL, 10);
        if (startind <= 0 || endind <= 0 || (startind > endind)){
            fprintf(stderr, "L_ARGUMENT_INVALID\n");
            return L_ARGUMENT_INVALID;
        }
    }
    //printf("startline index %ld\n", startind);
    //printf("endline index %ld\n", endind);

    /*if indicates -w, then -s input must have a * on either end of the word but not at both ends*/
    int isprefix = 0, issuffix = 0;
    if (wflag && ((sinput[0] != '*' && sinput[strlen(sinput) - 1] != '*') || (sinput[0] == '*' && sinput[strlen(sinput) - 1] == '*'))){
        fprintf(stderr, "WILDCARD_INVALID\n");
        return WILDCARD_INVALID;
    } else if (wflag && sinput[0] == '*') {
        issuffix = 1;
        memmove(sinput, sinput + 1, strlen(sinput));
    } else if (wflag && sinput[strlen(sinput) - 1] == '*') {
        isprefix = 1;
        sinput[strlen(sinput) - 1] = '\0';
    }

    /*implementing the functionality
    Checks if the line is in bounds of -l if that option is there, checks if the sinput is there. 
    if not then just put the line in output, if there is find out where and copy the other chars that arent the sinput
    put it in output and index over sinput and just put the rinput. Also does the -w options of prefix and suffix*/
    char line[MAX_LINE];
    int lineindex = 1;
    char word[MAX_LINE] = "";

    source_file = fopen(inputxt, "rb");
    target_file = fopen(outputxt, "wb+");

    /*iterating through each line of the input file*/
    while ((fgets(line, MAX_LINE, source_file)) != NULL) {
        if (lflag && !(lineindex >= startind && lineindex <= endind)) {
            // printf("ignoring line, just print the line: %s\n", line);
            fputs(line, target_file);
            lineindex++;
            continue;
        }
        // printf("\n\n\nReading line: %s WITH SIZE : %d\n", line, strlen(line));
        int startOfWord = 0;
        for(int i = 0; (unsigned long) i < strlen(line); i++) {
             if (isspace(line[i]) || ispunct(line[i]) || line[i] == '\n' || line[i] == '\0' || line[i] == EOF) {
                if (startOfWord < i && i - startOfWord > 0) {
                    // printf("\nStart of Word: %d WITH I : %ld And line size: %d\n", startOfWord, i, strlen(line));
                    char *wordPointer = strncpy(word, line + startOfWord, i - startOfWord);
                    handleWritingToTargetFile(wordPointer, issuffix, isprefix, sinput, rinput, target_file);
                    memset(word, 0, sizeof word);
                    wordPointer = NULL;
                }
                fputc(line[i], target_file);
                startOfWord = i + 1;
            }
        }
        if (feof(source_file) && (unsigned long) startOfWord < strlen(line)) {
            // printf("\nStart of Word: %d\n", startOfWord);
            char *wordPointer = strncpy(word, line + startOfWord, strlen(line) - startOfWord);
            handleWritingToTargetFile(wordPointer, issuffix, isprefix, sinput, rinput, target_file);
            memset(word, 0, sizeof word);
            wordPointer = NULL;
        }
        lineindex++;
    }
    fclose(source_file);
    fclose(target_file);
    return 0;
}
