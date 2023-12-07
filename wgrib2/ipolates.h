/*
 * ipolates.h
 *
 * 6/2010 Public Domain Wesley Ebisuzaki
 *  extracted from New_grid.c
 *
 */

#ifndef _CONFIG_H

#include "config.h"
#define _CONFIG_H

#endif



#ifdef G95
#define IPOLATES ipolates_
#define IPOLATEV ipolatev_
void g95_runtime_start(int ,char **);
void g95_runtime_stop(void);
static int g95_runstop = 0;
#endif

#ifdef GFORTRAN
#define IPOLATES ipolates_
#define IPOLATEV ipolatev_
#endif

#ifdef OPENF95
#define IPOLATES ipolates_
#define IPOLATEV ipolatev_
#endif

#ifdef IFORT
#define IPOLATES ipolates_
#define IPOLATEV ipolatev_
#endif

#ifdef FLANG
#define IPOLATES ipolates_
#define IPOLATEV ipolatev_
#endif

#ifdef NVFORTRAN
#define IPOLATES ipolates_
#define IPOLATEV ipolatev_
#endif

#ifdef XLF
#define IPOLATES ipolates
#define IPOLATEV ipolatev
#endif

#ifdef CRAYCE
#define IPOLATES ipolates_
#define IPOLATEV ipolatev_
#endif

#ifdef SOLARIS
#define IPOLATES ipolates_
#define IPOLATEV ipolatev_
#endif

#if USE_IPOLATES == 1

void IPOLATES(int *interpol, int *ipopt, int *kgds, int *kgds_out, int *npnts, int *n_out0,
                int *km, int *ibi, unsigned char *bitmap, float *data_in, int *n_out,
                float *rlat, float *rlon, int *ibo, unsigned char *bitmap_out,
                float *data_out, int *iret);

void IPOLATEV(int *interpol, int *ipopt, int *kgds, int *kgds_out, int *npnts, int *n_out0,
                int *km, int *ibi, unsigned char *bitmap, float *u_in, float *v_in,
                int *n_out, float *rlat, float *rlon, float *crot, float *srot, int *ibo,
                unsigned char *bitmap_out, float *u_out, float *v_out, int *iret);

#endif



#if USE_IPOLATES == 3

void IPOLATES(int *interpol, int *ipopt, int *gdt_in, int *gdttmpl_in, int *gdttmpl_size_in,
  int *gdt_out, int *gdttmpl_out, int *gdttmpl_size_out, int *mi, int *mo, int *km,
  int *ibi, unsigned char *bitmap, double *data_in, int *n_out, double *rlat, double *rlon,
   int *ibo, unsigned char *bitmap_out, double *data_out, int *iret);

void IPOLATEV(int *interpol, int *ipopt, int *gdt_in, int *gdttmpl_in, int *gdttmpl_size_in,
  int *gdt_out, int *gdttmpl_out, int *gdttmpl_size_out, int *mi, int *mo, int *km,
  int *ibi, unsigned char *bitmap, double *u_in, double *v_in, int *n_out, double *rlat, double *rlon,
   double *crot, double *srot, int *ibo, unsigned char *bitmap_out,
   double *u_out, double *v_out, int *iret);

#endif


