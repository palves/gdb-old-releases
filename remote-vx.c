/* Memory-access and commands for remote VxWorks processes, for GDB.
   Copyright (C) 1990  Free Software Foundation, Inc.

This file is part of GDB.

GDB is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 1, or (at your option)
any later version.

GDB is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GDB; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

#include "defs.h"
#include "tm-sun3.h"
#include "param-no-tm.h"
#include "frame.h"
#include "inferior.h"
#include "wait.h"
#include "target.h"
#include "gdbcore.h"
#include "command.h"
#include "symtab.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#define free bogon_free		/* Sun claims "int free()" not void */
#include <rpc/rpc.h>
#undef free
#include <netdb.h>
#include <ptrace.h>
#include "xdr_ptrace.h"
#include "xdr_ld.h"
#include "xdr_rdb.h"
#include "dbgRpcLib.h"

/* get rid of value.h if possible */
#include <value.h>
#include <symtab.h>
 
extern value call_function_by_hand ();
extern void symbol_file_command ();
extern int stop_soon_quietly;		/* for wait_for_inferior */

extern struct target_ops vx_ops, vx_run_ops;	/* Forward declaration */

/* Saved name of target host and called function for "info files".
   Both malloc'd.  */

static char *vx_host;
static char *vx_running;

/* Nonzero means target that is being debugged remotely has a floating
   point processor.  */

static int target_has_fp;

/* Default error message when the network is forking up.  */

static const char rpcerr[] = "network target debugging:  rpc error";

CLIENT *pClient;         /* client used in net debugging */
static int ptraceSock = RPC_ANYSOCK;
extern int errno;

enum clnt_stat net_clnt_call();
static void parse_args ();

static struct timeval rpcTimeout = { 10, 0 };

static char *skip_white_space ();
static char *find_white_space ();
 
/* returns -1 if error, 0 otherwise.  Load addresses of text, data, and bss 
   segments are stored at pTextAddr, pDataAddr, and pBssAddr (respectively). */

int
net_load (filename, pTextAddr, pDataAddr, pBssAddr)
    char *filename;
    CORE_ADDR *pTextAddr;
    CORE_ADDR *pDataAddr;
    CORE_ADDR *pBssAddr;
    {
    enum clnt_stat status;
    struct ldfile ldstruct;
 
    bzero ((char *) &ldstruct, sizeof (ldstruct));
 
    status = net_clnt_call (VX_LOAD, xdr_wrapstring, &filename, xdr_ldfile,
			    &ldstruct);
    if (status == RPC_SUCCESS)
        {
        if (*ldstruct.name == NULL)
            {
            /* load didn't succeed on the VxWorks side */
             
	    printf ("GDB:  LOAD FAILED!!!!!!************\n");
	    fflush (stdout);
            return -1;
            }
	*pTextAddr = ldstruct.txt_addr;
	*pDataAddr = ldstruct.data_addr;
	*pBssAddr = ldstruct.bss_addr;
	return 0;
        }
    else
        return -1;
    }
      
/* returns 0 if successful, errno otherwise */

int
vx_insert_breakpoint (addr)
    int addr;
    {
    int status;

    if ((status = net_break (addr, VX_BREAK_ADD)) == -1)
	error ("Can't insert remote breakpoints.");
    else
	return status;
    }

/* returns 0 if successful, errno otherwise */

int
vx_remove_breakpoint (addr)
    int addr;
    {
    int status;

    if ((status = net_break (addr, VX_BREAK_DELETE)) == -1)
	error ("Can't delete remote breakpoints.");
    else
	return status;
    }

/* returns 0 if successful, -1 if failed on VxWorks side, errno if RPC failed */

int
net_break (addr, procnum)
    int addr;
    u_long procnum;
    {
    enum clnt_stat status;
    int break_status;
    Rptrace ptrace_in;  /* XXX This is stupid.  It doesn't need to be a ptrace
                         * structure.  How about something smaller?
                         */

    switch (procnum)
        {
        case VX_BREAK_DELETE:
          break;
        case VX_BREAK_ADD:
          break;
        default:
	  error ("Unknown break procedure %d. Call Wind River Tech Support\n");
        }

    bzero ((char *) &ptrace_in, sizeof (ptrace_in));
    break_status = 0;

    ptrace_in.addr = addr;
    ptrace_in.pid = inferior_pid;

    status = net_clnt_call (procnum, xdr_rptrace, &ptrace_in, xdr_int, &break_status);

    if (status != RPC_SUCCESS)
	return errno;

    return break_status;
    }
 
/* Perform a function call in the inferior.
   ARGS is a vector of values of arguments (NARGS of them).
   FUNCTION is a value, the function to be called.
   Returns a value representing what the function returned.
   May fail to return, if a breakpoint or signal is hit
   during the execution of the function.  */

#ifdef FIXME
/* FIXME, function calls are really fried.  GO back to manual method. */
value
vx_call_function (function, nargs, args)
     value function;
     int nargs;
     value *args;
{
  register CORE_ADDR sp;
  register int i;
  CORE_ADDR start_sp;
  static REGISTER_TYPE dummy[] = CALL_DUMMY;
  REGISTER_TYPE dummy1[sizeof dummy / sizeof (REGISTER_TYPE)];
  CORE_ADDR old_sp;
  struct type *value_type;
  unsigned char struct_return;
  CORE_ADDR struct_addr;
  struct inferior_status inf_status;
  struct cleanup *old_chain;
  CORE_ADDR funaddr;
  int using_gcc;

  save_inferior_status (&inf_status, 1);
  old_chain = make_cleanup (restore_inferior_status, &inf_status);

  /* PUSH_DUMMY_FRAME is responsible for saving the inferior registers
     (and POP_FRAME for restoring them).  (At least on most machines)
     they are saved on the stack in the inferior.  */
  PUSH_DUMMY_FRAME;

  old_sp = sp = read_register (SP_REGNUM);

#if 1 INNER_THAN 2		/* Stack grows down */
  sp -= sizeof dummy;
  start_sp = sp;
#else				/* Stack grows up */
  start_sp = sp;
  sp += sizeof dummy;
#endif

  funaddr = find_function_addr (function, &value_type);

  {
    struct block *b = block_for_pc (funaddr);
    /* If compiled without -g, assume GCC.  */
    using_gcc = b == NULL || BLOCK_GCC_COMPILED (b);
  }

  /* Are we returning a value using a structure return or a normal
     value return? */

  struct_return = using_struct_return (function, funaddr, value_type,
				       using_gcc);

  /* Create a call sequence customized for this function
     and the number of arguments for it.  */
  bcopy (dummy, dummy1, sizeof dummy);
  FIX_CALL_DUMMY (dummy1, start_sp, funaddr, nargs, args,
		  value_type, using_gcc);

#if CALL_DUMMY_LOCATION == ON_STACK
  write_memory (start_sp, dummy1, sizeof dummy);

#else /* Not on stack.  */
#if CALL_DUMMY_LOCATION == BEFORE_TEXT_END
  /* Convex Unix prohibits executing in the stack segment. */
  /* Hope there is empty room at the top of the text segment. */
  {
    static checked = 0;
    if (!checked)
      for (start_sp = text_end - sizeof dummy; start_sp < text_end; ++start_sp)
	if (read_memory_integer (start_sp, 1) != 0)
	  error ("text segment full -- no place to put call");
    checked = 1;
    sp = old_sp;
    start_sp = text_end - sizeof dummy;
    write_memory (start_sp, dummy1, sizeof dummy);
  }
#else /* After text_end.  */
  {
    int errcode;
    sp = old_sp;
    start_sp = text_end;
    errcode = target_write_memory (start_sp, dummy1, sizeof dummy);
    if (errcode != 0)
      error ("Cannot write text segment -- call_function failed");
  }
#endif /* After text_end.  */
#endif /* Not on stack.  */

#ifdef STACK_ALIGN
  /* If stack grows down, we must leave a hole at the top. */
  {
    int len = 0;

    /* Reserve space for the return structure to be written on the
       stack, if necessary */

    if (struct_return)
      len += TYPE_LENGTH (value_type);
    
    for (i = nargs - 1; i >= 0; i--)
      len += TYPE_LENGTH (VALUE_TYPE (value_arg_coerce (args[i])));
#ifdef CALL_DUMMY_STACK_ADJUST
    len += CALL_DUMMY_STACK_ADJUST;
#endif
#if 1 INNER_THAN 2
    sp -= STACK_ALIGN (len) - len;
#else
    sp += STACK_ALIGN (len) - len;
#endif
  }
#endif /* STACK_ALIGN */

    /* Reserve space for the return structure to be written on the
       stack, if necessary */

    if (struct_return)
      {
#if 1 INNER_THAN 2
	sp -= TYPE_LENGTH (value_type);
	struct_addr = sp;
#else
	struct_addr = sp;
	sp += TYPE_LENGTH (value_type);
#endif
      }

#if defined (REG_STRUCT_HAS_ADDR)
  {
    /* This is a machine like the sparc, where we need to pass a pointer
       to the structure, not the structure itself.  */
    if (REG_STRUCT_HAS_ADDR (using_gcc))
      for (i = nargs - 1; i >= 0; i--)
	if (TYPE_CODE (VALUE_TYPE (args[i])) == TYPE_CODE_STRUCT)
	  {
	    CORE_ADDR addr;
#if !(1 INNER_THAN 2)
	    /* The stack grows up, so the address of the thing we push
	       is the stack pointer before we push it.  */
	    addr = sp;
#endif
	    /* Push the structure.  */
	    sp = value_push (sp, args[i]);
#if 1 INNER_THAN 2
	    /* The stack grows down, so the address of the thing we push
	       is the stack pointer after we push it.  */
	    addr = sp;
#endif
	    /* The value we're going to pass is the address of the thing
	       we just pushed.  */
	    args[i] = value_from_long (builtin_type_long, (LONGEST) addr);
	  }
  }
#endif /* REG_STRUCT_HAS_ADDR.  */

#ifdef PUSH_ARGUMENTS
  PUSH_ARGUMENTS(nargs, args, sp, struct_return, struct_addr);
#else /* !PUSH_ARGUMENTS */
  for (i = nargs - 1; i >= 0; i--)
    sp = value_arg_push (sp, args[i]);
#endif /* !PUSH_ARGUMENTS */

#ifdef CALL_DUMMY_STACK_ADJUST
#if 1 INNER_THAN 2
  sp -= CALL_DUMMY_STACK_ADJUST;
#else
  sp += CALL_DUMMY_STACK_ADJUST;
#endif
#endif /* CALL_DUMMY_STACK_ADJUST */

  /* Store the address at which the structure is supposed to be
     written.  Note that this (and the code which reserved the space
     above) assumes that gcc was used to compile this function.  Since
     it doesn't cost us anything but space and if the function is pcc
     it will ignore this value, we will make that assumption.

     Also note that on some machines (like the sparc) pcc uses a 
     convention like gcc's.  */

  if (struct_return)
    STORE_STRUCT_RETURN (struct_addr, sp);

  /* Write the stack pointer.  This is here because the statements above
     might fool with it.  On SPARC, this write also stores the register
     window into the right place in the new stack frame, which otherwise
     wouldn't happen.  (See write_inferior_registers in sparc-xdep.c.)  */
  write_register (SP_REGNUM, sp);

  /* Figure out the value returned by the function.  */
  {
    char retbuf[REGISTER_BYTES];

    /* Execute the stack dummy routine, calling FUNCTION.
       When it is done, discard the empty frame
       after storing the contents of all regs into retbuf.  */
    run_stack_dummy (start_sp + CALL_DUMMY_START_OFFSET, retbuf);

    do_cleanups (old_chain);

    return value_being_returned (value_type, retbuf, struct_return);
  }
}
/* should return a value of some sort */
 
value
vx_call_function (funcAddr, nargs, args, valueType)
    char *funcAddr;
    int nargs;
    value *args;
    struct type * valueType;
{
    int i;
    func_call funcInfo;
    arg_value *argValue;
    enum clnt_stat status;
    register int len;
    arg_value funcReturn;
    value gdbValue;

    argValue = (arg_value *) xmalloc (nargs * sizeof (arg_value));

    bzero (argValue, nargs * sizeof (arg_value));
    bzero (&funcReturn, sizeof (funcReturn));

    for (i = nargs - 1; i >= 0; i--)
	{
	len = TYPE_LENGTH (VALUE_TYPE (args [i]));

	switch (TYPE_CODE (VALUE_TYPE (args[i])))
	    {
	    /* XXX put other types here.  Where's CHAR, etc??? */

	    case TYPE_CODE_FLT:
		argValue[i].type = T_FLOAT;
		break;
	    case TYPE_CODE_INT:
	    case TYPE_CODE_PTR:
	    case TYPE_CODE_ENUM:
	    case TYPE_CODE_FUNC:
		argValue[i].type = T_INT;
		break;

	    case TYPE_CODE_UNDEF:
	    case TYPE_CODE_ARRAY:
	    case TYPE_CODE_STRUCT:
	    case TYPE_CODE_UNION:
	    case TYPE_CODE_VOID:
	    case TYPE_CODE_SET:
	    case TYPE_CODE_RANGE:
	    case TYPE_CODE_PASCAL_ARRAY:
	    case TYPE_CODE_MEMBER:	    /* C++ */
	    case TYPE_CODE_METHOD:	    /* C++ */
	    case TYPE_CODE_REF:		    /* C++ */
	    default:
		error ("No corresponding VxWorks type for %d.  CHECK IT OUT!!!\n",
			TYPE_CODE(VALUE_TYPE(args[i])));
	    } /* switch */
	if (TYPE_CODE(VALUE_TYPE(args[i])) == TYPE_CODE_FUNC)
	    argValue[i].arg_value_u.v_int = VALUE_ADDRESS(args[i]);
	else
	    bcopy (VALUE_CONTENTS (args[i]), (char *) &argValue[i].arg_value_u,
	       	   len);
	}

    /* XXX what should the type of this function addr be?
     * XXX Both in gdb and vxWorks
     */
    funcInfo.func_addr = (int) funcAddr;
    funcInfo.args.args_len = nargs;
    funcInfo.args.args_val = argValue;

    status = net_clnt_call (VX_CALL_FUNC, xdr_func_call, (char *) &funcInfo,
			    xdr_arg_value, &funcReturn);

    free ((char *) argValue);

    if (status == RPC_SUCCESS)
	{
	/* XXX this assumes that vxWorks ALWAYS returns an int, and that
	 * XXX gdb isn't expecting anything more
	 */

	/*******************
	if (funcReturn.type == T_UNKNOWN)
	    return YYYXXX...;
	*******************/
	gdbValue = allocate_value (valueType);
	bcopy (&funcReturn.arg_value_u.v_int, VALUE_CONTENTS (gdbValue),
		sizeof (int));
        return gdbValue;
	}
    else 
	error (rpcerr);
    }
#endif /* FIXME */
 
/* Start an inferior process and sets inferior_pid to its pid.
   EXEC_FILE is the file to run.
   ALLARGS is a string containing the arguments to the program.
   ENV is the environment vector to pass.
   Returns process id.  Errors reported with error().
   On VxWorks, we ignore exec_file.  */
 
void
vx_create_inferior (exec_file, args, env)
     char *exec_file;
     char *args;
     char **env;
{
  enum clnt_stat status;
  arg_array passArgs;
  TASK_START taskStart;

  bzero ((char *) &passArgs, sizeof (passArgs));
  bzero ((char *) &taskStart, sizeof (taskStart));

  /* parse arguments, put them in passArgs */

  parse_args (args, &passArgs);

  if (passArgs.arg_array_len == 0)
    error ("You must specify a function name to run, and arguments if any");

  status = net_clnt_call (PROCESS_START, xdr_arg_array, &passArgs,
			  xdr_TASK_START, &taskStart);

  if ((status != RPC_SUCCESS) || (taskStart.status == -1))
    error ("Can't create process on remote target machine");

  /* Save the name of the running function */
  if (vx_running)
    free (vx_running);
  vx_running = savestring (passArgs.arg_array_val[0],
			   strlen (passArgs.arg_array_val[0]));

#ifdef CREATE_INFERIOR_HOOK
  CREATE_INFERIOR_HOOK (pid);
#endif  

  push_target (&vx_run_ops);
  inferior_pid = taskStart.pid;

#if defined (START_INFERIOR_HOOK)
  START_INFERIOR_HOOK ();
#endif

  /* We will get a trace trap after one instruction.
     Insert breakpoints and continue.  */

  init_wait_for_inferior ();

  /* Set up the "saved terminal modes" of the inferior
     based on what modes we are starting it with.  */
  target_terminal_init ();

  /* Install inferior's terminal modes.  */
  target_terminal_inferior ();

  /* remote_start(args); */
  /* trap_expected = 0; */
  stop_soon_quietly = 1;
  wait_for_inferior ();		/* Get the task spawn event */
  stop_soon_quietly = 0;

  /* insert_step_breakpoint ();  FIXME, do we need this?  */
  proceed(-1, -1, 0);
}

/* fills ARGSTRUCT in argc, argv form with the arguments from the
 * argument string ARGSTRING */

static void
parse_args (argString, argStruct)
    char *argString;
    arg_array *argStruct;

    {
    register int argCount = 0;	/* number of arguments */
    register int argIndex = 0;
    register char *pArg;
    int argLen;
    char *white_space = " \t";
    char *p0;
 
    bzero ((char *) argStruct, sizeof (arg_array));
 
    /* parse arguments */

    /* first count how many arguments there are */

    p0 = argString;
    while (*p0 != '\0')
	{
	if ((p0 = skip_white_space (p0)) == '\0')
	    break;
	p0 = find_white_space (p0);
	argCount++;
	}

    argStruct->arg_array_len = argCount;
    argStruct->arg_array_val = (char **)
	xmalloc ( (1+argCount) * sizeof (char *) );

    p0 = argString;
    while (*p0 != '\0')
	{
	if ((p0 = skip_white_space (p0)) == '\0')
	    break;
	argLen = strcspn (p0, white_space);
	pArg = argStruct->arg_array_val [argIndex] = xmalloc (argLen + 1);
	strncpy (pArg, p0, argLen);
	pArg [argLen] = '\0';		/* null terminate string */
	p0 = find_white_space (p0);
	argIndex++;
	}

    argStruct->arg_array_val [argCount] = NULL;
    }

static char *
skip_white_space (p)
    register char *p;
    {
    while (*p == ' ' || *p == '\t')
	p++;
    return p;
    }
    
static char *
find_white_space (p)
    register char *p;
    {
    while (*p != ' ' && *p != '\t')
	{
	if (*p == '\0')
	    return p;
	p++;
	}
    return p;
    }
    
/* returns -1 if remote wait failed, 0 otherwise
 * calls error() if rpc failed
 */

int
net_wait (pEvent)
    RDB_EVENT *pEvent;
    {
    int pid;
    enum clnt_stat status;

    bzero ((char *) pEvent, sizeof (RDB_EVENT));

    pid = inferior_pid;
    status = net_clnt_call (PROCESS_WAIT, xdr_int, &pid, xdr_RDB_EVENT, pEvent);

    if (status == RPC_SUCCESS)
        return pEvent->status;
    else 
	error (rpcerr);
    }
    
/* returns -1 if remote task suspend failed, 0 otherwise
 * calls error() if rpc failed
 */

int
net_quit ()
    {
    int pid;
    int quit_status;
    enum clnt_stat status;

    quit_status = 0;

    /* don't let gdbTask suspend itself by passing a pid of 0 */
    if ((pid = inferior_pid) == 0)
	return -1;

    status = net_clnt_call (VX_TASK_SUSPEND, xdr_int, &pid, xdr_int, &quit_status);

    if (status == RPC_SUCCESS)
        return quit_status;
    else 
	error (rpcerr);
    }

/* Read a register or registers from the remote system.  */

int
vx_read_register (regno)
     int regno;
{
  enum clnt_stat status;
  Rptrace ptrace_in;
  Ptrace_return ptrace_out;
  struct regs inferior_registers;
  struct fp_status inferior_fp_registers;
  extern char registers[];

  bzero ((char *) &ptrace_in, sizeof (ptrace_in));
  bzero ((char *) &ptrace_out, sizeof (ptrace_out));

  /* FIXME, eventually only get the ones we need.  */
  registers_fetched ();
  
  ptrace_in.pid = inferior_pid;
  ptrace_out.info.more_data = (caddr_t) &inferior_registers;
  status = net_ptrace_clnt_call (PTRACE_GETREGS, &ptrace_in, &ptrace_out);
  if (status == -1)
    error (rpcerr);
  if (ptrace_out.status == -1)
    {
      errno = ptrace_out.errno;
      return -1;
    }
  
  bcopy (&inferior_registers, registers, 16 * 4);
  *(int *)&registers[REGISTER_BYTE (PS_REGNUM)] = inferior_registers.r_ps;
  *(int *)&registers[REGISTER_BYTE (PC_REGNUM)] = inferior_registers.r_pc;

  if (target_has_fp)
    {
      ptrace_in.pid = inferior_pid;
      ptrace_out.info.more_data = (caddr_t) &inferior_fp_registers;
      status = net_ptrace_clnt_call (PTRACE_GETFPREGS, &ptrace_in, &ptrace_out);
      if (status == -1)
	error (rpcerr);
      if (ptrace_out.status == -1)
	{
	  errno = ptrace_out.errno;
	  return -1;
	}
      
      bcopy (&inferior_fp_registers, &registers[REGISTER_BYTE (FP0_REGNUM)],
	     sizeof inferior_fp_registers.fps_regs);
      bcopy (&inferior_fp_registers.fps_control,
	 &registers[REGISTER_BYTE (FPC_REGNUM)],
	 sizeof inferior_fp_registers - sizeof inferior_fp_registers.fps_regs);
    }
  return 0;
}

/* Prepare to store registers.  Since we will store all of them,
   read out their current values now.  */

void
vx_prepare_to_store ()
{
  vx_read_register (-1);
}


/* Store our register values back into the inferior.
   If REGNO is -1, do this for all registers.
   Otherwise, REGNO specifies which register (so we can save time).  */
   /* FIXME, look at REGNO to save time here */

vx_write_register (regno)
     int regno;
{
  struct regs inferior_registers;
  struct fp_status inferior_fp_registers;
  extern char registers[];
  enum clnt_stat status;
  Rptrace ptrace_in;
  Ptrace_return ptrace_out;

  bzero ((char *) &ptrace_in, sizeof (ptrace_in));
  bzero ((char *) &ptrace_out, sizeof (ptrace_out));

  bcopy (registers, &inferior_registers, 16 * 4);
  inferior_registers.r_ps = *(int *)&registers[REGISTER_BYTE (PS_REGNUM)];
  inferior_registers.r_pc = *(int *)&registers[REGISTER_BYTE (PC_REGNUM)];
  ptrace (PTRACE_SETREGS, inferior_pid, &inferior_registers);
  ptrace_in.pid = inferior_pid;
  ptrace_in.info.ttype     = REGS;
  ptrace_in.info.more_data = (caddr_t) &inferior_registers;

  /* XXX change second param to be a proc number */
  status = net_ptrace_clnt_call (PTRACE_SETREGS, &ptrace_in, &ptrace_out);
  if (status == -1)
      error (rpcerr);
  if (ptrace_out.status == -1)
    {
      errno = ptrace_out.errno;
      return -1;
    }

  if (target_has_fp)
    {
      bcopy (&registers[REGISTER_BYTE (FP0_REGNUM)], &inferior_fp_registers,
	     sizeof inferior_fp_registers.fps_regs);
      bcopy (&registers[REGISTER_BYTE (FPC_REGNUM)],
	 &inferior_fp_registers.fps_control,
         sizeof inferior_fp_registers - sizeof inferior_fp_registers.fps_regs);

      ptrace_in.pid = inferior_pid;
      ptrace_in.info.ttype     = FPREGS;
      ptrace_in.info.more_data = (caddr_t) &inferior_fp_registers;

      status = net_ptrace_clnt_call (PTRACE_SETFPREGS, &ptrace_in, &ptrace_out);
      if (status == -1)
	  error (rpcerr);
      if (ptrace_out.status == -1)
	{
	  errno = ptrace_out.errno;
	  return -1;
	}
    }
  return 0;
}

/* Copy LEN bytes to or from remote inferior's memory starting at MEMADDR
   to debugger memory starting at MYADDR.  WRITE is true if writing to the
   inferior.
   On failure (cannot access inferior, usually because address is out
   of bounds) returns the value of errno. */

int
vx_xfer_memory (memaddr, myaddr, len, write)
     CORE_ADDR memaddr;
     char *myaddr;
     int len;
{
  enum clnt_stat status;
  Rptrace ptrace_in;
  Ptrace_return ptrace_out;
  C_bytes data;

  bzero ((char *) &ptrace_in, sizeof (ptrace_in));
  bzero ((char *) &ptrace_out, sizeof (ptrace_out));

  ptrace_in.pid = inferior_pid;		/* XXX pid unnecessary for READDATA */
  ptrace_in.addr = (int) memaddr;	/* Where from */
  ptrace_in.data = len;			/* How many bytes */

  if (write)
    {
      ptrace_in.info.ttype     = DATA;
      ptrace_in.info.more_data = (caddr_t) &data;

      data.bytes = (caddr_t) myaddr;	/* Where from */
      data.len   = len;			/* How many bytes (again, for XDR) */

      /* XXX change second param to be a proc number */
      status = net_ptrace_clnt_call (PTRACE_WRITEDATA, &ptrace_in, &ptrace_out);
    }
  else
    {
      ptrace_out.info.more_data = (caddr_t) &data;
      data.bytes = myaddr;		/* Where to */
      data.len   = len;			/* How many (again, for XDR) */

      /* XXX change second param to be a proc number */
      status = net_ptrace_clnt_call (PTRACE_READDATA, &ptrace_in, &ptrace_out);
    }

  if (status == -1)
      error (rpcerr);
  if (ptrace_out.status == -1)
    {
      return ptrace_out.errno;
    }
  return 0;
}

void
vx_files_info ()
{
  printf ("\tAttached to host `%s'.\n", vx_host);
}

void
vx_run_files_info ()
{
  printf ("\tRunning VxWorks process 0x%x, function `%s'.\n",
	  inferior_pid, vx_running);
}

void
vx_resume (step, siggnal)
     int step;
     int siggnal;
{
  enum clnt_stat status;
  Rptrace ptrace_in;
  Ptrace_return ptrace_out;

  if (siggnal != 0)
    error ("Cannot send signals to VxWorks processes");

  bzero ((char *) &ptrace_in, sizeof (ptrace_in));
  bzero ((char *) &ptrace_out, sizeof (ptrace_out));

  ptrace_in.pid = inferior_pid;
  ptrace_in.addr = 1;	/* Target side insists on this, or it panics.  */

  /* XXX change second param to be a proc number */
  status = net_ptrace_clnt_call (step? PTRACE_SINGLESTEP: PTRACE_CONT,
				 &ptrace_in, &ptrace_out);
  if (status == -1)
      error (rpcerr);
  if (ptrace_out.status == -1)
    {
      errno = ptrace_out.errno;
      perror_with_name ("Resuming remote process");
    }
}

void
vx_mourn_inferior ()
{
  pop_target ();		/* Pop back to no-child state */
  generic_mourn_inferior ();
}


/* This function allows the addition of incrementally linked object files.  */

void
vx_add_file_command (arg_string, from_tty)
     char* arg_string;
     int from_tty;
{
  CORE_ADDR text_addr;
  CORE_ADDR data_addr;
  CORE_ADDR bss_addr;
  
  if (arg_string == 0)
    error ("add-file takes a file name in VxWorks");

  arg_string = tilde_expand (arg_string);
  make_cleanup (free, arg_string);

  dont_repeat ();

  if (net_load (arg_string, &text_addr, &data_addr, &bss_addr) == -1)
    error ("Load failed on target machine");

  /* FIXME, for now we ignore data_addr and bss_addr.  */
  symbol_file_add (arg_string, from_tty, text_addr, 0);
}

/* returns -1    if ptrace status on VxWorks is bad,
 *        value  if PEEKTEXT call was successful
 *          0    otherwise, errno is set in this case too
 *
 * calls error() if rpc failed
 */

int
net_ptrace (request, pid, arg3, arg4)
    enum ptracereq request;
    int pid;
    char *arg3;
    int arg4;
{
    enum clnt_stat status;
    Rptrace ptrace_in;
    Ptrace_return ptrace_out;
    C_bytes out_data;

    bzero ((char *) &ptrace_in, sizeof (ptrace_in));
    bzero ((char *) &ptrace_out, sizeof (ptrace_out));

    switch (request)
	{
	case PTRACE_GETREGS:
	    ptrace_in.pid = pid;

	    ptrace_out.info.more_data = arg3;
	    bzero (ptrace_out.info.more_data, sizeof (struct regs));

	    /* XXX change second param to be a proc number */

	    if ((status = net_ptrace_clnt_call (request, &ptrace_in,
						&ptrace_out)) == -1)
		error (rpcerr);
	    break;

	case PTRACE_GETFPREGS:
	    ptrace_in.pid = pid;

	    ptrace_out.info.more_data = arg3;
	    bzero (ptrace_out.info.more_data, sizeof (struct fp_status));

	    /* XXX change second param to be a proc number */

	    if ((status = net_ptrace_clnt_call (request, &ptrace_in,
						&ptrace_out)) == -1)
		error (rpcerr);
	    break;

	case PTRACE_SETFPREGS:

	    ptrace_in.pid = pid;
	    ptrace_in.info.ttype     = FPREGS;
	    ptrace_in.info.more_data = arg3;

	    if ((status = net_ptrace_clnt_call (request, &ptrace_in,
						&ptrace_out)) == -1)
		error (rpcerr);
	    break;

	case PTRACE_SETREGS:

	    ptrace_in.pid = pid;
	    ptrace_in.info.ttype     = REGS;
	    ptrace_in.info.more_data = arg3;

	    /* XXX change second param to be a proc number */

	    if ((status = net_ptrace_clnt_call (request, &ptrace_in,
						&ptrace_out)) == -1)
		error (rpcerr);
	    break;

	case PTRACE_ATTACH:
	    ptrace_in.pid = pid;

	    /* XXX change second param to be a proc number */

	    if ((status = net_ptrace_clnt_call (request, &ptrace_in,
						&ptrace_out)) == -1)
		error (rpcerr);
	    break;

	case PTRACE_DETACH:
	    ptrace_in.pid = pid;

	    /* XXX change second param to be a proc number */

	    if ((status = net_ptrace_clnt_call (request, &ptrace_in,
						&ptrace_out)) == -1)
		error (rpcerr);
	    break;

	case PTRACE_KILL:
	    ptrace_in.pid = pid;

	    /* XXX change second param to be a proc number */

	    (void) net_ptrace_clnt_call (request, &ptrace_in, &ptrace_out);
	    /*
	     * This is a special case.  Return 0 always so that if the
	     * target has been rebooted, GDB won't get hung up on trying to kill
	     * the remote task.  GDB attempts to murder tasks when it quits,
	     * or gets a new inferior task, which it does with commands such as
	     * run and attach.
	     */

	    return 0;

	case PTRACE_CONT:
	    ptrace_in.pid = pid;
	    ptrace_in.addr = (int) arg3;

	    /* XXX change second param to be a proc number */

	    if ((status = net_ptrace_clnt_call (request, &ptrace_in,
						&ptrace_out)) == -1)
		error (rpcerr);
	    break;

	case PTRACE_PEEKTEXT:

	    ptrace_in.pid = pid;	/* XXX pid unnecessary for PEEKTEXT */
	    ptrace_in.addr = (int) arg3;

	    ptrace_out.info.more_data = (caddr_t) &out_data;
	    bzero (ptrace_out.info.more_data, sizeof (C_bytes));

	    /* XXX change second param to be a proc number */

	    if ((status = net_ptrace_clnt_call (request, &ptrace_in,
						&ptrace_out)) == -1)
		error (rpcerr);

	    if (ptrace_out.status == -1)
		break;

	    /* Convert the integer data pointed to by out_data.bytes to 
	     * host format.  XDR does not do the conversion, since it
	     * has been told to treat the data as a sequence of bytes.
	     */
	    /* CROSS-DEBUG */

	    return (unpack_long (builtin_type_int, out_data.bytes));

	case PTRACE_POKETEXT:

	    ptrace_in.pid = pid;	/* XXX pid unnecessary for POKETEXT */
	    ptrace_in.data = arg4;
	    ptrace_in.addr = (int) arg3;

	    /* XXX change second param to be a proc number */

	    if ((status = net_ptrace_clnt_call (request, &ptrace_in,
						&ptrace_out)) == -1)
		error (rpcerr);
	    break;

	case PTRACE_SINGLESTEP:
	    ptrace_in.pid = pid;

	    /* XXX change second param to be a proc number */

	    if ((status = net_ptrace_clnt_call (request, &ptrace_in,
						&ptrace_out)) == -1)
		error (rpcerr);
	    break;

	default:
	    error ("Unknown ptrace request %d.\n");
	    break;
	} /* switch on request */

    if (ptrace_out.status == -1)
	{
	errno = ptrace_out.errno;
	fflush (stdout);

	return -1;
	}
    return 0;
}

/* returns 0 if rpc successful, -1 otherwise */

int
net_ptrace_clnt_call (request, pPtraceIn, pPtraceOut)
    enum ptracereq request;
    Rptrace *pPtraceIn;
    Ptrace_return *pPtraceOut;
{
  enum clnt_stat status;

  status = net_clnt_call (request, xdr_rptrace, pPtraceIn, xdr_ptrace_return,
			  pPtraceOut);

  if (status != RPC_SUCCESS)
      return -1;

  return 0;
}

/* Query the target for the name of the file from which VxWorks was
 * booted.  pBootFile is the address of a pointer to the buffer to
 * receive the file name; if the pointer pointed to by pBootFile is 
 * NULL, memory for the buffer will be allocated by XDR.
 * Returns -1 if rpc failed, 0 otherwise.
 */

int
net_get_boot_file (pBootFile)
    char **pBootFile;
    {
    enum clnt_stat status;

    status = net_clnt_call (VX_BOOT_FILE_INQ, xdr_void, (char *) 0,
			    xdr_wrapstring, pBootFile);
    return (status == RPC_SUCCESS) ? 0 : -1;
    }

/* returns -1 if rpc failed, 0 otherwise
 * There's no way to check if the returned loadTable is correct.
 * VxWorks doesn't check it.
 */

int
net_get_symbols (pLoadTable)
    ldtabl *pLoadTable;		/* return pointer to ldtabl here */
    {
    enum clnt_stat status;

    bzero ((char *) pLoadTable, sizeof (struct ldtabl));

    status = net_clnt_call (VX_STATE_INQ, xdr_void, 0, xdr_ldtabl, pLoadTable);
    if (status != RPC_SUCCESS)
        return -1;

     return 0;
     }

/* returns status of symbol read on target side (0=success, -1=fail)
 * calls error() if rpc fails
 */

int
net_lookup_symbol (name, pAddr)
    char *name;		/* symbol name */
    unsigned int *pAddr;
    {
    enum clnt_stat status;
    SYMBOL_ADDR symbolAddr;

    *pAddr = 0;
    bzero ((char *) &symbolAddr, sizeof (symbolAddr));

    status = net_clnt_call (VX_SYMBOL_INQ, xdr_wrapstring, &name, xdr_SYMBOL_ADDR,
			    &symbolAddr);
    if (status != RPC_SUCCESS)
	error (rpcerr);

    *pAddr = symbolAddr.addr;
    return symbolAddr.status;
    }

/* returns 1 if target has floating point processor, 0 otherwise
 * calls error() if rpc fails
 */

int
net_check_for_fp ()
    {
    enum clnt_stat status;
    bool_t fp = 0;	/* true if fp processor is present on target board */

    status = net_clnt_call (VX_FP_INQUIRE, xdr_void, 0, xdr_bool, &fp);
    if (status != RPC_SUCCESS)
        error (rpcerr);

     return (int) fp;
     }

/* no return value.  calls error if error occurs. */

void
net_connect (host)
    char *host;
    {
    struct sockaddr_in destAddr;
    struct hostent *destHost;

    /* get the internet address for the given host */

    if ((destHost = (struct hostent *) gethostbyname (host)) == NULL)
	error ("Invalid hostname.  Couldn't attach remote target.");

    bzero (&destAddr, sizeof (destAddr));

    destAddr.sin_addr.s_addr = * (u_long *) destHost->h_addr;
    destAddr.sin_family      = AF_INET;
    destAddr.sin_port        = 0;	/* set to actual port that remote
					 * ptrace is listening on.
					 */

    /* create a tcp client transport on which to issue
     * calls to the remote ptrace server
     */

    ptraceSock = RPC_ANYSOCK;
    pClient = clnttcp_create (&destAddr, RDBPROG, RDBVERS, &ptraceSock, 0, 0);
    
    if (pClient == NULL)
	{
        clnt_pcreateerror ("\tnet_connect");
	error ("Couldn't connect to remote target.");
	}
     }

/* Sleep for the specified number of milliseconds 
 * (assumed to be less than 1000).
 * If select () is interrupted, returns immediately;
 * takes an error exit if select () fails for some other reason.
 */

static void
sleep_ms (ms)
     long ms;
{
  struct timeval select_timeout;
  int status;

  select_timeout.tv_sec = 0;
  select_timeout.tv_usec = ms * 1000;

  status = select (0, (fd_set *) 0, (fd_set *) 0, (fd_set *) 0, &select_timeout);

  if (status < 0 && errno != EINTR)
    perror_with_name ("select");
}

/* Wait for control to return from inferior to debugger.
   If inferior gets a signal, we may decide to start it up again
   instead of returning.  That is why there is a loop in this function.
   When this function actually returns it means the inferior
   should be left stopped and GDB should read more commands.  */

/* For network debugging with VxWorks.
 * VxWorks knows when tasks hit breakpoints, receive signals, exit, etc,
 * so vx_wait() receives this information directly from
 * VxWorks instead of trying to figure out what happenned via a wait() call.
 */

static int
vx_wait (status)
     int *status;
{
  register int pid;
  WAITTYPE w;
  RDB_EVENT rdbEvent;

  do
    {
      /* If CTRL-C is hit during this loop,
	 suspend the inferior process.  */

      if (quit_flag)
	{
	  net_quit ();
	  quit_flag = 0;
	}

      if (net_wait (&rdbEvent) == -1)
	error ("Wait on remote target failed");
      
      pid = rdbEvent.taskId;
      if (pid == 0)
	{
	  sleep_ms (200);	/* FIXME Don't kill the network too badly */
	}
      else if (pid != inferior_pid)
	fatal ("SOME TERRIBLE TYPE OF ERROR MAYBE???  wrong pid 0x%x\n", pid);
    } while (pid == 0);

  /* FIXME, eventually do more then SIGTRAP on everything...  */
  switch (rdbEvent.eventType)
    {
    case EVENT_EXIT:
      WSETEXIT (w, 0);
      /* FIXME is it possible to distinguish between a
	 XXX   normal vs abnormal exit in VxWorks? */
      break;

    case EVENT_START:
      WSETSTOP (w, SIGTRAP);
      break;

    case EVENT_STOP:
      WSETSTOP (w, SIGTRAP);
      /* XXX was it stopped by a signal?  act accordingly */
      break;

    case EVENT_BREAK:
	/* Expecting a trace trap.  Stop the inferior and
	 * return silently when it happens.  */
      WSETSTOP (w, SIGTRAP);
      break;

    case EVENT_SUSPEND:
      target_terminal_ours_for_output ();
      printf ("\nRemote task suspended\n");  /* FIXME */
      fflush (stdout);
      WSETSTOP (w, SIGTRAP);
      break;

    case EVENT_SIGNAL:
      /* FIXME:  Which signal, Bullwinkle? */
      WSETSTOP (w, SIGTRAP);
      break;
    } /* switch */
  *status = *(int *)&w;		/* Grumble union wait crap Grumble */
  return pid;
}

/* Target command for VxWorks target systems.

   Used in vxgdb.  Takes the name of a remote target machine
   running vxWorks and connects to it to initialize remote network
   debugging.  */

static void
vx_open (args, from_tty)
     char *args;
     int from_tty;
{
  register int desc;
  struct cleanup *old_chain;
  extern int close ();
  char *bootFile;
  char *fullname = NULL;
  extern char *source_path;
  struct ldtabl loadTable;
  struct ldfile *pLoadFile;
  int i;
  extern CLIENT *pClient;

  if (!args)
    error_no_arg ("target machine name");

  printf ("Attaching remote machine across net...\n");
  fflush (stdout);

  net_connect (args);

  push_target (&vx_ops);

  /* Save a copy of the target host's name.  */
  if (vx_host)
    free (vx_host);
  vx_host = savestring (args, strlen (args));

  target_has_fp = net_check_for_fp ();

  /* Find out the name of the file from which the target was booted
     and load its symbol table.  */

  bootFile = NULL;
  if (!net_get_boot_file (&bootFile))
    {
      if (*bootFile)
        symbol_file_command (bootFile, 0);
      else if (from_tty)
	printf ("VxWorks symbols not loaded.\n");
    }
  else
    error ("Can't retrieve boot file name from target machine.");

  clnt_freeres (pClient, xdr_wrapstring, &bootFile);

  if (net_get_symbols (&loadTable) != 0)
    error ("Can't read loaded modules from target machine");

  for (i = 0; i < loadTable.tbl_size; i++)
    {
    pLoadFile = &loadTable.tbl_ent [i];
#ifdef WRS_ORIG
    desc = openp (source_path, 0, pLoadFile->name, O_RDONLY, 0, &fullname);
    if (desc < 0)
	perror_with_name (pLoadFile->name);
    old_chain = make_cleanup (close, desc);
    add_file_at_addr (fullname, desc, pLoadFile->txt_addr, pLoadFile->data_addr,
		      pLoadFile->bss_addr);
#else
    /* Botches, FIXME:
       (1)  Searches the PATH, not the source path.
       (2)  data and bss are assumed to be at the usual offsets from text.  */
    symbol_file_add (pLoadFile->name, from_tty, pLoadFile->txt_addr, 0);
#endif
    }
  clnt_freeres (pClient, xdr_ldtabl, &loadTable);
#ifdef WRS_ORIG
  do_cleanups (old_chain);
#endif
  if (from_tty)
    {
      printf ("Success!\n");
      fflush (stdout);
    }
}

/* Cross-net conversion of floats to and from extended form.
   (This is needed because different target machines have different
    extended floating point formats.)  */

/* Convert from an extended float to a double.

   The extended float is stored as raw data pointed to by FROM.
   Return the converted value as raw data in the double pointed to by TO.
*/

static void
vx_convert_to_virtual (regno, from, to)
    int regno;
    char *from;
    char *to;
{
  enum clnt_stat status;
  ext_fp from_ext_fp;
  double to_double;

  if (REGISTER_CONVERTIBLE (regno)) 
    {
      bcopy (from, (char *) &from_ext_fp, sizeof (from_ext_fp));
      bzero ((char *) &to_double, sizeof (to_double));

      status = net_clnt_call (VX_CONV_FROM_68881, xdr_ext_fp, &from_ext_fp,
			      xdr_double, &to_double);
      if (status == RPC_SUCCESS)
	  bcopy ((char *) &to_double, to, sizeof (to_double));
      else
	  error (rpcerr);
    }
  else
    bcopy (from, to, REGISTER_VIRTUAL_SIZE (regno));
}


/* The converse:  convert from a double to an extended float.

   The double is stored as raw data pointed to by FROM.
   Return the converted value as raw data in the extended
   float pointed to by TO.
*/

static void
vx_convert_from_virtual (regno, from, to)
    int regno;
    char *from;
    char *to;
{
  enum clnt_stat status;
  ext_fp to_ext_fp;
  double from_double;

  if (REGISTER_CONVERTIBLE (regno)) 
    {
      bcopy (from, (char *) &from_double, sizeof (from_double));
      bzero ((char *) &to_ext_fp, sizeof (to_ext_fp));

      status = net_clnt_call (VX_CONV_TO_68881, xdr_double, &from_double,
			      xdr_ext_fp, &to_ext_fp);
      if (status == RPC_SUCCESS)
	  bcopy ((char *) &to_ext_fp, to, sizeof (to_ext_fp));
      else
	  error (rpcerr);
    }
  else
    bcopy (from, to, REGISTER_VIRTUAL_SIZE (regno));
}


enum clnt_stat
net_clnt_call (procNum, inProc, in, outProc, out)
    enum ptracereq procNum;
    xdrproc_t inProc;
    char *in;
    xdrproc_t outProc;
    char *out;

{
    enum clnt_stat status;
    
    status = clnt_call (pClient, procNum, inProc, in, outProc, out, rpcTimeout);

    if (status != RPC_SUCCESS)
	clnt_perrno (status);

    return status;
}


/* Target ops structure for accessing memory and such over the net */

struct target_ops vx_ops = {
	"vxworks", "VxWorks target machine via RPC over TCP/IP",
	vx_open, 0, /* vx_detach, */
	0, 0, /* resume, wait */
	0, 0, /* read_reg, write_reg */
	vx_prepare_to_store, vx_convert_to_virtual, vx_convert_from_virtual,
	vx_xfer_memory, vx_files_info,
	0, 0, /* insert_breakpoint, remove_breakpoint */
	0, 0, 0, 0, 0,	/* terminal stuff */
	0, /* vx_kill, */
	vx_add_file_command,
	call_function_by_hand,  /* FIXME, calling fns is maybe botched? */
	0, /* lookup_symbol FIXME */
	vx_create_inferior, 0,  /* mourn_inferior */
	0, /* next */
	1, 1, 0, 0, 0,	/* all mem, mem, stack, regs, exec */
	OPS_MAGIC,		/* Always the last thing */
};

/* Target ops structure for accessing VxWorks child processes over the net */

struct target_ops vx_run_ops = {
	"vxprocess", "VxWorks process",
	vx_open, 0, /* vx_detach, */
	vx_resume, vx_wait,
	vx_read_register, vx_write_register,
	vx_prepare_to_store, vx_convert_to_virtual, vx_convert_from_virtual,
	vx_xfer_memory, vx_run_files_info,
	vx_insert_breakpoint, vx_remove_breakpoint,
	0, 0, 0, 0, 0,	/* terminal stuff */
	0, /* vx_kill, */
	vx_add_file_command,
	call_function_by_hand,  /* FIXME, calling fns is maybe botched? */
	0, /* lookup_symbol FIXME */
	vx_create_inferior, vx_mourn_inferior,
	0, /* next */
	1, 1, 1, 1, 1,	/* all mem, mem, stack, regs, exec */
	OPS_MAGIC,		/* Always the last thing */
};
/* Remember when reading at end of file, there are two "ops" structures here. */

void
_initialize_vx ()
{
  add_target (&vx_ops);
  add_target (&vx_run_ops);
}
