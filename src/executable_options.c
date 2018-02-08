/** 
 * \author Daumen Anton and Nicolas Derumigny
 * \file executable_options.c
 * \brief Definition of the tools for the command-line options.
 *
 */

#include "executable_options.h"

CommandLineOptions_t commandLineOptions;

void parse_argument(
		int   argc,
		char  *argv[],
		char  *env[])
{
	int ret =  EXIT_FAILURE;
	int my_rank; MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

	commandLineOptions.sizePath = 0;
	commandLineOptions.num = 0;

	static struct option long_options[]={
		{"infile", required_argument, NULL, 'i'},
		{"num",    required_argument, NULL, 'n'},
		{"help",   no_argument,       NULL, 'h'}
	};

	int opt;
	while ((opt = getopt_long(argc, argv, "i:n:h", long_options, NULL)) != -1)
	{
		switch (opt)
		{
			case 'i':
			commandLineOptions.sizePath = strlen(optarg);
			// We do not forget the final \0 char
			commandLineOptions.infilePath = malloc((commandLineOptions.sizePath+1)*sizeof(char));
			strcpy(commandLineOptions.infilePath, optarg);
			break;

			case 'n':
			errno = 0;
			commandLineOptions.num = strtoll(optarg, NULL, 10);
			if (errno || strtoll(optarg, NULL, 10) <= 0)
			{
				goto help;
			}
			break;

			case 'h':
			ret = EXIT_SUCCESS;
			goto help;
			break;

			default:
			help:
			if (my_rank == 0)
				fprintf(stderr, "Usage: mpirun -n num_process %s {-i | --infile} infile {-n | --num} number_of_eigenvalues [-h]\n", argv[0]);
			exit(ret);
			break;
		}
	}
	if (commandLineOptions.sizePath == 0 || commandLineOptions.num == 0)
	{
		goto help;
	}
}