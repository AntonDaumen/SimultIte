/** 
 * \author Daumen Anton and Nicolas Derumigny
 * \file main.c
 * \brief Definition of the \a main() function for the SimultIte executable.
 *
 */
#include "executable_options.h"
#include "matrix_reader.h"

CommandLineOptions_t CommandLineOptions;

/**
 * \brief Main Function
 */
int main(
		int argc,
		char *argv[],
		char *env[])
{
	parse_argument(argc, argv, env);
    read_Matrix(CommandLineOptions.infilePath);
	return 0;
}
