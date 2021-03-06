/*  This file is part of the program psim.

    Copyright (C) 1994-1995, Andrew Cagney <cagney@highland.com.au>
    Copyright (C) 1997, Free Software Foundation, Inc.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
 
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 
    */


#ifndef _SIM_ENDIAN_H_
#define _SIM_ENDIAN_H_


/* C byte conversion functions */

INLINE_SIM_ENDIAN(unsigned_1) endian_h2t_1(unsigned_1 x);
INLINE_SIM_ENDIAN(unsigned_2) endian_h2t_2(unsigned_2 x);
INLINE_SIM_ENDIAN(unsigned_4) endian_h2t_4(unsigned_4 x);
INLINE_SIM_ENDIAN(unsigned_8) endian_h2t_8(unsigned_8 x);
INLINE_SIM_ENDIAN(unsigned_16) endian_h2t_16(unsigned_16 x);

INLINE_SIM_ENDIAN(unsigned_1) endian_t2h_1(unsigned_1 x);
INLINE_SIM_ENDIAN(unsigned_2) endian_t2h_2(unsigned_2 x);
INLINE_SIM_ENDIAN(unsigned_4) endian_t2h_4(unsigned_4 x);
INLINE_SIM_ENDIAN(unsigned_8) endian_t2h_8(unsigned_8 x);
INLINE_SIM_ENDIAN(unsigned_16) endian_t2h_16(unsigned_16 x);

INLINE_SIM_ENDIAN(unsigned_1) swap_1(unsigned_1 x);
INLINE_SIM_ENDIAN(unsigned_2) swap_2(unsigned_2 x);
INLINE_SIM_ENDIAN(unsigned_4) swap_4(unsigned_4 x);
INLINE_SIM_ENDIAN(unsigned_8) swap_8(unsigned_8 x);
INLINE_SIM_ENDIAN(unsigned_16) swap_16(unsigned_16 x);

INLINE_SIM_ENDIAN(unsigned_1) endian_h2be_1(unsigned_1 x);
INLINE_SIM_ENDIAN(unsigned_2) endian_h2be_2(unsigned_2 x);
INLINE_SIM_ENDIAN(unsigned_4) endian_h2be_4(unsigned_4 x);
INLINE_SIM_ENDIAN(unsigned_8) endian_h2be_8(unsigned_8 x);
INLINE_SIM_ENDIAN(unsigned_16) endian_h2be_16(unsigned_16 x);

INLINE_SIM_ENDIAN(unsigned_1) endian_be2h_1(unsigned_1 x);
INLINE_SIM_ENDIAN(unsigned_2) endian_be2h_2(unsigned_2 x);
INLINE_SIM_ENDIAN(unsigned_4) endian_be2h_4(unsigned_4 x);
INLINE_SIM_ENDIAN(unsigned_8) endian_be2h_8(unsigned_8 x);
INLINE_SIM_ENDIAN(unsigned_16) endian_be2h_16(unsigned_16 x);

INLINE_SIM_ENDIAN(unsigned_1) endian_h2le_1(unsigned_1 x);
INLINE_SIM_ENDIAN(unsigned_2) endian_h2le_2(unsigned_2 x);
INLINE_SIM_ENDIAN(unsigned_4) endian_h2le_4(unsigned_4 x);
INLINE_SIM_ENDIAN(unsigned_8) endian_h2le_8(unsigned_8 x);
INLINE_SIM_ENDIAN(unsigned_16) endian_h2le_16(unsigned_16 x);

INLINE_SIM_ENDIAN(unsigned_1) endian_le2h_1(unsigned_1 x);
INLINE_SIM_ENDIAN(unsigned_2) endian_le2h_2(unsigned_2 x);
INLINE_SIM_ENDIAN(unsigned_4) endian_le2h_4(unsigned_4 x);
INLINE_SIM_ENDIAN(unsigned_8) endian_le2h_8(unsigned_8 x);
INLINE_SIM_ENDIAN(unsigned_16) endian_le2h_16(unsigned_16 x);

INLINE_SIM_ENDIAN(void*) offset_1(unsigned_1 *x, unsigned ws, unsigned w);
INLINE_SIM_ENDIAN(void*) offset_2(unsigned_2 *x, unsigned ws, unsigned w);
INLINE_SIM_ENDIAN(void*) offset_4(unsigned_4 *x, unsigned ws, unsigned w);
INLINE_SIM_ENDIAN(void*) offset_8(unsigned_8 *x, unsigned ws, unsigned w);
INLINE_SIM_ENDIAN(void*) offset_16(unsigned_16 *x, unsigned ws, unsigned w);

INLINE_SIM_ENDIAN(unsigned_16) sim_endian_join_16 (unsigned_8 h, unsigned_8 l);
INLINE_SIM_ENDIAN(unsigned_8) sim_endian_split_16 (unsigned_16 word, int w);


/* SWAP */

#define SWAP_1(X) swap_1(X)
#define SWAP_2(X) swap_2(X)
#define SWAP_4(X) swap_4(X)
#define SWAP_8(X) swap_8(X)
#define SWAP_16(X) swap_16(X)


/* HOST to BE */

#define H2BE_1(X) endian_h2be_1(X)
#define H2BE_2(X) endian_h2be_2(X)
#define H2BE_4(X) endian_h2be_4(X)
#define H2BE_8(X) endian_h2be_8(X)
#define H2BE_16(X) endian_h2be_16(X)
#define BE2H_1(X) endian_be2h_1(X)
#define BE2H_2(X) endian_be2h_2(X)
#define BE2H_4(X) endian_be2h_4(X)
#define BE2H_8(X) endian_be2h_8(X)
#define BE2H_16(X) endian_be2h_16(X)


/* HOST to LE */

#define H2LE_1(X) endian_h2le_1(X)
#define H2LE_2(X) endian_h2le_2(X)
#define H2LE_4(X) endian_h2le_4(X)
#define H2LE_8(X) endian_h2le_8(X)
#define H2LE_16(X) endian_h2le_16(X)
#define LE2H_1(X) endian_le2h_1(X)
#define LE2H_2(X) endian_le2h_2(X)
#define LE2H_4(X) endian_le2h_4(X)
#define LE2H_8(X) endian_le2h_8(X)
#define LE2H_16(X) endian_le2h_16(X)


/* HOST to TARGET */

#define H2T_1(X) endian_h2t_1(X)
#define H2T_2(X) endian_h2t_2(X)
#define H2T_4(X) endian_h2t_4(X)
#define H2T_8(X) endian_h2t_8(X)
#define H2T_16(X) endian_h2t_16(X)
#define T2H_1(X) endian_t2h_1(X)
#define T2H_2(X) endian_t2h_2(X)
#define T2H_4(X) endian_t2h_4(X)
#define T2H_8(X) endian_t2h_8(X)
#define T2H_16(X) endian_t2h_16(X)


/* CONVERT IN PLACE

   These macros, given an argument of unknown size, swap its value in
   place if a host/target conversion is required. */

#define H2T(VARIABLE) \
do { \
  void *vp = &(VARIABLE); \
  switch (sizeof (VARIABLE)) { \
  case 1: *(unsigned_1*)vp = H2T_1(*(unsigned_1*)vp); break; \
  case 2: *(unsigned_2*)vp = H2T_2(*(unsigned_2*)vp); break; \
  case 4: *(unsigned_4*)vp = H2T_4(*(unsigned_4*)vp); break; \
  case 8: *(unsigned_8*)vp = H2T_8(*(unsigned_8*)vp); break; \
  case 16: *(unsigned_16*)vp = H2T_16(*(unsigned_16*)vp); break; \
  } \
} while (0)

#define T2H(VARIABLE) \
do { \
  switch (sizeof(VARIABLE)) { \
  case 1: VARIABLE = T2H_1(VARIABLE); break; \
  case 2: VARIABLE = T2H_2(VARIABLE); break; \
  case 4: VARIABLE = T2H_4(VARIABLE); break; \
  case 8: VARIABLE = T2H_8(VARIABLE); break; \
  /*case 16: VARIABLE = T2H_16(VARIABLE); break;*/ \
  } \
} while (0)

#define SWAP(VARIABLE) \
do { \
  switch (sizeof(VARIABLE)) { \
  case 1: VARIABLE = SWAP_1(VARIABLE); break; \
  case 2: VARIABLE = SWAP_2(VARIABLE); break; \
  case 4: VARIABLE = SWAP_4(VARIABLE); break; \
  case 8: VARIABLE = SWAP_8(VARIABLE); break; \
  /*case 16: VARIABLE = SWAP_16(VARIABLE); break;*/ \
  } \
} while (0)

#define H2BE(VARIABLE) \
do { \
  switch (sizeof(VARIABLE)) { \
  case 1: VARIABLE = H2BE_1(VARIABLE); break; \
  case 2: VARIABLE = H2BE_2(VARIABLE); break; \
  case 4: VARIABLE = H2BE_4(VARIABLE); break; \
  case 8: VARIABLE = H2BE_8(VARIABLE); break; \
  /*case 16: VARIABLE = H2BE_16(VARIABLE); break;*/ \
  } \
} while (0)

#define BE2H(VARIABLE) \
do { \
  switch (sizeof(VARIABLE)) { \
  case 1: VARIABLE = BE2H_1(VARIABLE); break; \
  case 2: VARIABLE = BE2H_2(VARIABLE); break; \
  case 4: VARIABLE = BE2H_4(VARIABLE); break; \
  case 8: VARIABLE = BE2H_8(VARIABLE); break; \
  /*case 16: VARIABLE = BE2H_16(VARIABLE); break;*/ \
  } \
} while (0)

#define H2LE(VARIABLE) \
do { \
  switch (sizeof(VARIABLE)) { \
  case 1: VARIABLE = H2LE_1(VARIABLE); break; \
  case 2: VARIABLE = H2LE_2(VARIABLE); break; \
  case 4: VARIABLE = H2LE_4(VARIABLE); break; \
  case 8: VARIABLE = H2LE_8(VARIABLE); break; \
  /*case 16: VARIABLE = H2LE_16(VARIABLE); break;*/ \
  } \
} while (0)

#define LE2H(VARIABLE) \
do { \
  switch (sizeof(VARIABLE)) { \
  case 1: VARIABLE = LE2H_1(VARIABLE); break; \
  case 2: VARIABLE = LE2H_2(VARIABLE); break; \
  case 4: VARIABLE = LE2H_4(VARIABLE); break; \
  case 8: VARIABLE = LE2H_8(VARIABLE); break; \
  /*case 16: VARIABLE = LE2H_16(VARIABLE); break;*/ \
  } \
} while (0)



/* TARGET WORD:

   Byte swap a quantity the size of the targets word */

#if (WITH_TARGET_WORD_BITSIZE == 64)
#define H2T_word(X) H2T_8(X)
#define T2H_word(X) T2H_8(X)
#define H2BE_word(X) H2BE_8(X)
#define BE2H_word(X) BE2H_8(X)
#define H2LE_word(X) H2LE_8(X)
#define LE2H_word(X) LE2H_8(X)
#define SWAP_word(X) SWAP_8(X)
#endif
#if (WITH_TARGET_WORD_BITSIZE == 32)
#define H2T_word(X) H2T_4(X)
#define T2H_word(X) T2H_4(X)
#define H2BE_word(X) H2BE_4(X)
#define BE2H_word(X) BE2H_4(X)
#define H2LE_word(X) H2LE_4(X)
#define LE2H_word(X) LE2H_4(X)
#define SWAP_word(X) SWAP_4(X)
#endif



/* TARGET CELL:

   Byte swap a quantity the size of the targets IEEE 1275 memory cell */

#define H2T_cell(X) H2T_4(X)
#define T2H_cell(X) T2H_4(X)
#define H2BE_cell(X) H2BE_4(X)
#define BE2H_cell(X) BE2H_4(X)
#define H2LE_cell(X) H2LE_4(X)
#define LE2H_cell(X) LE2H_4(X)
#define SWAP_cell(X) SWAP_4(X)



/* HOST Offsets:

   Address of high/low sub-word within a host word quantity.

   Address of sub-word N within a host word quantity.  NOTE: Numbering
   is BIG endian always. */

#define AH1_2(X) (unsigned_1*)offset_2((X), 1, 0)
#define AL1_2(X) (unsigned_1*)offset_2((X), 1, 1)

#define AH2_4(X) (unsigned_2*)offset_4((X), 2, 0)
#define AL2_4(X) (unsigned_2*)offset_4((X), 2, 1)

#define AH4_8(X) (unsigned_4*)offset_8((X), 4, 0)
#define AL4_8(X) (unsigned_4*)offset_8((X), 4, 1)

#define AH8_16(X) (unsigned_8*)offset_16((X), 8, 0)
#define AL8_16(X) (unsigned_8*)offset_16((X), 8, 1)

#if (WITH_TARGET_WORD_BITSIZE == 64)
#define AH_word(X) AH4_8(X)
#define AL_word(X) AL4_8(X)
#endif
#if (WITH_TARGET_WORD_BITSIZE == 32)
#define AH_word(X) AH2_4(X)
#define AL_word(X) AL2_4(X)
#endif


#define A1_2(X,N) (unsigned_1*)offset_2((X), 1, (N))

#define A1_4(X,N) (unsigned_1*)offset_4((X), 1, (N))
#define A2_4(X,N) (unsigned_2*)offset_4((X), 2, (N))

#define A1_8(X,N) (unsigned_1*)offset_8((X), 1, (N))
#define A2_8(X,N) (unsigned_2*)offset_8((X), 2, (N))
#define A4_8(X,N) (unsigned_4*)offset_8((X), 4, (N))

#define A1_16(X,N) (unsigned_1*)offset_16((X), 1, (N))
#define A2_16(X,N) (unsigned_2*)offset_16((X), 2, (N))
#define A4_16(X,N) (unsigned_4*)offset_16((X), 4, (N))
#define A8_16(X,N) (unsigned_8*)offset_16((X), 8, (N))




/* HOST Components:

   Value of sub-word within a host word quantity */

#define VH1_2(X) ((unsigned_1)((unsigned_2)(X) >> 8))
#define VL1_2(X) (unsigned_1)(X)

#define VH2_4(X) ((unsigned_2)((unsigned_4)(X) >> 16))
#define VL2_4(X) ((unsigned_2)(X))

#define VH4_8(X) ((unsigned_4)((unsigned_8)(X) >> 32))
#define VL4_8(X) ((unsigned_4)(X))

#define VH8_16(X) (sim_endian_split_16 ((X), 0))
#define VL8_16(X) (sim_endian_split_16 ((X), 1))

#if (WITH_TARGET_WORD_BITSIZE == 64)
#define VH_word(X) VH4_8(X)
#define VL_word(X) VL4_8(X)
#endif
#if (WITH_TARGET_WORD_BITSIZE == 32)
#define VH_word(X) VH2_4(X)
#define VL_word(X) VL2_4(X)
#endif


#define V1_2(X,N) ((unsigned_1)((unsigned_2)(X) >> ( 8 * (1 - (N)))))

#define V1_4(X,N) ((unsigned_1)((unsigned_4)(X) >> ( 8 * (3 - (N)))))
#define V2_4(X,N) ((unsigned_2)((unsigned_4)(X) >> (16 * (1 - (N)))))

#define V1_8(X,N) ((unsigned_1)((unsigned_8)(X) >> ( 8 * (7 - (N)))))
#define V2_8(X,N) ((unsigned_2)((unsigned_8)(X) >> (16 * (3 - (N)))))
#define V4_8(X,N) ((unsigned_4)((unsigned_8)(X) >> (32 * (1 - (N)))))

#define V1_16(X,N) (*A1_16 (&(X),N))
#define V2_16(X,N) (*A2_16 (&(X),N))
#define V4_16(X,N) (*A4_16 (&(X),N))
#define V8_16(X,N) (*A8_16 (&(X),N))


/* Reverse - insert sub-word into word quantity */

#define V2_H1(X) ((unsigned_2)(unsigned_1)(X) <<  8)
#define V2_L1(X) ((unsigned_2)(unsigned_1)(X))

#define V4_H2(X) ((unsigned_4)(unsigned_2)(X) << 16)
#define V4_L2(X) ((unsigned_4)(unsigned_2)(X))

#define V8_H4(X) ((unsigned_8)(unsigned_4)(X) << 32)
#define V8_L4(X) ((unsigned_8)(unsigned_4)(X))

#define V16_H8(X) ((unsigned_16)(unsigned_8)(X) << 64)
#define V16_L8(X) ((unsigned_16)(unsigned_8)(X))


#define V2_1(X,N) ((unsigned_2)(unsigned_1)(X) << ( 8 * (1 - (N))))

#define V4_1(X,N) ((unsigned_4)(unsigned_1)(X) << ( 8 * (3 - (N))))
#define V4_2(X,N) ((unsigned_4)(unsigned_2)(X) << (16 * (1 - (N))))

#define V8_1(X,N) ((unsigned_8)(unsigned_1)(X) << ( 8 * (7 - (N))))
#define V8_2(X,N) ((unsigned_8)(unsigned_2)(X) << (16 * (3 - (N))))
#define V8_4(X,N) ((unsigned_8)(unsigned_4)(X) << (32 * (1 - (N))))

#define V16_1(X,N) ((unsigned_16)(unsigned_1)(X) << ( 8 * (15 - (N))))
#define V16_2(X,N) ((unsigned_16)(unsigned_2)(X) << (16 * (7 - (N))))
#define V16_4(X,N) ((unsigned_16)(unsigned_4)(X) << (32 * (3 - (N))))
#define V16_8(X,N) ((unsigned_16)(unsigned_8)(X) << (64 * (1 - (N))))


/* Reverse - insert N sub-words into single word quantity */

#define U2_1(I0,I1) (V2_1(I0,0) | V2_1(I1,1))
#define U4_1(I0,I1,I2,I3) (V4_1(I0,0) | V4_1(I1,1) | V4_1(I2,2) | V4_1(I3,3))
#define U8_1(I0,I1,I2,I3,I4,I5,I6,I7) \
(V8_1(I0,0) | V8_1(I1,1) | V8_1(I2,2) | V8_1(I3,3) \
 | V8_1(I4,4) | V8_1(I5,5) | V8_1(I6,6) | V8_1(I7,7))
#define U16_1(I0,I1,I2,I3,I4,I5,I6,I7,I8,I9,I10,I11,I12,I13,I14,I15) \
(V16_1(I0,0) | V16_1(I1,1) | V16_1(I2,2) | V16_1(I3,3) \
 | V16_1(I4,4) | V16_1(I5,5) | V16_1(I6,6) | V16_1(I7,7) \
 | V16_1(I8,8) | V16_1(I9,9) | V16_1(I10,10) | V16_1(I11,11) \
 | V16_1(I12,12) | V16_1(I13,13) | V16_1(I14,14) | V16_1(I15,15))

#define U4_2(I0,I1) (V4_2(I0,0) | V4_2(I1,1))
#define U8_2(I0,I1,I2,I3) (V8_2(I0,0) | V8_2(I1,1) | V8_2(I2,2) | V8_2(I3,3))
#define U16_2(I0,I1,I2,I3,I4,I5,I6,I7) \
(V16_2(I0,0) | V16_2(I1,1) | V16_2(I2,2) | V16_2(I3,3) \
 | V16_2(I4,4) | V16_2(I5,5) | V16_2(I6,6) | V16_2(I7,7) )

#define U8_4(I0,I1) (V8_4(I0,0) | V8_4(I1,1))
#define U16_4(I0,I1,I2,I3) (V16_4(I0,0) | V16_4(I1,1) | V16_4(I2,2) | V16_4(I3,3))

#define U16_8(I0,I1) (sim_endian_join_16 (I0, I1))


#if (WITH_TARGET_WORD_BITSIZE == 64)
#define Vword_H(X) V8_H4(X)
#define Vword_L(X) V8_L4(X)
#endif
#if (WITH_TARGET_WORD_BITSIZE == 32)
#define Vword_H(X) V4_H2(X)
#define Vword_L(X) V4_L2(X)
#endif




#if H_REVEALS_MODULE_P (SIM_ENDIAN_INLINE)
#include "sim-endian.c"
#endif

#endif /* _SIM_ENDIAN_H_ */
