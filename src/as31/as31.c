/* ----------------------------------------------------------------------
 * FILE: main.c
 * PACKAGE: as31 - 8031/8051 Assembler.
 *
 * DESCRIPTION:
 *	The file contains main(). It handles the arguments and makes
 *	sure that pass 1 is done before pass 2 etc...
 *
 * REVISION HISTORY:
 *	Jan. 19, 1990 - Created. (Ken Stauffer)
 *
 * AUTHOR:
 *	All code in this file written by Ken Stauffer (University of Calgary).
 *	January, 1990. "Written by: Ken Stauffer"
 *
 *	March, 2001, Paul Stoffregen: see Makefile for details
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "as31.h"



int main(int argc, char **argv)
{
	char *dashF=NULL, *dashA=NULL;
	int use_stdout=0, do_lst=0;
	int r, i;

	fprintf(stderr, "AS31 2.0b3 (beta), March 20, 2001\n");
	fprintf(stderr, "Please report problems to: paul@pjrc.com\n\n");


	if (argc < 2) {
		fprintf(stderr,
			"Usage: %s [-l] [-s] [-Ffmt] [-Aarg] file.asm\n",
			argv[0]);
		fprintf(stderr, "\t -l :  create list file\n");
		fprintf(stderr, "\t -s :  send output to stdout\n");
		fprintf(stderr, "\t -F :  output format (intel hex default)\n");
		fprintf(stderr, "\t -A :  optional output format argument\n");
		emitusage();
		exit(1);
	}

	for (i=1; i<argc; i++ ) {
		if( argv[i][0] != '-' ) break;
		if( argv[i][1] == 'l' )
			do_lst = 1;
		else if( argv[i][1] == 's' )
			use_stdout = 1;
		else if( dashF == NULL && argv[i][1] == 'F' )
			dashF = argv[i]+2;
		else if( dashA == NULL && argv[i][1] == 'A' )
			dashA = argv[i]+2;
		else {
			fprintf(stderr,"Duplicate or unknown flag.\n");
			exit(1);
		}
	}
	if (i == argc) {
		fprintf(stderr,"Missing input file.\n");
		exit(1);
	}

	r = run_as31(argv[i], do_lst, use_stdout, dashF, dashA);
	return r;
}


/* the assembler calls here to display any messages */

void mesg(const char *str)
{
	if (str == NULL) str = "(null)";
	fprintf(stderr, "%s", str);
}



