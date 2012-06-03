/* Generic COFF swapping routines, for BFD.
   Copyright 1990, 1991, 1992, 1993 Free Software Foundation, Inc.
   Written by Cygnus Support.

This file is part of BFD, the Binary File Descriptor library.

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
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

/* This file contains routines used to swap COFF data.  It is a header
   file because the details of swapping depend on the details of the
   structures used by each COFF implementation.  This is included by
   coffcode.h, as well as by the ECOFF backend.

   Any file which uses this must first include "coff/internal.h" and
   "coff/CPU.h".  The functions will then be correct for that CPU.  */

#define PUTWORD bfd_h_put_32
#define PUTHALF bfd_h_put_16
#define	PUTBYTE bfd_h_put_8

#ifndef GET_FCN_LNNOPTR
#define GET_FCN_LNNOPTR(abfd, ext)  bfd_h_get_32(abfd, (bfd_byte *) ext->x_sym.x_fcnary.x_fcn.x_lnnoptr)
#endif

#ifndef GET_FCN_ENDNDX
#define GET_FCN_ENDNDX(abfd, ext)  bfd_h_get_32(abfd, (bfd_byte *) ext->x_sym.x_fcnary.x_fcn.x_endndx)
#endif

#ifndef PUT_FCN_LNNOPTR
#define PUT_FCN_LNNOPTR(abfd, in, ext)  PUTWORD(abfd,  in, (bfd_byte *) ext->x_sym.x_fcnary.x_fcn.x_lnnoptr)
#endif
#ifndef PUT_FCN_ENDNDX
#define PUT_FCN_ENDNDX(abfd, in, ext) PUTWORD(abfd, in, (bfd_byte *) ext->x_sym.x_fcnary.x_fcn.x_endndx)
#endif
#ifndef GET_LNSZ_LNNO
#define GET_LNSZ_LNNO(abfd, ext) bfd_h_get_16(abfd, (bfd_byte *) ext->x_sym.x_misc.x_lnsz.x_lnno)
#endif
#ifndef GET_LNSZ_SIZE
#define GET_LNSZ_SIZE(abfd, ext) bfd_h_get_16(abfd, (bfd_byte *) ext->x_sym.x_misc.x_lnsz.x_size)
#endif
#ifndef PUT_LNSZ_LNNO
#define PUT_LNSZ_LNNO(abfd, in, ext) bfd_h_put_16(abfd, in, (bfd_byte *)ext->x_sym.x_misc.x_lnsz.x_lnno)
#endif
#ifndef PUT_LNSZ_SIZE
#define PUT_LNSZ_SIZE(abfd, in, ext) bfd_h_put_16(abfd, in, (bfd_byte*) ext->x_sym.x_misc.x_lnsz.x_size)
#endif
#ifndef GET_SCN_SCNLEN
#define GET_SCN_SCNLEN(abfd,  ext) bfd_h_get_32(abfd, (bfd_byte *) ext->x_scn.x_scnlen)
#endif
#ifndef GET_SCN_NRELOC
#define GET_SCN_NRELOC(abfd,  ext) bfd_h_get_16(abfd, (bfd_byte *)ext->x_scn.x_nreloc)
#endif
#ifndef GET_SCN_NLINNO
#define GET_SCN_NLINNO(abfd, ext)  bfd_h_get_16(abfd, (bfd_byte *)ext->x_scn.x_nlinno)
#endif
#ifndef PUT_SCN_SCNLEN
#define PUT_SCN_SCNLEN(abfd,in, ext) bfd_h_put_32(abfd, in, (bfd_byte *) ext->x_scn.x_scnlen)
#endif
#ifndef PUT_SCN_NRELOC
#define PUT_SCN_NRELOC(abfd,in, ext) bfd_h_put_16(abfd, in, (bfd_byte *)ext->x_scn.x_nreloc)
#endif
#ifndef PUT_SCN_NLINNO
#define PUT_SCN_NLINNO(abfd,in, ext)  bfd_h_put_16(abfd,in, (bfd_byte  *) ext->x_scn.x_nlinno)
#endif
#ifndef GET_LINENO_LNNO
#define GET_LINENO_LNNO(abfd, ext) bfd_h_get_16(abfd, (bfd_byte *) (ext->l_lnno));
#endif
#ifndef PUT_LINENO_LNNO
#define PUT_LINENO_LNNO(abfd,val, ext) bfd_h_put_16(abfd,val,  (bfd_byte *) (ext->l_lnno));
#endif

#ifndef NO_COFF_RELOCS

static void
DEFUN(bfd_swap_reloc_in,(abfd, reloc_src, reloc_dst),
      bfd            *abfd AND
      RELOC *reloc_src AND
      struct internal_reloc *reloc_dst)
{
  reloc_dst->r_vaddr = bfd_h_get_32(abfd, (bfd_byte *)reloc_src->r_vaddr);
  reloc_dst->r_symndx = bfd_h_get_32(abfd, (bfd_byte *) reloc_src->r_symndx);

#ifdef RS6000COFF_C
  reloc_dst->r_type = bfd_h_get_8(abfd, reloc_src->r_type);
  reloc_dst->r_size = bfd_h_get_8(abfd, reloc_src->r_size);
#else
  reloc_dst->r_type = bfd_h_get_16(abfd, (bfd_byte *) reloc_src->r_type);
#endif

#ifdef SWAP_IN_RELOC_OFFSET
  reloc_dst->r_offset = SWAP_IN_RELOC_OFFSET(abfd,
					     (bfd_byte *) reloc_src->r_offset);
#endif
}


static unsigned int
DEFUN(coff_swap_reloc_out,(abfd, src, dst),
      bfd       *abfd AND
      PTR	src AND
      PTR	dst)
{
  struct internal_reloc *reloc_src = (struct internal_reloc *)src;
  struct external_reloc *reloc_dst = (struct external_reloc *)dst;
  bfd_h_put_32(abfd, reloc_src->r_vaddr, (bfd_byte *) reloc_dst->r_vaddr);
  bfd_h_put_32(abfd, reloc_src->r_symndx, (bfd_byte *) reloc_dst->r_symndx);
  bfd_h_put_16(abfd, reloc_src->r_type, (bfd_byte *)
	       reloc_dst->r_type);

#ifdef SWAP_OUT_RELOC_OFFSET
  SWAP_OUT_RELOC_OFFSET(abfd,
			reloc_src->r_offset,
			(bfd_byte *) reloc_dst->r_offset);
#endif
#ifdef SWAP_OUT_RELOC_EXTRA
  SWAP_OUT_RELOC_EXTRA(abfd,reloc_src, reloc_dst);
#endif

  return sizeof(struct external_reloc);
}

#endif /* NO_COFF_RELOCS */

static void
DEFUN(coff_swap_filehdr_in,(abfd, src, dst),
      bfd            *abfd AND
      PTR	     src AND
      PTR	     dst)
{
  FILHDR *filehdr_src = (FILHDR *) src;
  struct internal_filehdr *filehdr_dst = (struct internal_filehdr *) dst;
  filehdr_dst->f_magic = bfd_h_get_16(abfd, (bfd_byte *) filehdr_src->f_magic);
  filehdr_dst->f_nscns = bfd_h_get_16(abfd, (bfd_byte *)filehdr_src-> f_nscns);
  filehdr_dst->f_timdat = bfd_h_get_32(abfd, (bfd_byte *)filehdr_src-> f_timdat);
  filehdr_dst->f_symptr = bfd_h_get_32(abfd, (bfd_byte *)filehdr_src-> f_symptr);
  filehdr_dst->f_nsyms = bfd_h_get_32(abfd, (bfd_byte *)filehdr_src-> f_nsyms);
  filehdr_dst->f_opthdr = bfd_h_get_16(abfd, (bfd_byte *)filehdr_src-> f_opthdr);
  filehdr_dst->f_flags = bfd_h_get_16(abfd, (bfd_byte *)filehdr_src-> f_flags);
}

static  unsigned int
DEFUN(coff_swap_filehdr_out,(abfd, in, out),
      bfd       *abfd AND
      PTR	in AND
      PTR	out)
{
  struct internal_filehdr *filehdr_in = (struct internal_filehdr *)in;
  FILHDR *filehdr_out = (FILHDR *)out;
  bfd_h_put_16(abfd, filehdr_in->f_magic, (bfd_byte *) filehdr_out->f_magic);
  bfd_h_put_16(abfd, filehdr_in->f_nscns, (bfd_byte *) filehdr_out->f_nscns);
  bfd_h_put_32(abfd, filehdr_in->f_timdat, (bfd_byte *) filehdr_out->f_timdat);
  bfd_h_put_32(abfd, filehdr_in->f_symptr, (bfd_byte *) filehdr_out->f_symptr);
  bfd_h_put_32(abfd, filehdr_in->f_nsyms, (bfd_byte *) filehdr_out->f_nsyms);
  bfd_h_put_16(abfd, filehdr_in->f_opthdr, (bfd_byte *) filehdr_out->f_opthdr);
  bfd_h_put_16(abfd, filehdr_in->f_flags, (bfd_byte *) filehdr_out->f_flags);
  return sizeof(FILHDR);
}


#ifndef NO_COFF_SYMBOLS

static void
DEFUN(coff_swap_sym_in,(abfd, ext1, in1),
      bfd            *abfd AND
      PTR ext1 AND
      PTR in1)
{
  SYMENT *ext = (SYMENT *)ext1;
  struct internal_syment      *in = (struct internal_syment *)in1;

  if( ext->e.e_name[0] == 0) {
    in->_n._n_n._n_zeroes = 0;
    in->_n._n_n._n_offset = bfd_h_get_32(abfd, (bfd_byte *) ext->e.e.e_offset);
  }
  else {
#if SYMNMLEN != E_SYMNMLEN
   -> Error, we need to cope with truncating or extending SYMNMLEN!;
#else
    memcpy(in->_n._n_name, ext->e.e_name, SYMNMLEN);
#endif
  }
  in->n_value = bfd_h_get_32(abfd, (bfd_byte *) ext->e_value);
  in->n_scnum = bfd_h_get_16(abfd, (bfd_byte *) ext->e_scnum);
  if (sizeof(ext->e_type) == 2){
    in->n_type = bfd_h_get_16(abfd, (bfd_byte *) ext->e_type);
  }
  else {
    in->n_type = bfd_h_get_32(abfd, (bfd_byte *) ext->e_type);
  }
  in->n_sclass = bfd_h_get_8(abfd, ext->e_sclass);
  in->n_numaux = bfd_h_get_8(abfd, ext->e_numaux);
}

static unsigned int
DEFUN(coff_swap_sym_out,(abfd, inp, extp),
      bfd       *abfd AND
      PTR	inp AND
      PTR	extp)
{
  struct internal_syment *in = (struct internal_syment *)inp;
  SYMENT *ext =(SYMENT *)extp;
  if(in->_n._n_name[0] == 0) {
    bfd_h_put_32(abfd, 0, (bfd_byte *) ext->e.e.e_zeroes);
    bfd_h_put_32(abfd, in->_n._n_n._n_offset, (bfd_byte *)  ext->e.e.e_offset);
  }
  else {
#if SYMNMLEN != E_SYMNMLEN
    -> Error, we need to cope with truncating or extending SYMNMLEN!;
#else
    memcpy(ext->e.e_name, in->_n._n_name, SYMNMLEN);
#endif
  }
  bfd_h_put_32(abfd,  in->n_value , (bfd_byte *) ext->e_value);
  bfd_h_put_16(abfd,  in->n_scnum , (bfd_byte *) ext->e_scnum);
  if (sizeof(ext->e_type) == 2)
      {
	bfd_h_put_16(abfd,  in->n_type , (bfd_byte *) ext->e_type);
      }
  else
      {
	bfd_h_put_32(abfd,  in->n_type , (bfd_byte *) ext->e_type);
      }
  bfd_h_put_8(abfd,  in->n_sclass , ext->e_sclass);
  bfd_h_put_8(abfd,  in->n_numaux , ext->e_numaux);
  return sizeof(SYMENT);
}

static void
DEFUN(coff_swap_aux_in,(abfd, ext1, type, class, in1),
      bfd            *abfd AND
      PTR 	      ext1 AND
      int             type AND
      int             class AND
      PTR 	      in1)
{
  AUXENT    *ext = (AUXENT *)ext1;
  union internal_auxent *in = (union internal_auxent *)in1;

  switch (class) {
    case C_FILE:
      if (ext->x_file.x_fname[0] == 0) {
	  in->x_file.x_n.x_zeroes = 0;
	  in->x_file.x_n.x_offset = 
	   bfd_h_get_32(abfd, (bfd_byte *) ext->x_file.x_n.x_offset);
	} else {
#if FILNMLEN != E_FILNMLEN
	    -> Error, we need to cope with truncating or extending FILNMLEN!;
#else
	    memcpy (in->x_file.x_fname, ext->x_file.x_fname, FILNMLEN);
#endif
	  }
      break;

      /* RS/6000 "csect" auxents */
#ifdef RS6000COFF_C
    case C_EXT:
    case C_HIDEXT:
      in->x_csect.x_scnlen   = bfd_h_get_32 (abfd, (bfd_byte *) ext->x_csect.x_scnlen);
      in->x_csect.x_parmhash = bfd_h_get_32 (abfd, (bfd_byte *) ext->x_csect.x_parmhash);
      in->x_csect.x_snhash   = bfd_h_get_16 (abfd, (bfd_byte *) ext->x_csect.x_snhash);
      /* We don't have to hack bitfields in x_smtyp because it's defined by
	 shifts-and-ands, which are equivalent on all byte orders.  */
      in->x_csect.x_smtyp    = bfd_h_get_8  (abfd, (bfd_byte *) ext->x_csect.x_smtyp);
      in->x_csect.x_smclas   = bfd_h_get_8  (abfd, (bfd_byte *) ext->x_csect.x_smclas);
      in->x_csect.x_stab     = bfd_h_get_32 (abfd, (bfd_byte *) ext->x_csect.x_stab);
      in->x_csect.x_snstab   = bfd_h_get_16 (abfd, (bfd_byte *) ext->x_csect.x_snstab);
      break;
#endif

    case C_STAT:
#ifdef C_LEAFSTAT
    case C_LEAFSTAT:
#endif
    case C_HIDDEN:
      if (type == T_NULL) {
	  in->x_scn.x_scnlen = GET_SCN_SCNLEN(abfd, ext);
	  in->x_scn.x_nreloc = GET_SCN_NRELOC(abfd, ext);
	  in->x_scn.x_nlinno = GET_SCN_NLINNO(abfd, ext);
	  break;
	}
    default:
      in->x_sym.x_tagndx.l = bfd_h_get_32(abfd, (bfd_byte *) ext->x_sym.x_tagndx);
#ifndef NO_TVNDX
      in->x_sym.x_tvndx = bfd_h_get_16(abfd, (bfd_byte *) ext->x_sym.x_tvndx);
#endif

      if (ISARY(type)) {
#if DIMNUM != E_DIMNUM
	  -> Error, we need to cope with truncating or extending DIMNUM!;
#else
	  in->x_sym.x_fcnary.x_ary.x_dimen[0] = bfd_h_get_16(abfd, (bfd_byte *) ext->x_sym.x_fcnary.x_ary.x_dimen[0]);
	  in->x_sym.x_fcnary.x_ary.x_dimen[1] = bfd_h_get_16(abfd, (bfd_byte *) ext->x_sym.x_fcnary.x_ary.x_dimen[1]);
	  in->x_sym.x_fcnary.x_ary.x_dimen[2] = bfd_h_get_16(abfd, (bfd_byte *) ext->x_sym.x_fcnary.x_ary.x_dimen[2]);
	  in->x_sym.x_fcnary.x_ary.x_dimen[3] = bfd_h_get_16(abfd, (bfd_byte *) ext->x_sym.x_fcnary.x_ary.x_dimen[3]);
#endif
	}
      if (class == C_BLOCK || ISFCN(type) || ISTAG(class)) {
	in->x_sym.x_fcnary.x_fcn.x_lnnoptr = GET_FCN_LNNOPTR(abfd, ext);
	in->x_sym.x_fcnary.x_fcn.x_endndx.l = GET_FCN_ENDNDX(abfd, ext);
      }

      if (ISFCN(type)) {
	  in->x_sym.x_misc.x_fsize = bfd_h_get_32(abfd, (bfd_byte *) ext->x_sym.x_misc.x_fsize);
	}
      else {
	  in->x_sym.x_misc.x_lnsz.x_lnno = GET_LNSZ_LNNO(abfd, ext);
	  in->x_sym.x_misc.x_lnsz.x_size = GET_LNSZ_SIZE(abfd, ext);
	}
    }
}

static unsigned int
DEFUN(coff_swap_aux_out,(abfd, inp, type, class, extp),
  bfd   *abfd AND
  PTR 	inp AND
  int   type AND
  int   class AND
  PTR	extp)
{
  union internal_auxent *in = (union internal_auxent *)inp;
  AUXENT *ext = (AUXENT *)extp;

  memset((PTR)ext, 0, AUXESZ);
  switch (class) {
  case C_FILE:
    if (in->x_file.x_fname[0] == 0) {
      PUTWORD(abfd, 0, (bfd_byte *) ext->x_file.x_n.x_zeroes);
      PUTWORD(abfd,
	      in->x_file.x_n.x_offset,
	      (bfd_byte *) ext->x_file.x_n.x_offset);
    }
    else {
#if FILNMLEN != E_FILNMLEN
      -> Error, we need to cope with truncating or extending FILNMLEN!;
#else
      memcpy (ext->x_file.x_fname, in->x_file.x_fname, FILNMLEN);
#endif
    }
    break;

#ifdef RS6000COFF_C
  /* RS/6000 "csect" auxents */
  case C_EXT:
  case C_HIDEXT:
    PUTWORD (abfd, in->x_csect.x_scnlen,	ext->x_csect.x_scnlen);
    PUTWORD (abfd, in->x_csect.x_parmhash,	ext->x_csect.x_parmhash);
    PUTHALF (abfd, in->x_csect.x_snhash,	ext->x_csect.x_snhash);
    /* We don't have to hack bitfields in x_smtyp because it's defined by
       shifts-and-ands, which are equivalent on all byte orders.  */
    PUTBYTE (abfd, in->x_csect.x_smtyp,		ext->x_csect.x_smtyp);
    PUTBYTE (abfd, in->x_csect.x_smclas,	ext->x_csect.x_smclas);
    PUTWORD (abfd, in->x_csect.x_stab,		ext->x_csect.x_stab);
    PUTHALF (abfd, in->x_csect.x_snstab,	ext->x_csect.x_snstab);
    break;
#endif

  case C_STAT:
#ifdef C_LEAFSTAT
  case C_LEAFSTAT:
#endif
  case C_HIDDEN:
    if (type == T_NULL) {
      PUT_SCN_SCNLEN(abfd, in->x_scn.x_scnlen, ext);
      PUT_SCN_NRELOC(abfd, in->x_scn.x_nreloc, ext);
      PUT_SCN_NLINNO(abfd, in->x_scn.x_nlinno, ext);
      break;
    }
  default:
    PUTWORD(abfd, in->x_sym.x_tagndx.l, (bfd_byte *) ext->x_sym.x_tagndx);
#ifndef NO_TVNDX
    bfd_h_put_16(abfd, in->x_sym.x_tvndx , (bfd_byte *) ext->x_sym.x_tvndx);
#endif

    if (class == C_BLOCK || ISFCN(type) || ISTAG(class)) {
      PUT_FCN_LNNOPTR(abfd,  in->x_sym.x_fcnary.x_fcn.x_lnnoptr, ext);
      PUT_FCN_ENDNDX(abfd,  in->x_sym.x_fcnary.x_fcn.x_endndx.l, ext);
    }

    if (ISFCN(type)) {
      PUTWORD(abfd, in->x_sym.x_misc.x_fsize, (bfd_byte *)  ext->x_sym.x_misc.x_fsize);
    }
    else {
      if (ISARY(type)) {
#if DIMNUM != E_DIMNUM
	-> Error, we need to cope with truncating or extending DIMNUM!;
#else
	bfd_h_put_16(abfd, in->x_sym.x_fcnary.x_ary.x_dimen[0], (bfd_byte *)ext->x_sym.x_fcnary.x_ary.x_dimen[0]);
	bfd_h_put_16(abfd, in->x_sym.x_fcnary.x_ary.x_dimen[1], (bfd_byte *)ext->x_sym.x_fcnary.x_ary.x_dimen[1]);
	bfd_h_put_16(abfd, in->x_sym.x_fcnary.x_ary.x_dimen[2], (bfd_byte *)ext->x_sym.x_fcnary.x_ary.x_dimen[2]);
	bfd_h_put_16(abfd, in->x_sym.x_fcnary.x_ary.x_dimen[3], (bfd_byte *)ext->x_sym.x_fcnary.x_ary.x_dimen[3]);
#endif
      }
      PUT_LNSZ_LNNO(abfd, in->x_sym.x_misc.x_lnsz.x_lnno, ext);
      PUT_LNSZ_SIZE(abfd, in->x_sym.x_misc.x_lnsz.x_size, ext);
    }
  }
return sizeof(AUXENT);
}

#endif /* NO_COFF_SYMBOLS */

#ifndef NO_COFF_LINENOS

static void
DEFUN(coff_swap_lineno_in,(abfd, ext1, in1),
      bfd            *abfd AND
      PTR ext1 AND
      PTR in1)
{
  LINENO *ext = (LINENO *)ext1;
  struct internal_lineno      *in = (struct internal_lineno *)in1;

  in->l_addr.l_symndx = bfd_h_get_32(abfd, (bfd_byte *) ext->l_addr.l_symndx);
  in->l_lnno = GET_LINENO_LNNO(abfd, ext);
}

static unsigned int
DEFUN(coff_swap_lineno_out,(abfd, inp, outp),
      bfd       *abfd AND
      PTR	inp AND
      PTR	outp)
{
  struct internal_lineno *in = (struct internal_lineno *)inp;
  struct external_lineno *ext = (struct external_lineno *)outp;
  PUTWORD(abfd, in->l_addr.l_symndx, (bfd_byte *)
	  ext->l_addr.l_symndx);

  PUT_LINENO_LNNO (abfd, in->l_lnno, ext);
  return sizeof(struct external_lineno);
}

#endif /* NO_COFF_LINENOS */


static void
DEFUN(coff_swap_aouthdr_in,(abfd, aouthdr_ext1, aouthdr_int1),
      bfd            *abfd AND
      PTR aouthdr_ext1 AND
      PTR aouthdr_int1)
{
  AOUTHDR        *aouthdr_ext = (AOUTHDR *) aouthdr_ext1;
  struct internal_aouthdr *aouthdr_int = (struct internal_aouthdr *)aouthdr_int1;

  aouthdr_int->magic = bfd_h_get_16(abfd, (bfd_byte *) aouthdr_ext->magic);
  aouthdr_int->vstamp = bfd_h_get_16(abfd, (bfd_byte *) aouthdr_ext->vstamp);
  aouthdr_int->tsize = bfd_h_get_32(abfd, (bfd_byte *) aouthdr_ext->tsize);
  aouthdr_int->dsize = bfd_h_get_32(abfd, (bfd_byte *) aouthdr_ext->dsize);
  aouthdr_int->bsize = bfd_h_get_32(abfd, (bfd_byte *) aouthdr_ext->bsize);
  aouthdr_int->entry = bfd_h_get_32(abfd, (bfd_byte *) aouthdr_ext->entry);
  aouthdr_int->text_start = bfd_h_get_32(abfd, (bfd_byte *) aouthdr_ext->text_start);
  aouthdr_int->data_start = bfd_h_get_32(abfd, (bfd_byte *) aouthdr_ext->data_start);
#ifdef I960
  aouthdr_int->tagentries = bfd_h_get_32(abfd, (bfd_byte *) aouthdr_ext->tagentries);
#endif

#ifdef RS6000COFF_C
  aouthdr_int->o_toc = bfd_h_get_32(abfd, aouthdr_ext->o_toc);
  aouthdr_int->o_snentry = bfd_h_get_16(abfd, aouthdr_ext->o_snentry);
  aouthdr_int->o_sntext = bfd_h_get_16(abfd, aouthdr_ext->o_sntext);
  aouthdr_int->o_sndata = bfd_h_get_16(abfd, aouthdr_ext->o_sndata);
  aouthdr_int->o_sntoc = bfd_h_get_16(abfd, aouthdr_ext->o_sntoc);
  aouthdr_int->o_snloader = bfd_h_get_16(abfd, aouthdr_ext->o_snloader);
  aouthdr_int->o_snbss = bfd_h_get_16(abfd, aouthdr_ext->o_snbss);
  aouthdr_int->o_algntext = bfd_h_get_16(abfd, aouthdr_ext->o_algntext);
  aouthdr_int->o_algndata = bfd_h_get_16(abfd, aouthdr_ext->o_algndata);
  aouthdr_int->o_modtype = bfd_h_get_16(abfd, aouthdr_ext->o_modtype);
  aouthdr_int->o_maxstack = bfd_h_get_32(abfd, aouthdr_ext->o_maxstack);
#endif

#ifdef MIPSECOFF
  aouthdr_int->bss_start = bfd_h_get_32(abfd, aouthdr_ext->bss_start);
  aouthdr_int->gp_value = bfd_h_get_32(abfd, aouthdr_ext->gp_value);
  aouthdr_int->gprmask = bfd_h_get_32(abfd, aouthdr_ext->gprmask);
  aouthdr_int->cprmask[0] = bfd_h_get_32(abfd, aouthdr_ext->cprmask[0]);
  aouthdr_int->cprmask[1] = bfd_h_get_32(abfd, aouthdr_ext->cprmask[1]);
  aouthdr_int->cprmask[2] = bfd_h_get_32(abfd, aouthdr_ext->cprmask[2]);
  aouthdr_int->cprmask[3] = bfd_h_get_32(abfd, aouthdr_ext->cprmask[3]);
#endif
}

static unsigned int
DEFUN(coff_swap_aouthdr_out,(abfd, in, out),
      bfd       *abfd AND
      PTR	in AND
      PTR	out)
{
  struct internal_aouthdr *aouthdr_in = (struct internal_aouthdr *)in;
  AOUTHDR *aouthdr_out = (AOUTHDR *)out;
  bfd_h_put_16(abfd, aouthdr_in->magic, (bfd_byte *) aouthdr_out->magic);
  bfd_h_put_16(abfd, aouthdr_in->vstamp, (bfd_byte *) aouthdr_out->vstamp);
  bfd_h_put_32(abfd, aouthdr_in->tsize, (bfd_byte *) aouthdr_out->tsize);
  bfd_h_put_32(abfd, aouthdr_in->dsize, (bfd_byte *) aouthdr_out->dsize);
  bfd_h_put_32(abfd, aouthdr_in->bsize, (bfd_byte *) aouthdr_out->bsize);
  bfd_h_put_32(abfd, aouthdr_in->entry, (bfd_byte *) aouthdr_out->entry);
  bfd_h_put_32(abfd, aouthdr_in->text_start,
	       (bfd_byte *) aouthdr_out->text_start);
  bfd_h_put_32(abfd, aouthdr_in->data_start, (bfd_byte *) aouthdr_out->data_start);
#ifdef I960
  bfd_h_put_32(abfd, aouthdr_in->tagentries, (bfd_byte *) aouthdr_out->tagentries);
#endif

#ifdef MIPSECOFF
  bfd_h_put_32(abfd, aouthdr_in->bss_start, (bfd_byte *) aouthdr_out->bss_start);
  bfd_h_put_32(abfd, aouthdr_in->gp_value, (bfd_byte *) aouthdr_out->gp_value);
  bfd_h_put_32(abfd, aouthdr_in->gprmask, (bfd_byte *) aouthdr_out->gprmask);
  bfd_h_put_32(abfd, aouthdr_in->cprmask[0], (bfd_byte *) aouthdr_out->cprmask[0]);
  bfd_h_put_32(abfd, aouthdr_in->cprmask[1], (bfd_byte *) aouthdr_out->cprmask[1]);
  bfd_h_put_32(abfd, aouthdr_in->cprmask[2], (bfd_byte *) aouthdr_out->cprmask[2]);
  bfd_h_put_32(abfd, aouthdr_in->cprmask[3], (bfd_byte *) aouthdr_out->cprmask[3]);
#endif

  return sizeof(AOUTHDR);
}

static void
DEFUN(coff_swap_scnhdr_in,(abfd, ext, in),
      bfd            *abfd AND
      PTR	     ext AND
      PTR	     in)
{
  SCNHDR *scnhdr_ext = (SCNHDR *) ext;
  struct internal_scnhdr *scnhdr_int = (struct internal_scnhdr *) in;
  memcpy(scnhdr_int->s_name, scnhdr_ext->s_name, sizeof(scnhdr_int->s_name));
  scnhdr_int->s_vaddr = bfd_h_get_32(abfd, (bfd_byte *) scnhdr_ext->s_vaddr);
  scnhdr_int->s_paddr = bfd_h_get_32(abfd, (bfd_byte *) scnhdr_ext->s_paddr);
  scnhdr_int->s_size = bfd_h_get_32(abfd, (bfd_byte *)  scnhdr_ext->s_size);

  scnhdr_int->s_scnptr = bfd_h_get_32(abfd, (bfd_byte *) scnhdr_ext->s_scnptr);
  scnhdr_int->s_relptr = bfd_h_get_32(abfd, (bfd_byte *) scnhdr_ext->s_relptr);
  scnhdr_int->s_lnnoptr = bfd_h_get_32(abfd, (bfd_byte *) scnhdr_ext->s_lnnoptr);
  scnhdr_int->s_flags = bfd_h_get_32(abfd, (bfd_byte *) scnhdr_ext->s_flags);
#if defined(M88)
  scnhdr_int->s_nreloc = bfd_h_get_32(abfd, (bfd_byte *) scnhdr_ext->s_nreloc);
  scnhdr_int->s_nlnno = bfd_h_get_32(abfd, (bfd_byte *) scnhdr_ext->s_nlnno);
#else
  scnhdr_int->s_nreloc = bfd_h_get_16(abfd, (bfd_byte *) scnhdr_ext->s_nreloc);
  scnhdr_int->s_nlnno = bfd_h_get_16(abfd, (bfd_byte *) scnhdr_ext->s_nlnno);
#endif
#ifdef I960
  scnhdr_int->s_align = bfd_h_get_32(abfd, (bfd_byte *) scnhdr_ext->s_align);
#endif
}

static unsigned int
DEFUN(coff_swap_scnhdr_out,(abfd, in, out),
      bfd       *abfd AND
      PTR	in AND
      PTR	out)
{
  struct internal_scnhdr *scnhdr_int = (struct internal_scnhdr *)in;
  SCNHDR *scnhdr_ext = (SCNHDR *)out;
  memcpy(scnhdr_ext->s_name, scnhdr_int->s_name, sizeof(scnhdr_int->s_name));
  PUTWORD(abfd, scnhdr_int->s_vaddr, (bfd_byte *) scnhdr_ext->s_vaddr);
  PUTWORD(abfd, scnhdr_int->s_paddr, (bfd_byte *) scnhdr_ext->s_paddr);
  PUTWORD(abfd, scnhdr_int->s_size, (bfd_byte *) scnhdr_ext->s_size);
  PUTWORD(abfd, scnhdr_int->s_scnptr, (bfd_byte *) scnhdr_ext->s_scnptr);
  PUTWORD(abfd, scnhdr_int->s_relptr, (bfd_byte *) scnhdr_ext->s_relptr);
  PUTWORD(abfd, scnhdr_int->s_lnnoptr, (bfd_byte *) scnhdr_ext->s_lnnoptr);
  PUTWORD(abfd, scnhdr_int->s_flags, (bfd_byte *) scnhdr_ext->s_flags);
#if defined(M88)
  PUTWORD(abfd, scnhdr_int->s_nlnno, (bfd_byte *) scnhdr_ext->s_nlnno);
  PUTWORD(abfd, scnhdr_int->s_nreloc, (bfd_byte *) scnhdr_ext->s_nreloc);
#else
  PUTHALF(abfd, scnhdr_int->s_nlnno, (bfd_byte *) scnhdr_ext->s_nlnno);
  PUTHALF(abfd, scnhdr_int->s_nreloc, (bfd_byte *) scnhdr_ext->s_nreloc);
#endif

#if defined(I960)
  PUTWORD(abfd, scnhdr_int->s_align, (bfd_byte *) scnhdr_ext->s_align);
#endif
  return sizeof(SCNHDR);
}
