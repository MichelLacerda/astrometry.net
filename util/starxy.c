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

#include <assert.h>
#include <string.h>
#include <math.h>
#include <sys/param.h>

#include "starxy.h"

void starxy_compute_range(starxy_t* xy) {
    int i, N;
    xy->xlo = xy->ylo =  HUGE_VAL;
    xy->xhi = xy->yhi = -HUGE_VAL;
    N = starxy_n(xy);
    for (i=0; i<N; i++) {
        xy->xlo = MIN(xy->xlo, starxy_getx(xy, i));
        xy->xhi = MAX(xy->xhi, starxy_getx(xy, i));
        xy->ylo = MIN(xy->ylo, starxy_gety(xy, i));
        xy->yhi = MAX(xy->yhi, starxy_gety(xy, i));
    }
}

double starxy_getx(const starxy_t* f, int i) {
    assert(i < f->N);
    return f->x[i];
}

double starxy_gety(const starxy_t* f, int i) {
    assert(i < f->N);
    return f->y[i];
}

void starxy_get(const starxy_t* f, int i, double* xy) {
    xy[0] = starxy_getx(f, i);
    xy[1] = starxy_gety(f, i);
}

void starxy_setx(starxy_t* f, int i, double val) {
    assert(i < f->N);
    f->x[i] = val;
}

void starxy_sety(starxy_t* f, int i, double val) {
    assert(i < f->N);
    f->y[i] = val;
}

void starxy_set(starxy_t* f, int i, double x, double y) {
    assert(i < f->N);
    f->x[i] = x;
    f->y[i] = y;
}

int starxy_n(const starxy_t* f) {
    return f->N;
}

void starxy_free_data(starxy_t* f) {
    if (!f) return;
    free(f->x);
    free(f->y);
    free(f->flux);
    free(f->background);
}

void starxy_free(starxy_t* f) {
    starxy_free_data(f);
    free(f);
}

double* starxy_copy_x(const starxy_t* xy) {
    double* res = malloc(sizeof(double) * starxy_n(xy));
    memcpy(res, xy->x, sizeof(double) * starxy_n(xy));
    return res;
}

double* starxy_copy_y(const starxy_t* xy) {
    double* res = malloc(sizeof(double) * starxy_n(xy));
    memcpy(res, xy->y, sizeof(double) * starxy_n(xy));
    return res;
}

double* starxy_copy_xy(const starxy_t* xy) {
    int i, N;
    double* res;
    N = starxy_n(xy);
    res = malloc(sizeof(double) * 2 * N);
    for (i=0; i<N; i++) {
        res[2*i + 0] = starxy_getx(xy, i);
        res[2*i + 1] = starxy_gety(xy, i);
    }
    return res;
}

starxy_t* starxy_alloc(int N, bool flux, bool back) {
    starxy_t* xy = calloc(1, sizeof(starxy_t));
    starxy_alloc_data(xy, N, flux, back);
    return xy;
}

void starxy_alloc_data(starxy_t* f, int N, bool flux, bool back) {
    f->x = malloc(N * sizeof(double));
    f->y = malloc(N * sizeof(double));
    if (flux)
        f->flux = malloc(N * sizeof(double));
    else
        f->flux = NULL;
    if (back)
        f->background = malloc(N * sizeof(double));
    else
        f->background = NULL;
    f->N = N;
}

double* starxy_to_flat_array(starxy_t* xy, double* arr) {
    int nr = 2;
    int i, ind;
    if (xy->flux)
        nr++;
    if (xy->background)
        nr++;

    if (!arr)
        arr = malloc(nr * starxy_n(xy) * sizeof(double));

    ind = 0;
    for (i=0; i<xy->N; i++) {
        arr[ind] = xy->x[i];
        ind++;
        arr[ind] = xy->y[i];
        ind++;
        if (xy->flux) {
            arr[ind] = xy->flux[i];
            ind++;
        }
        if (xy->background) {
            arr[ind] = xy->background[i];
            ind++;
        }
    }
    return arr;
}

void starxy_from_dl(starxy_t* xy, dl* l, bool flux, bool back) {
    int i;
    int nr = 2;
    int ind;
    if (flux)
        nr++;
    if (back)
        nr++;

    starxy_alloc_data(xy, dl_size(l)/nr, flux, back);
    ind = 0;
    for (i=0; i<xy->N; i++) {
        xy->x[i] = dl_get(l, ind);
        ind++;
        xy->y[i] = dl_get(l, ind);
        ind++;
        if (flux) {
            xy->flux[i] = dl_get(l, ind);
            ind++;
        }
        if (back) {
            xy->background[i] = dl_get(l, ind);
            ind++;
        }
    }
}

