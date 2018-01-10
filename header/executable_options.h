/** 
 * \author Daumen Anton and Nicolas Derumigny
 * \file executable_options.h
 * \brief Definition of the tools for the command-line options.
 *
 */

#ifndef _EXECUTABLE_OPTION_H
#define _EXECUTABLE_OPTION_H

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

/// \brief Structure of command line options for the executable
struct CommandLineOptions_t
{
	/// Size of the \a infilePath array
	unsigned sizePath;
	/// Array of the path to the file descripting the matrix
	char     *infilePath;
	// Number of vectors to compute
	unsigned long long num;
};

typedef struct CommandLineOptions_t CommandLineOptions_t;

/// \brief Global variable of the parameters of the program
extern CommandLineOptions_t commandLineOptions;

/// \brief Fill the \a CommandLineOption global variable with the given arguments
void parse_argument(
	int   argc,
	char  *argv[],
	char  *env[]);

#endif