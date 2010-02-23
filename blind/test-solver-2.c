/*
  This file is part of the Astrometry.net suite.
  Copyright 2007-2010 Dustin Lang

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
#include <assert.h>

#include "solver.h"
#include "index.h"
#include "pquad.h"
#include "permutedsort.h"
#include "log.h"

static int compare_n(const void* v1, const void* v2, int N) {
    const int* u1 = v1;
    const int* u2 = v2;
    int i;
    for (i=0; i<N; i++) {
        if (u1[i] < u2[i]) return -1;
        if (u1[i] > u2[i]) return 1;
    }
    return 0;
}

static int compare_tri(const void* v1, const void* v2) {
	return compare_n(v1, v2, 3);
}
static int compare_quad(const void* v1, const void* v2) {
	return compare_n(v1, v2, 4);
}
static int compare_quint(const void* v1, const void* v2) {
	return compare_n(v1, v2, 5);
}


bl* quadlist;

void test_try_permutations(int* stars, double* code, int dimquad, solver_t* s) {
	int i;
    fflush(NULL);
	/*
	 printf("test_try_permutations: [");
	 for (i=0; i<dimquad; i++) {
	 printf("%s%i", (i?" ":""), stars[i]);
	 }
	 printf("]\n");
	 */
	printf("{");
	for (i=0; i<dimquad; i++) {
		printf("%s%i", (i?",":""), stars[i]);
	}
	printf("}, ");
    fflush(NULL);
    bl_append(quadlist, stars);
}

static starxy_t* field1() {
    starxy_t* starxy;
    double field[14];
    int i=0, N;
    // star0 A: (0,0)
    field[i++] = 0.0;
    field[i++] = 0.0;
    // star1 B: (2,2)
    field[i++] = 2.0;
    field[i++] = 2.0;
    // star2
    field[i++] = -1.0;
    field[i++] = 3.0;
    // star3
    field[i++] = 0.5;
    field[i++] = 1.5;
    // star4
    field[i++] = 1.0;
    field[i++] = 1.0;
    // star5
    field[i++] = 1.5;
    field[i++] = 0.5;
    // star6
    field[i++] = 3.0;
    field[i++] = -1.0;

    N = i/2;
    starxy = starxy_new(N, FALSE, FALSE);
    for (i=0; i<N; i++) {
        starxy_setx(starxy, i, field[i*2+0]);
        starxy_sety(starxy, i, field[i*2+1]);
    }
	return starxy;
}

void test1() {
	int i;
    solver_t* solver;
    index_t index;
    starxy_t* starxy;
    int wanted[][4] = {{0, 1, 4, 3}, {0, 1, 3, 4}, {1, 0, 4, 3}, {1, 0, 3, 4}, {0, 1, 4, 3}, {0, 1, 3, 4}, {1, 0, 4, 3}, {1, 0, 3, 4}, {0, 2, 4, 3}, {0, 2, 3, 4}, {2, 0, 4, 3}, {2, 0, 3, 4}, {0, 2, 4, 3}, {0, 2, 3, 4}, {2, 0, 4, 3}, {2, 0, 3, 4}, {1, 2, 4, 3}, {1, 2, 3, 4}, {2, 1, 4, 3}, {2, 1, 3, 4}, {1, 2, 4, 3}, {1, 2, 3, 4}, {2, 1, 4, 3}, {2, 1, 3, 4}, {2, 5, 0, 1}, {2, 5, 1, 0}, {5, 2, 0, 1}, {5, 2, 1, 0}, {2, 5, 0, 1}, {2, 5, 1, 0}, {5, 2, 0, 1}, {5, 2, 1, 0}, {2, 5, 0, 3}, {2, 5, 3, 0}, {5, 2, 0, 3}, {5, 2, 3, 0}, {2, 5, 0, 3}, {2, 5, 3, 0}, {5, 2, 0, 3}, {5, 2, 3, 0}, {2, 5, 0, 4}, {2, 5, 4, 0}, {5, 2, 0, 4}, {5, 2, 4, 0}, {2, 5, 0, 4}, {2, 5, 4, 0}, {5, 2, 0, 4}, {5, 2, 4, 0}, {2, 5, 1, 3}, {2, 5, 3, 1}, {5, 2, 1, 3}, {5, 2, 3, 1}, {2, 5, 1, 3}, {2, 5, 3, 1}, {5, 2, 1, 3}, {5, 2, 3, 1}, {2, 5, 1, 4}, {2, 5, 4, 1}, {5, 2, 1, 4}, {5, 2, 4, 1}, {2, 5, 1, 4}, {2, 5, 4, 1}, {5, 2, 1, 4}, {5, 2, 4, 1}, {2, 5, 3, 4}, {2, 5, 4, 3}, {5, 2, 3, 4}, {5, 2, 4, 3}, {2, 5, 3, 4}, {2, 5, 4, 3}, {5, 2, 3, 4}, {5, 2, 4, 3}, {0, 1, 5, 3}, {0, 1, 3, 5}, {1, 0, 5, 3}, {1, 0, 3, 5}, {0, 1, 5, 3}, {0, 1, 3, 5}, {1, 0, 5, 3}, {1, 0, 3, 5}, {0, 1, 5, 4}, {0, 1, 4, 5}, {1, 0, 5, 4}, {1, 0, 4, 5}, {0, 1, 5, 4}, {0, 1, 4, 5}, {1, 0, 5, 4}, {1, 0, 4, 5}, {0, 6, 4, 5}, {0, 6, 5, 4}, {6, 0, 4, 5}, {6, 0, 5, 4}, {0, 6, 4, 5}, {0, 6, 5, 4}, {6, 0, 4, 5}, {6, 0, 5, 4}, {1, 6, 4, 5}, {1, 6, 5, 4}, {6, 1, 4, 5}, {6, 1, 5, 4}, {1, 6, 4, 5}, {1, 6, 5, 4}, {6, 1, 4, 5}, {6, 1, 5, 4}, {2, 6, 0, 1}, {2, 6, 1, 0}, {6, 2, 0, 1}, {6, 2, 1, 0}, {2, 6, 0, 1}, {2, 6, 1, 0}, {6, 2, 0, 1}, {6, 2, 1, 0}, {2, 6, 0, 3}, {2, 6, 3, 0}, {6, 2, 0, 3}, {6, 2, 3, 0}, {2, 6, 0, 3}, {2, 6, 3, 0}, {6, 2, 0, 3}, {6, 2, 3, 0}, {2, 6, 0, 4}, {2, 6, 4, 0}, {6, 2, 0, 4}, {6, 2, 4, 0}, {2, 6, 0, 4}, {2, 6, 4, 0}, {6, 2, 0, 4}, {6, 2, 4, 0}, {2, 6, 0, 5}, {2, 6, 5, 0}, {6, 2, 0, 5}, {6, 2, 5, 0}, {2, 6, 0, 5}, {2, 6, 5, 0}, {6, 2, 0, 5}, {6, 2, 5, 0}, {2, 6, 1, 3}, {2, 6, 3, 1}, {6, 2, 1, 3}, {6, 2, 3, 1}, {2, 6, 1, 3}, {2, 6, 3, 1}, {6, 2, 1, 3}, {6, 2, 3, 1}, {2, 6, 1, 4}, {2, 6, 4, 1}, {6, 2, 1, 4}, {6, 2, 4, 1}, {2, 6, 1, 4}, {2, 6, 4, 1}, {6, 2, 1, 4}, {6, 2, 4, 1}, {2, 6, 1, 5}, {2, 6, 5, 1}, {6, 2, 1, 5}, {6, 2, 5, 1}, {2, 6, 1, 5}, {2, 6, 5, 1}, {6, 2, 1, 5}, {6, 2, 5, 1}, {2, 6, 3, 4}, {2, 6, 4, 3}, {6, 2, 3, 4}, {6, 2, 4, 3}, {2, 6, 3, 4}, {2, 6, 4, 3}, {6, 2, 3, 4}, {6, 2, 4, 3}, {2, 6, 3, 5}, {2, 6, 5, 3}, {6, 2, 3, 5}, {6, 2, 5, 3}, {2, 6, 3, 5}, {2, 6, 5, 3}, {6, 2, 3, 5}, {6, 2, 5, 3}, {2, 6, 4, 5}, {2, 6, 5, 4}, {6, 2, 4, 5}, {6, 2, 5, 4}, {2, 6, 4, 5}, {2, 6, 5, 4}, {6, 2, 4, 5}, {6, 2, 5, 4}, {3, 6, 0, 1}, {3, 6, 1, 0}, {6, 3, 0, 1}, {6, 3, 1, 0}, {3, 6, 0, 1}, {3, 6, 1, 0}, {6, 3, 0, 1}, {6, 3, 1, 0}, {3, 6, 0, 4}, {3, 6, 4, 0}, {6, 3, 0, 4}, {6, 3, 4, 0}, {3, 6, 0, 4}, {3, 6, 4, 0}, {6, 3, 0, 4}, {6, 3, 4, 0}, {3, 6, 0, 5}, {3, 6, 5, 0}, {6, 3, 0, 5}, {6, 3, 5, 0}, {3, 6, 0, 5}, {3, 6, 5, 0}, {6, 3, 0, 5}, {6, 3, 5, 0}, {3, 6, 1, 4}, {3, 6, 4, 1}, {6, 3, 1, 4}, {6, 3, 4, 1}, {3, 6, 1, 4}, {3, 6, 4, 1}, {6, 3, 1, 4}, {6, 3, 4, 1}, {3, 6, 1, 5}, {3, 6, 5, 1}, {6, 3, 1, 5}, {6, 3, 5, 1}, {3, 6, 1, 5}, {3, 6, 5, 1}, {6, 3, 1, 5}, {6, 3, 5, 1}, {3, 6, 4, 5}, {3, 6, 5, 4}, {6, 3, 4, 5}, {6, 3, 5, 4}, {3, 6, 4, 5}, {3, 6, 5, 4}, {6, 3, 4, 5}, {6, 3, 5, 4}};

	starxy = field1();

    quadlist = bl_new(16, 4*sizeof(uint));

    solver = solver_new();

    memset(&index, 0, sizeof(index_t));
    index.index_scale_lower = 1;
    index.index_scale_upper = 10;
	index.dimquads = 4;

    solver->funits_lower = 0.1;
    solver->funits_upper = 10;

    solver_add_index(solver, &index);
    solver_set_field(solver, starxy);
    solver_preprocess_field(solver);

    solver_run(solver);
	printf("\n");
	fflush(NULL);
    solver_free_field(solver);
    solver_free(solver);

    //
    assert(bl_size(quadlist) == (sizeof(wanted) / (4*sizeof(uint))));
    for (i=0; i<bl_size(quadlist); i++) {
        assert(compare_quad(bl_access(quadlist, i), wanted[i]) == 0);
    }

    bl_free(quadlist);

    starxy_free(starxy);
}

void test2() {
	int i;
    solver_t* solver;
    index_t index;
    starxy_t* starxy;
	int dimquads = 3;
    int wanted[][3] = {
		{0,1,3}, {1,0,3}, {0,1,3}, {1,0,3}, {0,2,3}, {2,0,3}, {0,2,3}, {2,0,3}, {1,2,3}, {2,1,3}, {1,2,3}, {2,1,3}, {2,4,3}, {4,2,3}, {2,4,3}, {4,2,3}, {0,1,4}, {1,0,4}, {0,1,4}, {1,0,4}, {0,2,4}, {2,0,4}, {0,2,4}, {2,0,4}, {0,3,4}, {3,0,4}, {0,3,4}, {3,0,4}, {1,2,4}, {2,1,4}, {1,2,4}, {2,1,4}, {1,3,4}, {3,1,4}, {1,3,4}, {3,1,4}, {0,5,4}, {5,0,4}, {0,5,4}, {5,0,4}, {1,5,4}, {5,1,4}, {1,5,4}, {5,1,4}, {2,5,0}, {5,2,0}, {2,5,0}, {5,2,0}, {2,5,1}, {5,2,1}, {2,5,1}, {5,2,1}, {2,5,3}, {5,2,3}, {2,5,3}, {5,2,3}, {2,5,4}, {5,2,4}, {2,5,4}, {5,2,4}, {3,5,4}, {5,3,4}, {3,5,4}, {5,3,4}, {0,1,5}, {1,0,5}, {0,1,5}, {1,0,5}, {0,6,4}, {6,0,4}, {0,6,4}, {6,0,4}, {0,6,5}, {6,0,5}, {0,6,5}, {6,0,5}, {1,6,4}, {6,1,4}, {1,6,4}, {6,1,4}, {1,6,5}, {6,1,5}, {1,6,5}, {6,1,5}, {2,6,0}, {6,2,0}, {2,6,0}, {6,2,0}, {2,6,1}, {6,2,1}, {2,6,1}, {6,2,1}, {2,6,3}, {6,2,3}, {2,6,3}, {6,2,3}, {2,6,4}, {6,2,4}, {2,6,4}, {6,2,4}, {2,6,5}, {6,2,5}, {2,6,5}, {6,2,5}, {3,6,0}, {6,3,0}, {3,6,0}, {6,3,0}, {3,6,1}, {6,3,1}, {3,6,1}, {6,3,1}, {3,6,4}, {6,3,4}, {3,6,4}, {6,3,4}, {3,6,5}, {6,3,5}, {3,6,5}, {6,3,5}, {4,6,5}, {6,4,5}, {4,6,5}, {6,4,5},
	};

	starxy = field1();
    quadlist = bl_new(16, 3*sizeof(uint));
    solver = solver_new();
    memset(&index, 0, sizeof(index_t));
    index.index_scale_lower = 1;
    index.index_scale_upper = 10;
	index.dimquads = 3;

    solver->funits_lower = 0.1;
    solver->funits_upper = 10;

    solver_add_index(solver, &index);
    solver_set_field(solver, starxy);
    solver_preprocess_field(solver);

    solver_run(solver);
	printf("\n");
	fflush(NULL);

    solver_free_field(solver);
    solver_free(solver);

    //
    assert(bl_size(quadlist) == (sizeof(wanted) / (3*sizeof(uint))));
	bl_sort(quadlist, compare_tri);
	qsort(wanted, sizeof(wanted)/(dimquads*sizeof(int)), dimquads*sizeof(int), compare_tri);
    for (i=0; i<bl_size(quadlist); i++) {
		//int* i1 = bl_access(quadlist, i);
		//int* i2 = wanted[i];
		//printf("[%i, %i, %i] vs [%i, %i, %i]\n", i1[0],i1[1],i1[2], i2[0],i2[1],i2[2]);
        assert(compare_tri(bl_access(quadlist, i), wanted[i]) == 0);
    }
    bl_free(quadlist);
    starxy_free(starxy);
}

void test3() {
	int i;
    solver_t* solver;
    index_t index;
    starxy_t* starxy;
    int wanted[][5] = {
{2,5,0,1,3}, {2,5,0,3,1}, {2,5,1,0,3}, {2,5,1,3,0}, {2,5,3,0,1}, {2,5,3,1,0}, {5,2,0,1,3}, {5,2,0,3,1}, {5,2,1,0,3}, {5,2,1,3,0}, {5,2,3,0,1}, {5,2,3,1,0}, {2,5,0,1,3}, {2,5,0,3,1}, {2,5,1,0,3}, {2,5,1,3,0}, {2,5,3,0,1}, {2,5,3,1,0}, {5,2,0,1,3}, {5,2,0,3,1}, {5,2,1,0,3}, {5,2,1,3,0}, {5,2,3,0,1}, {5,2,3,1,0}, {2,5,0,1,4}, {2,5,0,4,1}, {2,5,1,0,4}, {2,5,1,4,0}, {2,5,4,0,1}, {2,5,4,1,0}, {5,2,0,1,4}, {5,2,0,4,1}, {5,2,1,0,4}, {5,2,1,4,0}, {5,2,4,0,1}, {5,2,4,1,0}, {2,5,0,1,4}, {2,5,0,4,1}, {2,5,1,0,4}, {2,5,1,4,0}, {2,5,4,0,1}, {2,5,4,1,0}, {5,2,0,1,4}, {5,2,0,4,1}, {5,2,1,0,4}, {5,2,1,4,0}, {5,2,4,0,1}, {5,2,4,1,0}, {2,5,0,3,4}, {2,5,0,4,3}, {2,5,3,0,4}, {2,5,3,4,0}, {2,5,4,0,3}, {2,5,4,3,0}, {5,2,0,3,4}, {5,2,0,4,3}, {5,2,3,0,4}, {5,2,3,4,0}, {5,2,4,0,3}, {5,2,4,3,0}, {2,5,0,3,4}, {2,5,0,4,3}, {2,5,3,0,4}, {2,5,3,4,0}, {2,5,4,0,3}, {2,5,4,3,0}, {5,2,0,3,4}, {5,2,0,4,3}, {5,2,3,0,4}, {5,2,3,4,0}, {5,2,4,0,3}, {5,2,4,3,0}, {2,5,1,3,4}, {2,5,1,4,3}, {2,5,3,1,4}, {2,5,3,4,1}, {2,5,4,1,3}, {2,5,4,3,1}, {5,2,1,3,4}, {5,2,1,4,3}, {5,2,3,1,4}, {5,2,3,4,1}, {5,2,4,1,3}, {5,2,4,3,1}, {2,5,1,3,4}, {2,5,1,4,3}, {2,5,3,1,4}, {2,5,3,4,1}, {2,5,4,1,3}, {2,5,4,3,1}, {5,2,1,3,4}, {5,2,1,4,3}, {5,2,3,1,4}, {5,2,3,4,1}, {5,2,4,1,3}, {5,2,4,3,1}, {0,1,5,3,4}, {0,1,5,4,3}, {0,1,3,5,4}, {0,1,3,4,5}, {0,1,4,5,3}, {0,1,4,3,5}, {1,0,5,3,4}, {1,0,5,4,3}, {1,0,3,5,4}, {1,0,3,4,5}, {1,0,4,5,3}, {1,0,4,3,5}, {0,1,5,3,4}, {0,1,5,4,3}, {0,1,3,5,4}, {0,1,3,4,5}, {0,1,4,5,3}, {0,1,4,3,5}, {1,0,5,3,4}, {1,0,5,4,3}, {1,0,3,5,4}, {1,0,3,4,5}, {1,0,4,5,3}, {1,0,4,3,5}, {2,6,0,1,3}, {2,6,0,3,1}, {2,6,1,0,3}, {2,6,1,3,0}, {2,6,3,0,1}, {2,6,3,1,0}, {6,2,0,1,3}, {6,2,0,3,1}, {6,2,1,0,3}, {6,2,1,3,0}, {6,2,3,0,1}, {6,2,3,1,0}, {2,6,0,1,3}, {2,6,0,3,1}, {2,6,1,0,3}, {2,6,1,3,0}, {2,6,3,0,1}, {2,6,3,1,0}, {6,2,0,1,3}, {6,2,0,3,1}, {6,2,1,0,3}, {6,2,1,3,0}, {6,2,3,0,1}, {6,2,3,1,0}, {2,6,0,1,4}, {2,6,0,4,1}, {2,6,1,0,4}, {2,6,1,4,0}, {2,6,4,0,1}, {2,6,4,1,0}, {6,2,0,1,4}, {6,2,0,4,1}, {6,2,1,0,4}, {6,2,1,4,0}, {6,2,4,0,1}, {6,2,4,1,0}, {2,6,0,1,4}, {2,6,0,4,1}, {2,6,1,0,4}, {2,6,1,4,0}, {2,6,4,0,1}, {2,6,4,1,0}, {6,2,0,1,4}, {6,2,0,4,1}, {6,2,1,0,4}, {6,2,1,4,0}, {6,2,4,0,1}, {6,2,4,1,0}, {2,6,0,1,5}, {2,6,0,5,1}, {2,6,1,0,5}, {2,6,1,5,0}, {2,6,5,0,1}, {2,6,5,1,0}, {6,2,0,1,5}, {6,2,0,5,1}, {6,2,1,0,5}, {6,2,1,5,0}, {6,2,5,0,1}, {6,2,5,1,0}, {2,6,0,1,5}, {2,6,0,5,1}, {2,6,1,0,5}, {2,6,1,5,0}, {2,6,5,0,1}, {2,6,5,1,0}, {6,2,0,1,5}, {6,2,0,5,1}, {6,2,1,0,5}, {6,2,1,5,0}, {6,2,5,0,1}, {6,2,5,1,0}, {2,6,0,3,4}, {2,6,0,4,3}, {2,6,3,0,4}, {2,6,3,4,0}, {2,6,4,0,3}, {2,6,4,3,0}, {6,2,0,3,4}, {6,2,0,4,3}, {6,2,3,0,4}, {6,2,3,4,0}, {6,2,4,0,3}, {6,2,4,3,0}, {2,6,0,3,4}, {2,6,0,4,3}, {2,6,3,0,4}, {2,6,3,4,0}, {2,6,4,0,3}, {2,6,4,3,0}, {6,2,0,3,4}, {6,2,0,4,3}, {6,2,3,0,4}, {6,2,3,4,0}, {6,2,4,0,3}, {6,2,4,3,0}, {2,6,0,3,5}, {2,6,0,5,3}, {2,6,3,0,5}, {2,6,3,5,0}, {2,6,5,0,3}, {2,6,5,3,0}, {6,2,0,3,5}, {6,2,0,5,3}, {6,2,3,0,5}, {6,2,3,5,0}, {6,2,5,0,3}, {6,2,5,3,0}, {2,6,0,3,5}, {2,6,0,5,3}, {2,6,3,0,5}, {2,6,3,5,0}, {2,6,5,0,3}, {2,6,5,3,0}, {6,2,0,3,5}, {6,2,0,5,3}, {6,2,3,0,5}, {6,2,3,5,0}, {6,2,5,0,3}, {6,2,5,3,0}, {2,6,0,4,5}, {2,6,0,5,4}, {2,6,4,0,5}, {2,6,4,5,0}, {2,6,5,0,4}, {2,6,5,4,0}, {6,2,0,4,5}, {6,2,0,5,4}, {6,2,4,0,5}, {6,2,4,5,0}, {6,2,5,0,4}, {6,2,5,4,0}, {2,6,0,4,5}, {2,6,0,5,4}, {2,6,4,0,5}, {2,6,4,5,0}, {2,6,5,0,4}, {2,6,5,4,0}, {6,2,0,4,5}, {6,2,0,5,4}, {6,2,4,0,5}, {6,2,4,5,0}, {6,2,5,0,4}, {6,2,5,4,0}, {2,6,1,3,4}, {2,6,1,4,3}, {2,6,3,1,4}, {2,6,3,4,1}, {2,6,4,1,3}, {2,6,4,3,1}, {6,2,1,3,4}, {6,2,1,4,3}, {6,2,3,1,4}, {6,2,3,4,1}, {6,2,4,1,3}, {6,2,4,3,1}, {2,6,1,3,4}, {2,6,1,4,3}, {2,6,3,1,4}, {2,6,3,4,1}, {2,6,4,1,3}, {2,6,4,3,1}, {6,2,1,3,4}, {6,2,1,4,3}, {6,2,3,1,4}, {6,2,3,4,1}, {6,2,4,1,3}, {6,2,4,3,1}, {2,6,1,3,5}, {2,6,1,5,3}, {2,6,3,1,5}, {2,6,3,5,1}, {2,6,5,1,3}, {2,6,5,3,1}, {6,2,1,3,5}, {6,2,1,5,3}, {6,2,3,1,5}, {6,2,3,5,1}, {6,2,5,1,3}, {6,2,5,3,1}, {2,6,1,3,5}, {2,6,1,5,3}, {2,6,3,1,5}, {2,6,3,5,1}, {2,6,5,1,3}, {2,6,5,3,1}, {6,2,1,3,5}, {6,2,1,5,3}, {6,2,3,1,5}, {6,2,3,5,1}, {6,2,5,1,3}, {6,2,5,3,1}, {2,6,1,4,5}, {2,6,1,5,4}, {2,6,4,1,5}, {2,6,4,5,1}, {2,6,5,1,4}, {2,6,5,4,1}, {6,2,1,4,5}, {6,2,1,5,4}, {6,2,4,1,5}, {6,2,4,5,1}, {6,2,5,1,4}, {6,2,5,4,1}, {2,6,1,4,5}, {2,6,1,5,4}, {2,6,4,1,5}, {2,6,4,5,1}, {2,6,5,1,4}, {2,6,5,4,1}, {6,2,1,4,5}, {6,2,1,5,4}, {6,2,4,1,5}, {6,2,4,5,1}, {6,2,5,1,4}, {6,2,5,4,1}, {2,6,3,4,5}, {2,6,3,5,4}, {2,6,4,3,5}, {2,6,4,5,3}, {2,6,5,3,4}, {2,6,5,4,3}, {6,2,3,4,5}, {6,2,3,5,4}, {6,2,4,3,5}, {6,2,4,5,3}, {6,2,5,3,4}, {6,2,5,4,3}, {2,6,3,4,5}, {2,6,3,5,4}, {2,6,4,3,5}, {2,6,4,5,3}, {2,6,5,3,4}, {2,6,5,4,3}, {6,2,3,4,5}, {6,2,3,5,4}, {6,2,4,3,5}, {6,2,4,5,3}, {6,2,5,3,4}, {6,2,5,4,3}, {3,6,0,1,4}, {3,6,0,4,1}, {3,6,1,0,4}, {3,6,1,4,0}, {3,6,4,0,1}, {3,6,4,1,0}, {6,3,0,1,4}, {6,3,0,4,1}, {6,3,1,0,4}, {6,3,1,4,0}, {6,3,4,0,1}, {6,3,4,1,0}, {3,6,0,1,4}, {3,6,0,4,1}, {3,6,1,0,4}, {3,6,1,4,0}, {3,6,4,0,1}, {3,6,4,1,0}, {6,3,0,1,4}, {6,3,0,4,1}, {6,3,1,0,4}, {6,3,1,4,0}, {6,3,4,0,1}, {6,3,4,1,0}, {3,6,0,1,5}, {3,6,0,5,1}, {3,6,1,0,5}, {3,6,1,5,0}, {3,6,5,0,1}, {3,6,5,1,0}, {6,3,0,1,5}, {6,3,0,5,1}, {6,3,1,0,5}, {6,3,1,5,0}, {6,3,5,0,1}, {6,3,5,1,0}, {3,6,0,1,5}, {3,6,0,5,1}, {3,6,1,0,5}, {3,6,1,5,0}, {3,6,5,0,1}, {3,6,5,1,0}, {6,3,0,1,5}, {6,3,0,5,1}, {6,3,1,0,5}, {6,3,1,5,0}, {6,3,5,0,1}, {6,3,5,1,0}, {3,6,0,4,5}, {3,6,0,5,4}, {3,6,4,0,5}, {3,6,4,5,0}, {3,6,5,0,4}, {3,6,5,4,0}, {6,3,0,4,5}, {6,3,0,5,4}, {6,3,4,0,5}, {6,3,4,5,0}, {6,3,5,0,4}, {6,3,5,4,0}, {3,6,0,4,5}, {3,6,0,5,4}, {3,6,4,0,5}, {3,6,4,5,0}, {3,6,5,0,4}, {3,6,5,4,0}, {6,3,0,4,5}, {6,3,0,5,4}, {6,3,4,0,5}, {6,3,4,5,0}, {6,3,5,0,4}, {6,3,5,4,0}, {3,6,1,4,5}, {3,6,1,5,4}, {3,6,4,1,5}, {3,6,4,5,1}, {3,6,5,1,4}, {3,6,5,4,1}, {6,3,1,4,5}, {6,3,1,5,4}, {6,3,4,1,5}, {6,3,4,5,1}, {6,3,5,1,4}, {6,3,5,4,1}, {3,6,1,4,5}, {3,6,1,5,4}, {3,6,4,1,5}, {3,6,4,5,1}, {3,6,5,1,4}, {3,6,5,4,1}, {6,3,1,4,5}, {6,3,1,5,4}, {6,3,4,1,5}, {6,3,4,5,1}, {6,3,5,1,4}, {6,3,5,4,1}
	};

	starxy = field1();
    quadlist = bl_new(16, 5*sizeof(uint));
    solver = solver_new();
    memset(&index, 0, sizeof(index_t));
    index.index_scale_lower = 1;
    index.index_scale_upper = 10;
	index.dimquads = 5;

    solver->funits_lower = 0.1;
    solver->funits_upper = 10;

    solver_add_index(solver, &index);
    solver_set_field(solver, starxy);
    solver_preprocess_field(solver);

    solver_run(solver);
	printf("\n");
	fflush(NULL);

    solver_free_field(solver);
    solver_free(solver);

    //
    assert(bl_size(quadlist) == (sizeof(wanted) / (5*sizeof(uint))));
    for (i=0; i<bl_size(quadlist); i++) {
		//int* i1 = bl_access(quadlist, i);
		//int* i2 = wanted[i];
		//printf("[%i, %i, %i] vs [%i, %i, %i]\n", i1[0],i1[1],i1[2], i2[0],i2[1],i2[2]);
        assert(compare_quint(bl_access(quadlist, i), wanted[i]) == 0);
    }
    bl_free(quadlist);
    starxy_free(starxy);
}


char* OPTIONS = "v";

int main(int argc, char** args) {
    int argchar;

	while ((argchar = getopt(argc, args, OPTIONS)) != -1)
		switch (argchar) {
        case 'v':
            log_init(LOG_ALL+1);
            break;
        }

    test1();
    test2();
    test3();
    return 0;
}

