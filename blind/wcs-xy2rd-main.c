/*
  This file is part of the Astrometry.net suite.
  Copyright 2006-2008 Dustin Lang, Keir Mierle and Sam Roweis.

  The Astrometry.net suite is free software; you can redistribute
  it and/or modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation, version 2.

  The Astrometry.net suite is distributed in the hope that it will be
  useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with the Astrometry.net suite ; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "an-bool.h"
#include "bl.h"
#include "boilerplate.h"
#include "wcs-rd2xy.h"
#include "sip_qfits.h"
#include "sip.h"
#include "errors.h"

const char* OPTIONS = "hi:o:w:f:R:D:te:x:y:";

void print_help(char* progname) {
	boilerplate_help_header(stdout);
	printf("\nUsage: %s\n"
		   "   -w <WCS input file>\n"
		   "   [-e <extension>] FITS HDU number to read WCS from (default 0 = primary)\n"
		   "   -i <xyls input file>\n"
		   "   -o <rdls output file>\n"
		   "  [-f <xyls field index>] (default: all)\n"
		   "  [-X <x-column-name> -Y <y-column-name>]\n"
		   "  [-t]: just use TAN projection, even if SIP extension exists.\n"
		   "\n"
		   "You can also specify a single point to convert (result is printed to stdout):\n"
		   "  [-x <pixel>]\n"
		   "  [-y <pixel>]\n"
		   "\n", progname);
}

extern char *optarg;
extern int optind, opterr, optopt;

int main(int argc, char** args) {
	int c;
	char* rdlsfn = NULL;
	char* wcsfn = NULL;
	char* xylsfn = NULL;
	char* xcol = NULL;
	char* ycol = NULL;
	bool forcetan = FALSE;
	il* fields;
	int ext = 0;
	double x, y;

	x = y = HUGE_VAL;
	fields = il_new(16);

    while ((c = getopt(argc, args, OPTIONS)) != -1) {
        switch (c) {
		case 'x':
			x = atof(optarg);
			break;
		case 'y':
			y = atof(optarg);
			break;
		case 'e':
			ext = atoi(optarg);
			break;
        case 'h':
			print_help(args[0]);
			exit(0);
		case 't':
			forcetan = TRUE;
			break;
		case 'o':
			rdlsfn = optarg;
			break;
		case 'i':
			xylsfn = optarg;
			break;
		case 'w':
			wcsfn = optarg;
			break;
		case 'f':
			il_append(fields, atoi(optarg));
			break;
		case 'X':
			xcol = optarg;
			break;
		case 'Y':
			ycol = optarg;
			break;
		}
	}

	if (optind != argc) {
		print_help(args[0]);
		exit(-1);
	}

	if (!(wcsfn && ((rdlsfn && xylsfn) || ((x != HUGE_VAL) && (y != HUGE_VAL))))) {
		print_help(args[0]);
		exit(-1);
	}

	if (!xylsfn) {
		double ra,dec;
		sip_t sip;
		// read WCS.
		if (!sip_read_tan_or_sip_header_file_ext(wcsfn, ext, &sip, forcetan)) {
			ERROR("Failed to read WCS file");
			exit(-1);
		}
		// convert immediately.
		sip_pixelxy2radec(&sip, x, y, &ra, &dec);
		printf("Pixel (%f, %f) -> RA,Dec (%f, %f)\n", x, y, ra, dec);
		exit(0);
	}

    if (wcs_xy2rd(wcsfn, ext, xylsfn, rdlsfn, 
                  xcol, ycol, forcetan, fields)) {
        ERROR("wcs-xy2rd failed");
        exit(-1);
    }

	return 0;
}