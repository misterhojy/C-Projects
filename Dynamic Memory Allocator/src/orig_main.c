
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include "version.h"
#include "global.h"
#include "gradedb.h"
#include "stats.h"
#include "read.h"
#include "write.h"
#include "normal.h"
#include "sort.h"
#include "error.h"
#include "report.h"

/*
 * Course grade computation program
 */

#define REPORT          0
#define COLLATE         1
#define FREQUENCIES     2
#define QUANTILES       3
#define SUMMARIES       4
#define MOMENTS         5
#define COMPOSITES      6
#define INDIVIDUALS     7
#define HISTOGRAMS      8
#define TABSEP          9
#define ALLOUTPUT      10
#define SORTBY         11
#define NONAMES        12
#define OUTPUT         13


static struct option_info {
        unsigned int val;
        char *name;
        char chr;
        int has_arg;
        char *argname;
        char *descr;
} option_table[] = { 
 {REPORT,         "report",    'r',      no_argument, NULL,
                  "Process input data and produce specified reports."},
 {COLLATE,        "collate",   'c',      no_argument, NULL,
                  "Collate input data and dump to standard output."},
 {FREQUENCIES,    "freqs",     0,        no_argument, NULL,
                  "Print frequency tables."},
 {QUANTILES,      "quants",    0,        no_argument, NULL,
                  "Print quantile information."},
 {SUMMARIES,      "summaries", 0,        no_argument, NULL,
                  "Print quantile summaries."},
 {MOMENTS,        "stats",     0,        no_argument, NULL,
                  "Print means and standard deviations."},
 {COMPOSITES,     "comps",     0,        no_argument, NULL,
                  "Print students' composite scores."},
 {INDIVIDUALS,    "indivs",    0,        no_argument, NULL,
                  "Print students' individual scores."},
 {HISTOGRAMS,     "histos",    0,        no_argument, NULL,
                  "Print histograms of assignment scores."},
 {TABSEP,         "tabsep",    0,        no_argument, NULL,
                  "Print tab-separated table of student scores."},
 {ALLOUTPUT,      "all",       'a',      no_argument, NULL,
                  "Print all reports."},
 {SORTBY,         "sortby",    'k',      required_argument, "key",
                  "Sort by {name, id, score}."},
 {NONAMES,        "nonames",   'n',      no_argument, NULL,
                  "Suppress printing of students' names."},
 {OUTPUT,        "output",   'o',      required_argument, "file",
                 "Specify file to be used for output."}
};

static char *short_options = "rcak:no:";
static struct option long_options[15]; 

static void init_options() {
    for(unsigned int i = 0; i < 14; i++) {  
        struct option_info *oip = &option_table[i];
        if(oip->val != i) {
            fprintf(stderr, "Option initialization error\n");
            abort();
        }
        struct option *op = &long_options[i];
        op->name = oip->name;
        op->has_arg = oip->has_arg;
        op->flag = NULL;
        op->val = oip->val;
    }

}


static int report = 0, collate = 0, freqs = 0, quantiles = 0, summaries = 0, moments = 0,
           scores = 0, composite = 0, histograms = 0, tabsep = 0, nonames = 0;

static void usage();

FILE *output_file = NULL;

int orig_main(argc, argv)
int argc;
char *argv[];
{
        Course *c;
        Stats *s;
        char optval; 
        int (*compare)() = comparename;

        fprintf(stderr, BANNER);
        init_options();
        if(argc <= 1) usage(argv[0]);
        while(optind < argc) {
            if((optval = getopt_long(argc, argv, short_options, long_options, NULL)) != -1) {  
                switch(optval) {
                case ('r'):
                case REPORT: 
                    report++; 
                    if (report > 1) usage(argv[0]);
                    break;
                case ('c'):
                case COLLATE: 
                    collate++; 
                    if (collate > 1) usage(argv[0]);
                    break;
                case TABSEP: 
                    tabsep++;
                    if (report == 0 && collate == 0) usage(argv[0]);
                    break;
                case ('n'):
                case NONAMES: 
                    nonames++;
                    if (report == 0 && collate == 0) usage(argv[0]);
                    break;
                case ('o'):
                case OUTPUT:
                    if (report == 0 && collate == 0) usage(argv[0]);
                    output_file = fopen(optarg, "w");
                    if (output_file == NULL) {
                        fatal("Can't open data file %s.\n", optarg);
                    }
                    break;
                case ('k'):
                case SORTBY:
                    if (report == 0 && collate == 0) usage(argv[0]);
                    if(!strcmp(optarg, "name")) {
                        compare = comparename; 
                    } else if(!strcmp(optarg, "id")) {
                        compare = compareid;
                    } else if(!strcmp(optarg, "score")) {
                        compare = comparescore;
                    } else {
                        if (optval == 107)
                        {
                            optval = 11;
                        }
                        
                        fprintf(stderr,
                                "Option '%s' requires argument from {name, id, score}.\n\n",
                                option_table[(int)optval].name);
                        usage(argv[0]);
                    }
                    break;
                case FREQUENCIES: 
                    freqs++;
                    if (report == 0 && collate == 0) usage(argv[0]);
                    break;
                case QUANTILES: 
                    quantiles++; 
                    if (report == 0 && collate == 0) usage(argv[0]);
                    break;
                case SUMMARIES: 
                    summaries++; 
                    if (report == 0 && collate == 0) usage(argv[0]);
                    break;
                case MOMENTS: 
                    moments++; 
                    if (report == 0 && collate == 0) usage(argv[0]);
                    break;
                case COMPOSITES: 
                    composite++; 
                    if (report == 0 && collate == 0) usage(argv[0]);
                    break;
                case INDIVIDUALS: 
                    scores++; 
                    if (report == 0 && collate == 0) usage(argv[0]);
                    break;
                case HISTOGRAMS: 
                    histograms++; 
                    if (report == 0 && collate == 0) usage(argv[0]);
                    break;
                case ('a'):
                case ALLOUTPUT:
                    if (report == 0 && collate == 0) usage(argv[0]);
                    freqs++; quantiles++; summaries++; moments++;
                    composite++; scores++; histograms++; tabsep++;
                    break;
                case '?':
                    usage(argv[0]);
                    break;
                default:
                    usage(argv[0]);
                    break;
                }
            } else {
                break;
            }
        }
         if(optind == argc) {
                fprintf(stderr, "No input file specified.\n\n");
                usage(argv[0]);
        }
        char *ifile = argv[optind];
        if(report == collate) {
                fprintf(stderr, "Exactly one of '%s' or '%s' is required.\n\n",
                        option_table[REPORT].name, option_table[COLLATE].name);
                usage(argv[0]);
        }

        if (!output_file)
                {
                    output_file = stdout;
                }

        fprintf(stderr, "Reading input data...\n"); //Why are we printing to the error
        c = readfile(ifile);
        if(geterrors()) {    //errors in the scope of orig_main.c always no initial
           printf("%d error%s found, so no computations were performed.\n",
                  geterrors(), geterrors() == 1 ? " was": "s were");
           exit(EXIT_FAILURE);
        }

        fprintf(stderr, "Calculating statistics...\n");
        s = statistics(c);
        if(s == NULL) fatal("There is no data from which to generate reports.");
        normalize(c);
        composites(c);
        sortrosters(c, comparename);
        checkfordups(c->roster);
        if(collate) {
                fprintf(stderr, "Dumping collated data...\n");
                writecourse(output_file, c);
                exit(geterrors() ? EXIT_FAILURE : EXIT_SUCCESS);
        }
        sortrosters(c, compare);

        fprintf(stderr, "Producing reports...\n");
        reportparams(output_file, ifile, c);
        if(moments) reportmoments(output_file, s);
        if(composite) reportcomposites(output_file, c, nonames);
        if(freqs) reportfreqs(output_file, s);
        if(quantiles) reportquantiles(output_file, s);
        if(summaries) reportquantilesummaries(output_file, s);
        if(histograms) reporthistos(output_file, c, s);
        if(scores) reportscores(output_file, c, nonames);
        if(tabsep) reporttabs(output_file, c);

        fprintf(stderr, "\nProcessing complete.\n");
        printf("%d warning%s issued.\n", getwarning()+geterrors(),
               getwarning()+geterrors() == 1? " was": "s were");
        exit(geterrors() ? EXIT_FAILURE : EXIT_SUCCESS);
}

void usage(name)    //output does not need to be changed
char *name;
{
        struct option_info *opt;

        fprintf(stderr, "Usage: %s [options] <data file>\n", name);
        fprintf(stderr, "Valid options are:\n");
        for(unsigned int i = 0; i < 14; i++) {
                opt = &option_table[i];
                char optchr[5] = {' ', ' ', ' ', ' ', '\0'};
                if(opt->chr)
                  sprintf(optchr, "-%c, ", opt->chr);
                char arg[32];
                if(opt->has_arg)
                    sprintf(arg, " <%.10s>", opt->argname);
                else
                    sprintf(arg, "%.13s", "");
                fprintf(stderr, "\t%s--%-10s%-13s\t%s\n",
                            optchr, opt->name, arg, opt->descr);
                opt++;
        }
        exit(EXIT_FAILURE);
}
