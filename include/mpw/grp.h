/* $Id: grp.h,v 1.1 1994/04/14 19:57:48 shebs Exp $ */

#pragma once

#include "sys/types.h"

struct group {
  char *gr_name;
  gid_t gr_gid;
  char *gr_passwd;
  char **gr_mem;
};
