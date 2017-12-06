/** 
 * \author Daumen Anton and Nicolas Derumigny
 * \file executable_options.c
 * \brief Definition of the tools for the command-line options.
 *
 */

#include "executable_options.h"

int parse_argument(
		int   argc,
		char  *argv[],
		char  *env[])
{
	if (argc == 1)
		goto help;

	static struct option long_options[]={
		{"infile", required_argument, NULL, 'i'}
	};

	int opt;
	while ((opt = getopt_long(argc, argv, "i:", long_options, NULL)) != -1){
		switch (opt){
			case 'i':
			CommandLineOptions.sizePath = strlen(optarg);
			// We do not forget the final \0 char
			CommandLineOptions.infilePath = malloc((CommandLineOptions.sizePath+1)*sizeof(char));
			strcpy(CommandLineOptions.infilePath,optarg);
			break;

			default:
			help:
			fprintf(stderr, "Usage: %s {-i | --infile} infile\n", argv[0]);
			return EXIT_FAILURE;
			break;
		}
	}
	return EXIT_SUCCESS;
}