/* nodes.c -- How to get an Info file and node. */

#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include "nodes.h"
#include "search.h"
#include "filesys.h"
#include "info-utils.h"

#if !defined (O_RDONLY)
#  if defined (HAVE_SYS_FCNTL_H)
#    include <sys/fcntl.h>
#  else
#    include <fcntl.h>
#  endif /* !HAVE_SYS_FCNTL_H */
#endif /* !O_RDONLY */

#if !defined (errno)
extern int errno;
#endif /* !errno */

/* **************************************************************** */
/*								    */
/*		     Functions Static to this File		    */
/*								    */
/* **************************************************************** */

static void forget_info_file (), remember_info_file ();
static void free_file_buffer_tags (), free_file_buffer_nodes ();
static void free_info_tag (), free_info_node ();
static void get_nodes_of_tags_table (), get_nodes_of_info_file ();
static void get_tags_of_indirect_tags_table ();
static void info_reload_file_buffer_contents ();
static char *adjust_nodestart ();
static FILE_BUFFER *make_file_buffer ();
static FILE_BUFFER *info_load_file_internal (), *info_find_file_internal ();
static NODE *info_node_of_file_buffer_nodes ();
static NODE *info_node_of_file_buffer_tags ();

static long get_node_length ();

/* Magic number that RMS used to decide how much a tags table pointer could
   be off by.  I feel that it should be much smaller, like on the order of
   4. */
#define DEFAULT_INFO_FUDGE 1000

/* Passed to *_internal functions.  INFO_GET_TAGS says to do what is
   neccessary to fill in the nodes or tags arrays in FILE_BUFFER. */
#define INFO_NO_TAGS  0
#define INFO_GET_TAGS 1

/* **************************************************************** */
/*								    */
/*			 Global Variables			    */
/*								    */
/* **************************************************************** */

/* When non-zero, this is a string describing the recent file error. */
char *info_recent_file_error = (char *)NULL;

/* The list of already loaded nodes. */
FILE_BUFFER **info_loaded_files = (FILE_BUFFER **)NULL;

/* The number of slots currently allocated to LOADED_FILES. */
int info_loaded_files_slots = 0;

/* **************************************************************** */
/*								    */
/*		 Public Functions for Node Manipulation		    */
/*								    */
/* **************************************************************** */

/* Used to build "dir" menu from "localdir" files found in INFOPATH. */
extern void maybe_build_dir_node ();

/* Return a pointer to a NODE structure for the Info node (FILENAME)NODENAME.
   FILENAME can be passed as NULL, in which case the filename of "dir" is used.
   NODENAME can be passed as NULL, in which case the nodename of "Top" is used.
   If the node cannot be found, return a NULL pointer. */
NODE *
info_get_node (filename, nodename)
     char *filename, *nodename;
{
  FILE_BUFFER *file_buffer;

  file_buffer = (FILE_BUFFER *)NULL;
  info_recent_file_error = (char *)NULL;

  info_parse_node (nodename, DONT_SKIP_NEWLINES);
  nodename = (char *)NULL;

  if (info_parsed_filename)
    filename = info_parsed_filename;

  if (info_parsed_nodename)
    nodename = info_parsed_nodename;

  /* If FILENAME is not specified, it defaults to "dir". */
  if (!filename)
    filename = "dir";

  /* If the file to be looked up is "dir", build the contents from all of
     the "localdir"'s found in INFOPATH. */
  if (stricmp (filename, "dir") == 0)
    maybe_build_dir_node ("localdir");

  /* Find the correct info file. */
  file_buffer = info_find_file (filename);

  if (!file_buffer)
    {
      if (filesys_error_number)
	info_recent_file_error =
	  filesys_error_string (filename, filesys_error_number);
      return ((NODE *)NULL);
    }

  return (info_get_node_of_file_buffer (nodename, file_buffer));
}

/* Return a pointer to a NODE structure for the Info node NODENAME in
   FILE_BUFFER.  NODENAME can be passed as NULL, in which case the
   nodename of "Top" is used.  If the node cannot be found, return a
   NULL pointer. */
NODE *
info_get_node_of_file_buffer (nodename, file_buffer)
     char *nodename;
     FILE_BUFFER *file_buffer;
{
  NODE *node = (NODE *)NULL;

  /* If we are unable to find the file, we have to give up.  There isn't
     anything else we can do. */
  if (!file_buffer)
    return ((NODE *)NULL);

  /* If the file buffer was gc'ed, reload the contents now. */
  if (!file_buffer->contents)
    info_reload_file_buffer_contents (file_buffer);

  /* If NODENAME is not specified, it defaults to "Top". */
  if (!nodename)
    nodename = "Top";

  /* If the name of the node that we wish to find is exactly "*", then the
     node body is the contents of the entire file.  Create and return such
     a node. */
  if (strcmp (nodename, "*") == 0)
    {
      node = (NODE *)xmalloc (sizeof (NODE));
      node->filename = file_buffer->fullpath;
      node->parent   = (char *)NULL;
      node->nodename = savestring ("*");
      node->contents = file_buffer->contents;
      node->nodelen = file_buffer->filesize;
      node->flags = 0;
    }

  /* If this is the "main" info file, it might contain a tags table.  Search
     the tags table for an entry which matches the node that we want.  If
     there is a tags table, get the file which contains this node, but don't
     bother building a node list for it. */
  else if (file_buffer->tags)
    node = info_node_of_file_buffer_tags (file_buffer, nodename);

  /* If this file doesn't contain a tag table, then search the NODES
     list for this file to see if the node is present. */
  else if (file_buffer->nodes)
    node = info_node_of_file_buffer_nodes (file_buffer, nodename);

  if (node && (file_buffer->flags & N_IsCompressed))
    node->flags |= N_IsCompressed;

  /* Return the results of our node search. */
  return (node);
}

/* Locate the file named by FILENAME, and return the information structure
   describing this file.  The file may appear in our list of loaded files
   already, or it may not.  If it does not already appear, find the file,
   and add it to the list of loaded files.  If the file cannot be found,
   return a NULL FILE_BUFFER *. */
FILE_BUFFER *
info_find_file (filename)
     char *filename;
{
  return (info_find_file_internal (filename, INFO_GET_TAGS));
}

/* Load the info file FILENAME, remembering information about it in a
   file buffer. */
FILE_BUFFER *
info_load_file (filename)
     char *filename;
{
  return (info_load_file_internal (filename, INFO_GET_TAGS));
}


/* **************************************************************** */
/*								    */
/*		    Private Functions Implementation		    */
/*								    */
/* **************************************************************** */

/* The workhorse for info_find_file ().  Non-zero 2nd argument says to
   try to build a tags table (or otherwise glean the nodes) for this
   file once found.  By default, we build the tags table, but when this
   function is called by info_get_node () when we already have a valid
   tags table describing the nodes, it is unnecessary. */
static FILE_BUFFER *
info_find_file_internal (filename, get_tags)
     char *filename;
     int get_tags;
{
  register int i;
  register FILE_BUFFER *file_buffer;

  /* First try to find the file in our list of already loaded files. */
  if (info_loaded_files)
    {
      for (i = 0; file_buffer = info_loaded_files[i]; i++)
	if ((strcmp (filename, file_buffer->filename) == 0) ||
	    (strcmp (filename, file_buffer->fullpath) == 0) ||
	    ((*filename != '/') &&
	     strcmp (filename,
		     filename_non_directory (file_buffer->fullpath)) == 0))
	  {
	    struct stat new_info, *old_info;

	    /* This file is loaded.  If the filename that we want is
	       specifically "dir", then simply return the file buffer. */
	    if (stricmp (filename, "dir") == 0)
	      return (file_buffer);

	    /* The file appears to be already loaded, and it is not "dir".
	       Check to see if it has changed since the last time it was
	       loaded. */
	    if (stat (file_buffer->fullpath, &new_info) == -1)
	      {
		filesys_error_number = errno;
		return ((FILE_BUFFER *)NULL);
	      }

	    old_info = &file_buffer->finfo;

	    if ((new_info.st_size != old_info->st_size) ||
		(new_info.st_mtime != old_info->st_mtime))
	      {
		/* The file has changed.  Forget that we ever had loaded it
		   in the first place. */
		forget_info_file (filename);
		break;
	      }
	    else
	      {
		/* The info file exists, and has not changed since the last
		   time it was loaded.  If the caller requested a nodes list
		   for this file, and there isn't one here, build the nodes
		   for this file_buffer.  In any case, return the file_buffer
		   object. */
		if (get_tags && (!file_buffer->tags && !file_buffer->nodes))
		  build_tags_and_nodes (file_buffer);

		return (file_buffer);
	      }
	  }
    }

  /* The file wasn't loaded.  Try to load it now. */
  file_buffer = info_load_file_internal (filename, get_tags);

  /* If the file was loaded, remember the name under which it was found. */
  if (file_buffer)
    remember_info_file (file_buffer);

  return (file_buffer);
}

/* The workhorse function for info_load_file ().  Non-zero second argument
   says to build a list of tags (or nodes) for this file.  This is the
   default behaviour when info_load_file () is called, but it is not
   necessary when loading a subfile for which we already have tags. */
static FILE_BUFFER *
info_load_file_internal (filename, get_tags)
     char *filename;
     int get_tags;
{
  char *fullpath, *contents;
  long filesize;
  struct stat finfo;
  int retcode;
  FILE_BUFFER *file_buffer = (FILE_BUFFER *)NULL;

  /* Get the full pathname of this file, as known by the info system.
     That is to say, search along INFOPATH and expand tildes, etc. */
  fullpath = info_find_fullpath (filename);

  /* Did we actually find the file? */
  retcode = stat (fullpath, &finfo);

  /* If the file referenced by the name returned from info_find_fullpath ()
     doesn't exist, then try again with the last part of the filename
     appearing in lowercase. */
  if (retcode < 0)
    {
      char *lowered_name;
      char *basename;

      lowered_name = savestring (filename);
      basename = (char *)rindex (lowered_name, '/');

      if (basename)
	basename++;
      else
	basename = lowered_name;

      while (*basename)
	{
	  if (isupper (*basename))
	    *basename = tolower (*basename);

	  basename++;
	}

      fullpath = info_find_fullpath (lowered_name);
      free (lowered_name);

      retcode = stat (fullpath, &finfo);
    }

  /* If the file wasn't found, give up, returning a NULL pointer. */
  if (retcode < 0)
    {
      filesys_error_number = errno;
      return ((FILE_BUFFER *)NULL);
    }

  /* Otherwise, try to load the file. */
  contents = filesys_read_info_file (fullpath, &filesize, &finfo);

  if (!contents)
    return ((FILE_BUFFER *)NULL);

  /* The file was found, and can be read.  Allocate FILE_BUFFER and fill
     in the various members. */
  file_buffer = make_file_buffer ();
  file_buffer->filename = savestring (filename);
  file_buffer->fullpath = savestring (fullpath);
  file_buffer->finfo = finfo;
  file_buffer->filesize = filesize;
  file_buffer->contents = contents;
  if (file_buffer->filesize != file_buffer->finfo.st_size)
    file_buffer->flags |= N_IsCompressed;

  /* If requested, build the tags and nodes for this file buffer. */
  if (get_tags)
    build_tags_and_nodes (file_buffer);

  return (file_buffer);
}

/* Grovel FILE_BUFFER->contents finding tags and nodes, and filling in the
   various slots.  This can also be used to rebuild a tag or node table. */
void
build_tags_and_nodes (file_buffer)
     FILE_BUFFER *file_buffer;
{
  SEARCH_BINDING binding;
  long position;

  free_file_buffer_tags (file_buffer);
  free_file_buffer_nodes (file_buffer);
  file_buffer->flags &= ~N_HasTagsTable;

  /* See if there is a tags table in this info file. */
  binding.buffer = file_buffer->contents;
  binding.start = file_buffer->filesize;
  binding.end = binding.start - 1000;
  if (binding.end < 0)
    binding.end = 0;
  binding.flags = S_FoldCase;

  position = search_backward (TAGS_TABLE_END_LABEL, &binding);

  /* If there is a tag table, find the start of it, and grovel
     over it extracting tag information. */
  if (position != -1)
    while (1)
      {
	long tags_table_begin, tags_table_end;

	binding.end = position;
	binding.start = binding.end - 5 - strlen (TAGS_TABLE_END_LABEL);
	if (binding.start < 0)
	  binding.start = 0;

	position = find_node_separator (&binding);

	/* For this test, (and all others here) failure indicates a bogus
	   tags table.  Grovel the file. */
	if (position == -1)
	  break;

	/* Remember the end of the tags table. */
	binding.start = position;
	tags_table_end = binding.start;
	binding.end = 0;

	/* Locate the start of the tags table. */
	position = search_backward (TAGS_TABLE_BEG_LABEL, &binding);

	if (position == -1)
	  break;

	binding.end = position;
	binding.start = binding.end - 5 - strlen (TAGS_TABLE_BEG_LABEL);
	position = find_node_separator (&binding);

	if (position == -1)
	  break;

	/* The file contains a valid tags table.  If it is an indirect table,
	   then fill FILE_BUFFER's tag member.  If it is not indirect, the
	   table simply points to nodes in this file, so fill the node's
	   member. */
	file_buffer->flags |= N_HasTagsTable;
	tags_table_begin = position;

	/* If this isn't an indirect tags table, just remember the nodes
	   described locally in this tags table.  Note that binding.end
	   is pointing to just after the beginning label. */
	binding.start = binding.end;
	binding.end = file_buffer->filesize;

	if (!looking_at (TAGS_TABLE_IS_INDIRECT_LABEL, &binding))
	  {
	    binding.start = tags_table_begin;
	    binding.end = tags_table_end;
	    get_nodes_of_tags_table (file_buffer, &binding);
	    return;
	  }
	else
	  {
	    /* This is an indirect tags table.  Build TAGS member. */
	    SEARCH_BINDING indirect;

	    indirect.start = tags_table_begin;
	    indirect.end = 0;
	    indirect.buffer = binding.buffer;
	    indirect.flags = S_FoldCase;

	    position = search_backward (INDIRECT_TAGS_TABLE_LABEL, &indirect);

	    if (position == -1)
	      {
		/* This file is malformed.  Give up. */
		return;
	      }

	    indirect.start = position;
	    indirect.end = tags_table_begin;
	    binding.start = tags_table_begin;
	    binding.end = tags_table_end;
	    get_tags_of_indirect_tags_table (file_buffer, &indirect, &binding);
	    return;
	  }
      }

  /* This file doesn't contain any kind of tags table.  Grovel the
     file and build node entries for it. */
  get_nodes_of_info_file (file_buffer);
}

/* Search through FILE_BUFFER->contents building an array of NODE_ENTRY *,
   one entry per each node present in the file.  Store the nodes in
   FILE_BUFFER->nodes, and the number of allocated slots in
   FILE_BUFFER->nodes_slots. */
static void
get_nodes_of_info_file (file_buffer)
     FILE_BUFFER *file_buffer;
{
  long nodestart;
  int nodes_index = 0;
  SEARCH_BINDING binding;

  binding.buffer = file_buffer->contents;
  binding.start = 0;
  binding.end = file_buffer->filesize;
  binding.flags = S_FoldCase;

  while ((nodestart = find_node_separator (&binding)) != -1)
    {
      int start, end;
      char *nodeline;
      NODE_ENTRY *entry;

      /* Skip past the characters just found. */
      binding.start = nodestart;
      binding.start += skip_node_separator (binding.buffer + binding.start);

      /* Move to the start of the line defining the node. */
      nodeline = binding.buffer + binding.start;

      /* Find "Node:" */
      start = string_in_line (INFO_NODE_LABEL, nodeline);

      /* If not there, this is not the start of a node. */
      if (start == -1)
	continue;

      /* Find the start of the nodename. */
      start += skip_whitespace (nodeline + start);

      /* Find the end of the nodename. */
      end = start +
	skip_node_characters (nodeline + start, DONT_SKIP_NEWLINES);

      /* Okay, we have isolated the node name, and we know where the
	 node starts.  Remember this information in a NODE structure. */
      entry = (NODE_ENTRY *)xmalloc (sizeof (NODE_ENTRY));
      entry->nodename = (char *)xmalloc (1 + (end - start));
      strncpy (entry->nodename, nodeline + start, end - start);
      entry->nodename[end - start] = '\0';
      entry->nodestart = nodestart;
      {
	SEARCH_BINDING node_body;

	node_body.buffer = binding.buffer + binding.start;
	node_body.start = 0;
	node_body.end = binding.end - binding.start;
	node_body.flags = S_FoldCase;
	entry->nodelen = get_node_length (&node_body);
      }

      /* Add this node structure to the array of node structures in this
	 FILE_BUFFER. */
      add_pointer_to_array (entry, nodes_index, file_buffer->nodes,
			    file_buffer->nodes_slots, 100, NODE_ENTRY *);
    }
}

/* Return the length of the node which starts at BINDING. */
static long
get_node_length (binding)
     SEARCH_BINDING *binding;
{
  register int i;
  char *body;

  /* From the Info-RFC file:
     [A node] ends with either a ^_, a ^L, or the end of file. */
  for (i = binding->start, body = binding->buffer; i < binding->end; i++)
    {
      if (body[i] == INFO_FF || body[i] == INFO_COOKIE)
	break;
    }
  return ((long) i - binding->start);
}

/* Build and save the array of nodes in FILE_BUFFER by searching through the
   contents of BUFFER_BINDING for a tags table, and groveling the contents. */
static void
get_nodes_of_tags_table (file_buffer, buffer_binding)
     FILE_BUFFER *file_buffer;
     SEARCH_BINDING *buffer_binding;
{
  int offset, nodes_index = 0;
  SEARCH_BINDING *search;
  long position;

  search = copy_binding (buffer_binding);

  /* Find the start of the tags table. */
  position = find_tags_table (search);

  /* If none, we're all done. */
  if (position == -1)
    return;

  /* Move to one character before the start of the actual table. */
  search->start = position;
  search->start += skip_node_separator (search->buffer + search->start);
  search->start += strlen (TAGS_TABLE_BEG_LABEL);
  search->start--;

  /* The tag table consists of lines containing node names and positions.
     Do each line until we find one that doesn't contain a node name. */
  while ((position = search_forward ("\n", search)) != -1)
    {
      NODE_ENTRY *entry;
      char *nodedef;

      /* Prepare to skip this line. */
      search->start = position;
      search->start++;

      /* Skip past informative "(Indirect)" tags table line. */
      if (!nodes_index && looking_at (TAGS_TABLE_IS_INDIRECT_LABEL, search))
	continue;

      /* Find the label preceding the node name. */
      offset =
	string_in_line (INFO_NODE_LABEL, search->buffer + search->start);

      /* If not there, not a defining line, so we must be out of the
	 tags table. */
      if (offset == -1)
	break;

      /* Point to the beginning of the node definition. */
      search->start += offset;
      nodedef = search->buffer + search->start;
      nodedef += skip_whitespace (nodedef);

      /* Move past the node's name. */
      for (offset = 0;
	   (nodedef[offset]) && (nodedef[offset] != INFO_TAGSEP);
	   offset++);

      if (nodedef[offset] != INFO_TAGSEP)
	continue;

      entry = (NODE_ENTRY *)xmalloc (sizeof (NODE_ENTRY));
      entry->nodename = (char *)xmalloc (1 + offset);
      strncpy (entry->nodename, nodedef, offset);
      entry->nodename[offset] = '\0';
      offset++;
      entry->nodestart = (long) atol (nodedef + offset);

      /* We don't know the length of this node yet. */
      entry->nodelen = -1;

      /* Add this node structure to the array of node structures in this
	 FILE_BUFFER. */
      add_pointer_to_array (entry, nodes_index, file_buffer->nodes,
			    file_buffer->nodes_slots, 100, NODE_ENTRY *);
    }
  free (search);
}

typedef struct {
  char *filename;
  long first_byte;
} SUBFILE;

/* Remember in FILE_BUFFER the nodenames, subfilenames, and offsets within the
   subfiles of every node which appears in TAGS_BINDING.  The 2nd argument is
   a binding surrounding the indirect files list. */
static void
get_tags_of_indirect_tags_table (file_buffer, indirect_binding, tags_binding)
     FILE_BUFFER *file_buffer;
     SEARCH_BINDING *indirect_binding, *tags_binding;
{
  register int i;
  FILE_BUFFER *temp_file_buffer;
  SUBFILE **subfiles = (SUBFILE **)NULL;
  int subfiles_index = 0, subfiles_slots = 0;
  NODE_ENTRY *entry;

  /* First get the list of tags from the tags table.  Then create a
     NODE_ENTRY for each entry in the tags table, looking up the
     associated file in the indirect list. */

  temp_file_buffer = make_file_buffer ();
  get_nodes_of_tags_table (temp_file_buffer, tags_binding);

  /* We have the list of tags in temp_file_buffer->nodes.  Get the
     list of subfiles from the indirect table. */
  {
    char *start, *end, *line;
    SUBFILE *subfile;

    start = indirect_binding->buffer + indirect_binding->start;
    end = indirect_binding->buffer + indirect_binding->end;
    line = start;

    while (line < end)
      {
	int colon;

	colon = string_in_line (":", line);

	if (colon == -1)
	  break;

	subfile = (SUBFILE *)xmalloc (sizeof (SUBFILE));
	subfile->filename = (char *)xmalloc (colon);
	strncpy (subfile->filename, line, colon - 1);
	subfile->filename[colon - 1] = '\0';
	subfile->first_byte = (long) atol (line + colon);

	add_pointer_to_array
	  (subfile, subfiles_index, subfiles, subfiles_slots, 10, SUBFILE *);

	while (*line++ != '\n');
      }
  }

  /* If we have successfully built the indirect files table, then
     combine the information in the two tables, remembering it in
     file_buffer->tags.  Otherwise, give up. */
  if (!subfiles)
    {
      free_file_buffer_nodes (temp_file_buffer);
      free (temp_file_buffer);
      return;
    }
  else
    {
      register int nodes_index;
      TAG **tags = (TAG **)NULL;
      long header_length, tags_index = 0, tags_slots = 0;
      SEARCH_BINDING binding;

      /* Find the length of the header of the file containing the indirect
	 tags table.  This header appears at the start of every file.  We
	 want the absolute position of each node within each subfile, so
	 we subtract the start of the containing subfile from the logical
	 position of the node, and then add the length of the header in. */
      binding.buffer = file_buffer->contents;
      binding.start = 0;
      binding.end = file_buffer->filesize;
      binding.flags = S_FoldCase;

      header_length = find_node_separator (&binding);
      if (header_length == -1)
	header_length = 0;

      /* Build the file buffer's list of subfiles. */
      {
	char *containing_dir, *temp;
	int len_containing_dir;

	containing_dir = savestring (file_buffer->fullpath);
	temp = (char *)rindex (containing_dir, '/');

	if (temp)
	  *temp = '\0';

	len_containing_dir = strlen (containing_dir);

	for (i = 0; subfiles[i]; i++);

	file_buffer->subfiles = (char **) xmalloc ((1 + i) * sizeof (char *));

	for (i = 0; subfiles[i]; i++)
	  {
	    char *fullpath;

	    fullpath = (char *) xmalloc
	      (2 + strlen (subfiles[i]->filename) + len_containing_dir);

	    sprintf (fullpath, "%s/%s",
		     containing_dir, subfiles[i]->filename);

	    file_buffer->subfiles[i] = fullpath;
	  }
	file_buffer->subfiles[i] = (char *)NULL;
	free (containing_dir);
      }

      /* For each node in the indirect files tags table, remember the
	 node name, associated subfile, and starting position in the
	 file_buffer->tags member. */
      for (nodes_index = 0;
	   entry = temp_file_buffer->nodes[nodes_index];
	   nodes_index++)
	{
	  TAG *tag;

	  tag = (TAG *)xmalloc (sizeof (TAG));
	  tag->nodename = entry->nodename;

	  for (i = 0;
	       subfiles[i] && entry->nodestart >= subfiles[i]->first_byte;
	       i++);

	  /* If the Info file containing the indirect tags table is
	     malformed, then give up. */
	  if (!i)
	    {
	      /* The Info file containing the indirect tags table is
		 malformed.  Give up. */
	      for (i = 0; subfiles[i]; i++)
		{
		  free (subfiles[i]->filename);
		  free (subfiles[i]);
		  free (file_buffer->subfiles[i]);
		}
	      file_buffer->subfiles = (char **)NULL;
	      free_file_buffer_nodes (temp_file_buffer);
	      free (temp_file_buffer);

	      if (tags)
		{
		  for (i = 0; tags[i]; i++)
		    free (tags[i]);

		  free (tags);
		}

	      return;
	    }

	  /* SUBFILES[i] is the index of the first subfile whose logical
	     first byte is greater than the logical offset of this node's
	     starting position.  This means that the subfile directly
	     preceding this one is the one containing the node. */

	  tag->filename = file_buffer->subfiles[i - 1];
	  tag->nodestart = entry->nodestart - subfiles[i -1]->first_byte;
	  tag->nodestart += header_length;
	  tag->nodelen = -1;

	  add_pointer_to_array (tag, tags_index, tags, tags_slots, 100, TAG *);
	}

      /* We have successfully built the tags table.  Remember it in the
	 file_buffer->tags member. */
      file_buffer->tags = tags;
      file_buffer->tags_slots = tags_slots;
      file_buffer->flags |= N_TagsIndirect;
    }

  /* Free the structures assigned to temp_file_buffer->nodes.  Do not
     free the nodename strings found in each entry, since they are now
     in use in file_buffer->tags. */
  for (i = 0; entry = temp_file_buffer->nodes[i]; i++)
    free (entry);

  free (temp_file_buffer);

  /* Free the structures assigned to SUBFILES.  Free the names as well
     as the structures themselves, then finally, the array. */
  for (i = 0; subfiles[i]; i++)
    {
      free (subfiles[i]->filename);
      free (subfiles[i]);
    }
  free (subfiles);
}

/* Return the node from FILE_BUFFER which matches NODENAME by searching
   the tags table in FILE_BUFFER.  If the node could not be found, return
   a NULL pointer. */
static NODE *
info_node_of_file_buffer_tags (file_buffer, nodename)
     FILE_BUFFER *file_buffer;
     char *nodename;
{
  register int i;
  TAG *tag;

  for (i = 0; tag = file_buffer->tags[i]; i++)
    if (strcmp (nodename, tag->nodename) == 0)
      {
	FILE_BUFFER *subfile;

	subfile = info_find_file_internal (tag->filename, INFO_NO_TAGS);

	if (!subfile)
	  return ((NODE *)NULL);

	if (!subfile->contents)
	  info_reload_file_buffer_contents (subfile);

	/* If we were able to find this file and load it, then return
	   the node within it. */
	{
	  NODE *node;

	  node = (NODE *)xmalloc (sizeof (NODE));
	  node->filename = (subfile->fullpath);
	  node->parent   = file_buffer->fullpath;
	  node->nodename = tag->nodename;
	  node->contents = subfile->contents + tag->nodestart;
	  node->flags = N_HasTagsTable | N_TagsIndirect;
	  if (subfile->flags & N_IsCompressed)
	    node->flags |= N_IsCompressed;

	  /* If TAG->nodelen hasn't been calculated yet, then we aren't
	     in a position to trust the entry pointer.  Adjust things so
	     that ENTRY->nodestart gets the exact address of the start of
	     the node separator which starts this node, and NODE->contents
	     gets the address of the line defining this node.  If we cannot
	     do that, the node isn't really here. */
	  if (tag->nodelen == -1)
	    {
	      int min, max;
	      char *node_sep;
	      SEARCH_BINDING node_body;
	      char *buff_end;

	      min = max = DEFAULT_INFO_FUDGE;

	      if (tag->nodestart < DEFAULT_INFO_FUDGE)
		min = tag->nodestart;

	      if (DEFAULT_INFO_FUDGE >
		  (subfile->filesize - tag->nodestart))
		max = subfile->filesize - tag->nodestart;

	      /* NODE_SEP gets the address of the separator which defines
		 this node, or (char *)NULL if the node wasn't found.
		 NODE->contents is side-effected to point to right after
		 the separator. */
	      node_sep = adjust_nodestart (node, min, max);
	      if (node_sep == (char *)NULL)
		{
		  free (node);
		  return ((NODE *)NULL);
		}
	      /* Readjust tag->nodestart. */
	      tag->nodestart = node_sep - subfile->contents;

	      /* Calculate the length of the current node. */
	      buff_end = subfile->contents + subfile->filesize;

	      node_body.buffer = node->contents;
	      node_body.start = 0;
	      node_body.end = buff_end - node_body.buffer;
	      node_body.flags = 0;
	      tag->nodelen = get_node_length (&node_body);
	    }
	  else
	    {
	      /* Since we know the length of this node, we have already
		 adjusted tag->nodestart to point to the exact start of
		 it.  Simply skip the node separator. */
	      node->contents += skip_node_separator (node->contents);
	    }

	  node->nodelen = tag->nodelen;
	  return (node);
	}
      }

  /* There was a tag table for this file, and the node wasn't found.
     Return NULL, since this file doesn't contain the desired node. */
  return ((NODE *)NULL);
}

/* Return the node from FILE_BUFFER which matches NODENAME by searching
   the nodes list in FILE_BUFFER.  If the node could not be found, return
   a NULL pointer. */
static NODE *
info_node_of_file_buffer_nodes (file_buffer, nodename)
     FILE_BUFFER *file_buffer;
     char *nodename;
{
  register int i;
  NODE_ENTRY *entry;

  if (!file_buffer->nodes)
    return ((NODE *)NULL);

  for (i = 0; entry = file_buffer->nodes[i]; i++)
    if (strcmp (nodename, entry->nodename) == 0)
      {
	NODE *node;

	node = (NODE *)xmalloc (sizeof (NODE));
	node->filename = file_buffer->fullpath;
	node->parent   = (char *)NULL;
	node->nodename = entry->nodename;
	node->contents = file_buffer->contents + entry->nodestart;
	node->flags = 0;

	if (file_buffer->flags & N_HasTagsTable)
	  node->flags |= N_HasTagsTable;

	/* If entry->nodelen hasn't been calculated yet, then we aren't
	   in a position to trust the entry pointer.  Adjust things so
	   that ENTRY->nodestart gets the exact address of the start of
	   the node separator which starts this node, while NODE->contents
	   gets the address of the line defining this node.  If we cannot
	   do that, the node isn't really here. */
	if (entry->nodelen == -1)
	  {
	    int min, max;
	    char *node_sep;
	    SEARCH_BINDING node_body;
	    char *buff_end;

	    min = max = DEFAULT_INFO_FUDGE;

	    if (entry->nodestart < DEFAULT_INFO_FUDGE)
	      min = entry->nodestart;

	    if (DEFAULT_INFO_FUDGE >
		(file_buffer->filesize - entry->nodestart))
	      max = file_buffer->filesize - entry->nodestart;

	    /* NODE_SEP gets the address of the separator which defines this
	       node, or (char *)NULL if the node wasn't found.  NODE->contents
	       is side-effected to point to right after the separator. */
	    node_sep = adjust_nodestart (node, min, max);
	    if (node_sep == (char *)NULL)
	      {
		free (node);
		return ((NODE *)NULL);
	      }

	    /* Readjust entry->nodestart. */
	    entry->nodestart = node_sep - file_buffer->contents;

	    /* Calculate the length of the current node. */
	    buff_end = file_buffer->contents + file_buffer->filesize;

	    node_body.buffer = node->contents;
	    node_body.start = 0;
	    node_body.end = buff_end - node_body.buffer;
	    node_body.flags = 0;
	    entry->nodelen = get_node_length (&node_body);
	  }
	else
	  {
	    /* Since we know the length of this node, we have already
	       adjusted entry->nodestart to point to the exact start of
	       it.  Simply skip the node separator. */
	    node->contents += skip_node_separator (node->contents);
	  }
	node->nodelen  = entry->nodelen;
	return (node);
      }

  /* There was a nodes list for this file, and the node wasn't found.
     Return NULL, since this file doesn't contain the desired node. */
  return ((NODE *)NULL);
}

/* **************************************************************** */
/*								    */
/*		Managing file_buffers, nodes, and tags.		    */
/*								    */
/* **************************************************************** */

static FILE_BUFFER *
make_file_buffer ()
{
  FILE_BUFFER *file_buffer;

  file_buffer = (FILE_BUFFER *)xmalloc (sizeof (FILE_BUFFER));
  file_buffer->filename = file_buffer->fullpath = (char *)NULL;
  file_buffer->contents = (char *)NULL;
  file_buffer->tags = (TAG **)NULL;
  file_buffer->nodes = (NODE_ENTRY **)NULL;
  file_buffer->subfiles = (char **)NULL;
  file_buffer->tags_slots = 0;
  file_buffer->nodes_slots = 0;
  file_buffer->flags = 0;

  return (file_buffer);
}

/* Add FILE_BUFFER to our list of already loaded info files. */
static void
remember_info_file (file_buffer)
     FILE_BUFFER *file_buffer;
{
  int i;

  for (i = 0; info_loaded_files && info_loaded_files[i]; i++)
    ;

  add_pointer_to_array (file_buffer, i, info_loaded_files,
			info_loaded_files_slots, 10, FILE_BUFFER *);
}

/* Forget the contents, tags table, nodes list, and names of FILENAME. */
static void
forget_info_file (filename)
     char *filename;
{
  register int i;
  FILE_BUFFER *file_buffer;

  if (!info_loaded_files)
    return;

  for (i = 0; file_buffer = info_loaded_files[i]; i++)
    if ((strcmp (filename, file_buffer->filename) == 0) ||
	(strcmp (filename, file_buffer->fullpath) == 0))
      {
	free (file_buffer->filename);
	free (file_buffer->fullpath);

	if (file_buffer->contents)
	  free (file_buffer->contents);
	
	/* Note that free_file_buffer_tags () also kills the subfiles
	   list, since the subfiles list is only of use in conjunction
	   with tags. */
	free_file_buffer_tags (file_buffer);
	free_file_buffer_nodes (file_buffer);


	while (info_loaded_files[i] = info_loaded_files[++i])
	  ;

	break;
      }
}

/* Free the tags (if any) associated with FILE_BUFFER. */
static void
free_file_buffer_tags (file_buffer)
     FILE_BUFFER *file_buffer;
{
  register int i;

  if (file_buffer->tags)
    {
      register TAG *tag;

      for (i = 0; tag = file_buffer->tags[i]; i++)
	free_info_tag (tag);

      free (file_buffer->tags);
      file_buffer->tags = (TAG **)NULL;
      file_buffer->tags_slots = 0;
    }

  if (file_buffer->subfiles)
    {
      for (i = 0; file_buffer->subfiles[i]; i++)
	free (file_buffer->subfiles[i]);

      free (file_buffer->subfiles);
      file_buffer->subfiles = (char **)NULL;
    }
}

/* Free the nodes (if any) associated with FILE_BUFFER. */
static void
free_file_buffer_nodes (file_buffer)
     FILE_BUFFER *file_buffer;
{
  if (file_buffer->nodes)
    {
      register int i;
      register NODE_ENTRY *entry;

      for (i = 0; entry = file_buffer->nodes[i]; i++)
	free_info_node (entry);

      free (file_buffer->nodes);
      file_buffer->nodes = (NODE_ENTRY **)NULL;
      file_buffer->nodes_slots = 0;
    }
}

/* Free the data associated with TAG, as well as TAG itself. */
static void
free_info_tag (tag)
     TAG *tag;
{
  free (tag->nodename);

  /* We don't free tag->filename, because that filename is part of the
     subfiles list for the containing FILE_BUFFER.  free_info_tags ()
     will free the subfiles when it is appropriate. */

  free (tag);
}

/* Free the data associated with ENTRY, as well as ENTRY itself. */
static void
free_info_node (entry)
     NODE_ENTRY *entry;
{
  free (entry->nodename);
  free (entry);
}

/* Load the contents of FILE_BUFFER->contents.  This function is called
   when a file buffer was loaded, and then in order to conserve memory, the
   file buffer's contents were freed and the pointer was zero'ed.  Note that
   the file was already loaded at least once successfully, so the tags and/or
   nodes members are still correctly filled. */
static void
info_reload_file_buffer_contents (fb)
     FILE_BUFFER *fb;
{
  fb->flags &= ~N_IsCompressed;

  /* Let the filesystem do all the work for us. */
  fb->contents =
    filesys_read_info_file (fb->fullpath, &(fb->filesize), &(fb->finfo));
  if (fb->filesize != fb->finfo.st_size)
    fb->flags |= N_IsCompressed;
}

/* Return the actual starting memory location of NODE, side-effecting
   NODE->contents.  MIN and MAX are bounds for a search if one is necessary.
   Because of the way that tags are implemented, the physical nodestart may
   not actually be where the tag says it is.  If that is the case, but the
   node was found anyway, set N_UpdateTags in NODE->flags.  If the node is
   found, return non-zero.  NODE->contents is returned positioned right after
   the node separator that precedes this node, while the return value is
   position directly on the separator that precedes this node.  If the node
   could not be found, return a NULL pointer. */
static char *
adjust_nodestart (node, min, max)
     NODE *node;
     int min, max;
{
  long position;
  SEARCH_BINDING node_body;

  /* Define the node body. */
  node_body.buffer = node->contents;
  node_body.start = 0;
  node_body.end = max;
  node_body.flags = 0;

  /* Try the optimal case first.  Who knows?  This file may actually be
     formatted (mostly) correctly. */
  if (node_body.buffer[0] != INFO_COOKIE && min > 2)
    node_body.buffer -= 3;

  position = find_node_separator (&node_body);

  /* If we found a node start, then check it out. */
  if (position != -1)
    {
      int sep_len;

      sep_len = skip_node_separator (node->contents);

      /* If we managed to skip a node separator, then check for this node
	 being the right one. */
      if (sep_len != 0)
	{
	  char *nodedef, *nodestart;
	  int offset;

	  nodestart = node_body.buffer + position + sep_len;
	  nodedef = nodestart;
	  offset = string_in_line (INFO_NODE_LABEL, nodedef);

	  if (offset != -1)
	    {
	      nodedef += offset;
	      nodedef += skip_whitespace (nodedef);
	      offset = skip_node_characters (nodedef, DONT_SKIP_NEWLINES);
	      if ((offset == strlen (node->nodename)) &&
		  (strncmp (node->nodename, nodedef, offset) == 0))
		{
		  node->contents = nodestart;
		  return (node_body.buffer + position);
		}
	    }
	}
    }

  /* Oh well, I guess we have to try to find it in a larger area. */
  node_body.buffer = node->contents - min;
  node_body.start = 0;
  node_body.end = min + max;
  node_body.flags = 0;

  position = find_node_in_binding (node->nodename, &node_body);

  /* If the node couldn't be found, we lose big. */
  if (position == -1)
    return ((char *)NULL);

  /* Otherwise, the node was found, but the tags table could need updating
     (if we used a tag to get here, that is).  Set the flag in NODE->flags. */
  node->contents = node_body.buffer + position;
  node->contents += skip_node_separator (node->contents);
  if (node->flags & N_HasTagsTable)
    node->flags |= N_UpdateTags;
  return (node_body.buffer + position);
}
