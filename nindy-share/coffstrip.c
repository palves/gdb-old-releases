/*****************************************************************************
 * 		Copyright (c) 1990, Intel Corporation
 *
 * Intel hereby grants you permission to copy, modify, and 
 * distribute this software and its documentation.  Intel grants
 * this permission provided that the above copyright notice 
 * appears in all copies and that both the copyright notice and
 * this permission notice appear in supporting documentation.  In
 * addition, Intel grants this permission provided that you
 * prominently mark as not part of the original any modifications
 * made to this software or documentation, and that the name of 
 * Intel Corporation not be used in advertising or publicity 
 * pertaining to distribution of the software or the documentation 
 * without specific, written prior permission.  
 *
 * Intel Corporation does not warrant, guarantee or make any 
 * representations regarding the use of, or the results of the use
 * of, the software and documentation in terms of correctness, 
 * accuracy, reliability, currentness, or otherwise; and you rely
 * on the software, documentation and results solely at your own risk.
 *****************************************************************************/

static char rcsid[] =
	"$Id";

/*****************************************************************************
 *
 * Functions to perform b.out -> COFF conversion, and to strip symbol and
 *	relocation information from COFF files.
 *
 *****************************************************************************/

#include <stdio.h>
#include <sys/types.h>	/* Needed by file.h on Sys V */
#include <sys/file.h>
#include <setjmp.h>
#include <fcntl.h>	/* Needed on Sys V */
#include "coff.h"

/* The following are defined differently in coff.h & b.out.h -- and we
 * don't need either version.  #undef them just to let this compile.
 */
#undef		N_ABS
#undef		n_name

#include "b.out.h"

#ifdef USG
#	define SIGCHLD	SIGCLD
#	include <unistd.h>
#	include "sysv.h"
#else	/* BSD */
#	include "string.h"
#endif


/*****************************************************************************
 * PUT_LITTLE_END
 *	takes a number in host byte order and places it in a
 *	different location in little-endian (i80960) order.
 *
 * MAKE_LITTLE_END
 *	converts a number from host byte order to little-endian
 *	(i80960) order in place.
 *****************************************************************************/
#define PUT_LITTLE_END(n,w)	put_little_end( (char*)&w, (long) n, sizeof(w) )
#define MAKE_LITTLE_END(n)	put_little_end( (char*)&n, (long) n, sizeof(n) )


static char * xmalloc();
extern void free ();

/* Forward declarations. */
static void strip_bout ();
static void strip_coff ();
static int xopen ();
static void xread ();
static int xseek ();
static void xwrite ();

static jmp_buf	ioerr;	/* For recovery from I/O errors during file processing*/


/*****************************************************************************
 * coffstrip:
 *	Passed the name of an executable object file in either b.out or
 *	COFF format.
 *
 *	If the file is in b.out format, it is converted to little-endian
 *	COFF format (i.e., the format suitable for downloading to NINDY).
 *
 *	In either case, the COFF file is then stripped of all relocation
 *	and symbol information, to speed up its download.
 *
 * RETURNS:
 *	pointer to the name of the stripped COFF file (a tmp file that has
 *	been created and closed); NULL on error.
 *****************************************************************************/
char *
coffstrip( fn )
    char *fn;	/* Name of object file */
{
	extern char *mktemp();
	static char template[] = "/tmp/commXXXXXX";

	char *tempfile;	/* Stripped copy of object file			*/
	int infd;	/* Descriptor of object file to be downloaded	*/
	int outfd;	/* Descriptor on which to write stripped object file */

	struct filehdr *coff_hdr;
	struct exec *bout_hdr;
	char buf[10];		/* The first 10 bytes of the input file */


	infd = outfd = -1;

	if ( setjmp(ioerr) != 0 ){
		/* Come here on I/O error
		 */
		if ( infd != -1 ){
			close( infd );
		}
		if ( outfd != -1 ){
			close( outfd );
			unlink( tempfile );
		}
		return NULL;
	}

	/**********************************************************************
	 * Open input and output files.
	 **********************************************************************/

	infd = xopen( fn, O_RDONLY );
	tempfile = mktemp( template );
	outfd = xopen( tempfile, O_WRONLY|O_CREAT|O_TRUNC );

	/**********************************************************************
	 * Distinguish COFF from b.out files; call the appropriate routine to
	 * strip the symbols and relocation info from an object file.
	 **********************************************************************/

	xseek( infd, 0 );
	xread( infd, buf, sizeof(buf) );

	coff_hdr = (struct filehdr *) buf;
	bout_hdr = (struct exec *) buf;

	if ( (coff_hdr->f_magic == I960ROMAGIC)
	||   (coff_hdr->f_magic == I960RWMAGIC) ){
		strip_coff( infd, outfd );

	} else if ( !N_BADMAG(*bout_hdr) ){
		strip_bout( infd, outfd );

	} else {
		fprintf( stderr, "Unknown input file format\n" );
		longjmp( ioerr, 1 );
	}

	close( infd );
	close( outfd );
	return tempfile;
}



/******************************************************************************
 * put_little_end:
 *	Convert a number from host byte order to little-endian (i80960)
 *	byte order.
 *
 ******************************************************************************/
static
put_little_end( buf, value, n )
    char *buf;		/* Put output here		*/
    long value;		/* The integer to be converted	*/
    int	n;		/* Number of bytes to output (# of significant low-order
			 *	bytes in 'value').
			 */
{
	while ( n-- ){
		*buf++ = value;
		value >>= 8;
	}
}

/******************************************************************************
 * make_scn:
 *	Write a COFF section header to a stripped (no symbol info or relocation
 *	information) file.  Fields in the header are written in i80960
 *	(little-endian) byte order.
 *
 ******************************************************************************/
static
put_shdr( fd, name, paddr, size, flags, fileaddr , align )
    int fd;		/* Descriptor of file to which header is written */
    char *name;		/* Section name					*/
    long paddr;		/* Section runtime physical address		*/
    long size;		/* Size of section				*/
    long flags;		/* Section flags				*/
    long fileaddr;	/* Address of section binary in output file	*/
    long align;		/* Section alignment				*/
    /* NOTE THAT ALL OTHER FIELDS OF THE HEADER ARE ZEROED, BECAUSE
     * FILE IS ASSUMED TO BE STRIPPED!
     */
{
	struct scnhdr s;

	bzero( &s, sizeof(s) );
	strcpy( s.s_name, name );
	PUT_LITTLE_END( paddr, s.s_paddr );
	s.s_vaddr = s.s_paddr;
	PUT_LITTLE_END( size, s.s_size );
	PUT_LITTLE_END( flags, s.s_flags );
	PUT_LITTLE_END( fileaddr, s.s_scnptr );
	PUT_LITTLE_END( align, s.s_align );
	xwrite( fd, &s, sizeof(s) );
}

/******************************************************************************
 * strip_bout:
 *	Strip symbol and relocation information from a 'b.out' format object
 *	file and convert it into a COFF file.  The COFF file will have 3
 *	sections:  one each for data, text, and bss.
 *
 ******************************************************************************/
static void
strip_bout( infd, outfd )
    int infd;	/* Descriptor on which b.out file should be read.  File type
		 *	assumed already to have been validated.
		 */
    int outfd;	/* Descriptor on which to write out coff file.	*/
{
	struct exec hdr;/* Header from input file			*/
	char *buf;	/* Dynamic buffer for copying between files	*/
	int buflen;	/* Current size of buffer 'buf' (in bytes)	*/
	struct filehdr f; /* COFF file header				*/
	struct aouthdr a; /* COFF 'optional' (aout) header		*/

#	define HDRSIZE	(sizeof(f)+sizeof(a)+(3*sizeof(struct scnhdr)))
			/* Total size of COFF file headers	*/


	/* Read b.out file header.
	 */
	xseek( infd, 0 );
	xread( infd, &hdr, sizeof(hdr) );

	/* Generate COFF file header
	 */
	bzero( &f, sizeof(f) );
	PUT_LITTLE_END( I960ROMAGIC, f.f_magic );
	PUT_LITTLE_END( 3, f.f_nscns );
	PUT_LITTLE_END( sizeof(a), f.f_opthdr );
	PUT_LITTLE_END( F_RELFLG|F_EXEC|F_LNNO|F_LSYMS|F_AR32WR|F_I960BA,
								f.f_flags );
	xwrite( outfd, &f, sizeof(f) );

	/* Generate COFF aout header
	 */
	bzero( &a, sizeof(a) );
	PUT_LITTLE_END( hdr.a_entry, a.entry );
	PUT_LITTLE_END( BMAGIC, a.magic );
	PUT_LITTLE_END( hdr.a_text, a.tsize );
	PUT_LITTLE_END( hdr.a_data, a.dsize );
	PUT_LITTLE_END( hdr.a_bss, a.bsize );
	PUT_LITTLE_END( hdr.a_entry, a.entry );
	PUT_LITTLE_END( hdr.a_tload, a.text_start );
	PUT_LITTLE_END( hdr.a_dload, a.data_start );
	xwrite( outfd, &a, sizeof(a) );

	/* Generate section headers
	 */
	put_shdr( outfd, ".text", hdr.a_tload, hdr.a_text, STYP_TEXT,
							HDRSIZE, 0 );
	put_shdr( outfd, ".data", hdr.a_dload, hdr.a_data, STYP_DATA,
							HDRSIZE+hdr.a_text, 0 );
	put_shdr( outfd, ".bss",  hdr.a_dload+hdr.a_data, hdr.a_bss, STYP_BSS,
							0, 0);

	/* Write text and data
	 */
	buflen = hdr.a_text + hdr.a_data;
	buf = xmalloc( buflen );
	xseek( infd, N_TXTOFF(hdr) );
	xread( infd, buf, buflen );
	xwrite( outfd, buf, buflen );
	free( buf );
}


/******************************************************************************
 * strip_coff:
 *	Strip symbol and relocation information from a COFF file.
 *
 ******************************************************************************/
static void
strip_coff( infd, outfd )
    int infd;	/* Descriptor on which COFF file should be read.  File type
		 *	assumed already to have been validated.
		 */
    int outfd;	/* Descriptor on which to write out stripped file.	*/
{
	char *buf;	/* Dynamic buffer for copying from infd to outf */
	struct filehdr f;	/* Header of output file		*/
	struct aouthdr a;	/* "Optional" a.out header of output file */
	struct scnhdr *shdrs;	/* Array of section headers from infd */
	int shdr_size;		/* Size of shdrs array, in bytes	*/
	struct scnhdr *sP;	/* Pointer to an entry in shdrs array	*/
	int nsecs;		/* Number of sections (& sect. hdrs) in files */
	int pos;		/* Address in 'outfd' to which next section
				 *	will be written.
				 */
	int sec_size;		/* Size of next section to be copied	*/
	int i;			/* Loop counter				*/


	xseek( infd, 0 );

	/* Read file header, strip it, put it into i80960 byte order, write
	 * it out.
	 */
	xread( infd, &f, sizeof(f) );
	nsecs = f.f_nscns;
	MAKE_LITTLE_END( f.f_magic );
	MAKE_LITTLE_END( f.f_nscns );
	MAKE_LITTLE_END( f.f_timdat );
	f.f_nsyms = f.f_symptr = 0;
	MAKE_LITTLE_END( f.f_opthdr );
	f.f_flags |= F_RELFLG | F_LNNO | F_LSYMS;
	MAKE_LITTLE_END( f.f_flags );
	xwrite( outfd, &f, sizeof(f) );

	/* If there's an "optional" aout header:  read it, put into i80960
	 * byte order, write it out.
	 */
	if ( f.f_opthdr ){
		xread( infd, &a, sizeof(a) );
		MAKE_LITTLE_END( a.magic );
		MAKE_LITTLE_END( a.vstamp );
		MAKE_LITTLE_END( a.tsize );
		MAKE_LITTLE_END( a.dsize );
		MAKE_LITTLE_END( a.bsize );
		MAKE_LITTLE_END( a.entry );
		MAKE_LITTLE_END( a.text_start );
		MAKE_LITTLE_END( a.data_start );
		a.tagentries = 0;	/* ASSUMES THERE NEVER ARE ANY */
		xwrite( outfd, &a, sizeof(a) );
	}

	/* Read in ALL the section headers.
	 */
	shdr_size = nsecs * sizeof(struct scnhdr);
	shdrs = (struct scnhdr *) xmalloc( shdr_size ); 
	xread( infd, shdrs, shdr_size );

	/* Write out section headers, one at a time.  For robustness, we will
	 * NOT assume that section binary immediately follows the headers, nor
	 * will we assume that the section binaries come in the same order as
	 * the corresponding section headers -- although we'll make both
	 * assumptions true in the output file.  Therefore, we update the
	 * file pointer of each section via the variable 'pos', which is
	 * initialized to point just after the headers.
	 */
	pos = lseek(outfd,0,L_INCR) + shdr_size;
	for ( sP= shdrs, i = 0; i < nsecs; i++, sP++ ){
		if ( sP->s_scnptr == 0 ){
			/* This section has no binary in the file.
			 */
			put_shdr( outfd, sP->s_name, sP->s_paddr, sP->s_size,
						sP->s_flags, 0, sP->s_align );
		} else {
			/* This section DOES have binary: update 'pos'.
			 */
			put_shdr( outfd, sP->s_name, sP->s_paddr, sP->s_size,
						sP->s_flags, pos, sP->s_align );
			pos += sP->s_size;
		}
	}

	/* Copy binary for each section.
	 */
	for ( i = 0; i < nsecs; i++ ){
		if ( shdrs[i].s_scnptr != 0 ){
			sec_size = shdrs[i].s_size;
			buf = xmalloc( sec_size );
			xseek( infd, shdrs[i].s_scnptr );
			xread( infd, buf, sec_size );
			xwrite( outfd, buf, sec_size );
			free( buf );
		}
	}
	free( shdrs );
}

/* NOTE!:
 *	All of the following routines assume that the jumpbuf 'ioerr' has
 *	been set up to restore control correctly after an error is detected.
 */


/******************************************************************************
 * xmalloc:
 *	Perform memory allocation with error checking.
 *	Return pointer to allocated memory (guaranteed non-NULL).
 ******************************************************************************/
static char *
xmalloc( len )
    int len;	/* Allocate this many bytes	*/
{
	char *p;	/* Pointer to allocated memory */
	extern char *malloc();

	p = malloc( len );
	if ( p == NULL ){
		perror( "malloc failed" );
		longjmp( ioerr, 1 );
	}
	return p;
}

/******************************************************************************
 * xopen:
 *	Open file for I/O, with error checking.
 *	Return file descriptor.
 ******************************************************************************/
static int
xopen( fn, mode )
    char *fn;	/* Name of file			*/
    int mode;	/* Mode in which to open it	*/
{
	int fd;	/* File descriptor	*/

	fd = open( fn, mode, 0777 );
	if ( fd < 0 ){
		perror( fn );
		longjmp( ioerr, 1 );
	}
	return fd;
}


/******************************************************************************
 * xread:
 *	Read from file, with error checking.
 ******************************************************************************/
static void
xread( fd, bufp, n )
    int fd;	/* File descriptor		*/
    char *bufp;	/* Pointer to input buffer	*/
    int n;	/* Size of input buffer		*/
{
	if ( read( fd, bufp, n ) != n ){
		perror( "File read failed" );
		longjmp( ioerr, 1 );
	}
}


/******************************************************************************
 * xseek:
 *	Seek to location relative to beginning of file, with error checking.
 *	Return position.
 ******************************************************************************/
static int
xseek( fd, offset )
    int fd;	/* File descriptor					*/
    int offset;	/* Offset into file at which to position I/O pointer	*/
{
	int pos;

	pos = lseek( fd, offset, L_SET );

 	if ( pos == -1 ){
		perror( "Seek failed" );
		longjmp( ioerr, 1 );
	}
	return pos;
}


/******************************************************************************
 * xwrite:
 *	Write to file, with error checking.
 ******************************************************************************/
static void
xwrite( fd, bufp, n )
    int fd;	/* File descriptor		*/
    char *bufp;	/* Pointer to output buffer	*/
    int n;	/* Size of output buffer	*/
{
	if ( write( fd, bufp, n ) != n ){
		perror( "File write failed" );
		longjmp( ioerr, 1 );
	}
}
