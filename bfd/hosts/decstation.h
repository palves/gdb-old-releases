#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <string.h>
#include <sys/file.h>
#include <malloc.h>
#ifndef O_ACCMODE
#define O_ACCMODE (O_RDONLY | O_WRONLY | O_RDWR)
#endif
#define SEEK_SET 0
#define SEEK_CUR 1

#include <machine/param.h>
#include <machine/vmparam.h>
#define	HOST_PAGE_SIZE		NBPG
/* #define	HOST_SEGMENT_SIZE	NBPG -- we use HOST_DATA_START_ADDR */
#define	HOST_MACHINE_ARCH	bfd_arch_mips
/* #define	HOST_MACHINE_MACHINE	 */

#define	HOST_TEXT_START_ADDR		USRTEXT
#define	HOST_DATA_START_ADDR		USRDATA
#define	HOST_STACK_END_ADDR		USRSTACK

#define TRAD_UNIX_CORE_FILE_FAILING_SIGNAL(core_bfd) \
  ((core_bfd)->tdata.trad_core_data->u.u_arg[0])

#include "fopen-same.h"

/* EXACT TYPES */
typedef char int8e_type;
typedef unsigned char uint8e_type;
typedef short int16e_type;
typedef unsigned short uint16e_type;
typedef int int32e_type;
typedef unsigned int uint32e_type;

/* CORRECT SIZE OR GREATER */
typedef char int8_type;
typedef unsigned char uint8_type;
typedef short int16_type;
typedef unsigned short uint16_type;
typedef int int32_type;
typedef unsigned int uint32_type;

/* other */

extern char *getenv();
