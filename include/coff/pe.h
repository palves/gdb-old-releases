/* PE COFF header information */

#ifndef _PE_H
#define _PE_H

/* additional flags to be set for section headers to allow the NT loader to
   read and write to the section data (to replace the addresses of data in
   dlls for one thing); also to execute the section in .text's case */
#define IMAGE_SCN_MEM_DISCARDABLE 0x02000000
#define IMAGE_SCN_MEM_EXECUTE     0x20000000
#define IMAGE_SCN_MEM_READ        0x40000000
#define IMAGE_SCN_MEM_WRITE       0x80000000


  /* NT allows long filenames, we want to accommodate this.  This may break
     some of the bfd functions */
#undef  FILNMLEN
#define FILNMLEN	18	/* # characters in a file name		*/


#ifdef COFF_IMAGE_WITH_PE
/* The filehdr is only weired in images */

#undef FILHDR
struct external_PE_filehdr
{
  /* DOS header fields */
  char e_magic[2];		/* Magic number, 0x5a4d */
  char e_cblp[2];		/* Bytes on last page of file, 0x90 */
  char e_cp[2];			/* Pages in file, 0x3 */
  char e_crlc[2];		/* Relocations, 0x0 */
  char e_cparhdr[2];		/* Size of header in paragraphs, 0x4 */
  char e_minalloc[2];		/* Minimum extra paragraphs needed, 0x0 */
  char e_maxalloc[2];		/* Maximum extra paragraphs needed, 0xFFFF */
  char e_ss[2];			/* Initial (relative) SS value, 0x0 */
  char e_sp[2];			/* Initial SP value, 0xb8 */
  char e_csum[2];		/* Checksum, 0x0 */
  char e_ip[2];			/* Initial IP value, 0x0 */
  char e_cs[2];			/* Initial (relative) CS value, 0x0 */
  char e_lfarlc[2];		/* File address of relocation table, 0x40 */
  char e_ovno[2];		/* Overlay number, 0x0 */
  char e_res[4][2];		/* Reserved words, all 0x0 */
  char e_oemid[2];		/* OEM identifier (for e_oeminfo), 0x0 */
  char e_oeminfo[2];		/* OEM information; e_oemid specific, 0x0 */
  char e_res2[10][2];		/* Reserved words, all 0x0 */
  char e_lfanew[4];		/* File address of new exe header, 0x80 */
  char dos_message[16][4];	/* other stuff, always follow DOS header */
  char nt_signature[4];		/* required NT signature, 0x4550 */ 

  /* From standard header */  


  char f_magic[2];		/* magic number			*/
  char f_nscns[2];		/* number of sections		*/
  char f_timdat[4];		/* time & date stamp		*/
  char f_symptr[4];		/* file pointer to symtab	*/
  char f_nsyms[4];		/* number of symtab entries	*/
  char f_opthdr[2];		/* sizeof(optional hdr)		*/
  char f_flags[2];		/* flags			*/

};


#define FILHDR struct external_PE_filehdr


#endif

typedef struct 
{
  AOUTHDR standard;

  /* NT extra fields; see internal.h for descriptions */
  char  ImageBase[4];
  char  SectionAlignment[4];
  char  FileAlignment[4];
  char  MajorOperatingSystemVersion[2];
  char  MinorOperatingSystemVersion[2];
  char  MajorImageVersion[2];
  char  MinorImageVersion[2];
  char  MajorSubsystemVersion[2];
  char  MinorSubsystemVersion[2];
  char  Reserved1[4];
  char  SizeOfImage[4];
  char  SizeOfHeaders[4];
  char  CheckSum[4];
  char  Subsystem[2];
  char  DllCharacteristics[2];
  char  SizeOfStackReserve[4];
  char  SizeOfStackCommit[4];
  char  SizeOfHeapReserve[4];
  char  SizeOfHeapCommit[4];
  char  LoaderFlags[4];
  char  NumberOfRvaAndSizes[4];
  /* IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES]; */
  char  DataDirectory[16][2][4]; /* 16 entries, 2 elements/entry, 4 chars */

} PEAOUTHDR;


#undef AOUTSZ
#define AOUTSZ sizeof(PEAOUTHDR)

#undef  E_FILNMLEN
#define E_FILNMLEN	18	/* # characters in a file name		*/
#endif



