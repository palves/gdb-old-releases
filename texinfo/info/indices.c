/* indices.c -- Commands for dealing with an Info file Index. */

#include "info.h"
#include "indices.h"

/* User-visible variable controls the output of info-index-next. */
int show_index_match = 1;

/* In the Info sense, an index is a menu.  This variable holds the last
   parsed index. */
static REFERENCE **index_index = (REFERENCE **)NULL;

/* The offset of the most recently selected index element. */
static int index_offset = 0;

/* Variable which holds the last string searched for. */
static char *index_search = (char *)NULL;

/* A couple of "globals" describing where the initial index was found. */
static char *initial_index_filename = (char *)NULL;
static char *initial_index_nodename = (char *)NULL;

/* A structure associating index names with index offset ranges. */
typedef struct {
  char *name;			/* The nodename of this index. */
  int first;			/* The index in our list of the first entry. */
  int last;			/* The index in our list of the last entry. */
} INDEX_NAME_ASSOC;

/* An array associating index nodenames with index offset ranges. */
static INDEX_NAME_ASSOC **index_nodenames = (INDEX_NAME_ASSOC **)NULL;
static int index_nodenames_index = 0;
static int index_nodenames_slots = 0;

/* Add the name of NODE, and the range of the associated index elements
   (passed in ARRAY) to index_nodenames. */
static void
add_index_to_index_nodenames (array, node)
     REFERENCE **array;
     NODE *node;
{
  register int i, last;
  INDEX_NAME_ASSOC *assoc;

  for (last = 0; array[last]; last++);
  assoc = (INDEX_NAME_ASSOC *)xmalloc (sizeof (INDEX_NAME_ASSOC));
  assoc->name = savestring (node->nodename);

  if (!index_nodenames_index)
    {
      assoc->first = 0;
      assoc->last = last;
    }
  else
    {
      for (i = 0; index_nodenames[i + 1]; i++);
      assoc->first = 1 + index_nodenames[i]->last;
      assoc->last = assoc->first + last;
    }
  add_pointer_to_array
    (assoc, index_nodenames_index, index_nodenames, index_nodenames_slots,
     10, INDEX_NAME_ASSOC *);
}

/* Find and return the indices of WINDOW's file.  The indices are defined
   as the first node in the file containing the word "Index" and any
   immediately following nodes whose names also contain "Index".  All such
   indices are concatenated and the result returned.  If WINDOW's info file
   doesn't have any indices, a NULL pointer is returned. */
REFERENCE **
info_indices_of_window (window)
     WINDOW *window;
{
  FILE_BUFFER *fb;

  fb = file_buffer_of_window (window);

  return (info_indices_of_file_buffer (fb));
}

REFERENCE **
info_indices_of_file_buffer (file_buffer)
     FILE_BUFFER *file_buffer;
{
  register int i;
  REFERENCE **result = (REFERENCE **)NULL;

  /* No file buffer, no indices. */
  if (!file_buffer)
    return ((REFERENCE **)NULL);

  /* Reset globals describing where the index was found. */
  maybe_free (initial_index_filename);
  maybe_free (initial_index_nodename);
  initial_index_filename = (char *)NULL;
  initial_index_nodename = (char *)NULL;

  if (index_nodenames)
    {
      for (i = 0; index_nodenames[i]; i++)
	{
	  free (index_nodenames[i]->name);
	  free (index_nodenames[i]);
	}

      index_nodenames_index = 0;
      index_nodenames[0] = (INDEX_NAME_ASSOC *)NULL;
    }

  /* If we have an indirect tags table, grovel it. */
  if (file_buffer->tags)
    {
      TAG *tag;

      for (i = 0; tag = file_buffer->tags[i]; i++)
	{
	  if (string_in_line ("Index", tag->nodename) != -1)
	    {
	      NODE *node;
	      REFERENCE **menu;

	      /* Found one.  Get its menu. */
	      node = info_get_node (tag->filename, tag->nodename);
	      if (!node)
		continue;

	      /* Remember the filename and nodename of this index. */
	      initial_index_filename = savestring (file_buffer->filename);
	      initial_index_nodename = savestring (tag->nodename);

	      menu = info_menu_of_node (node);

	      /* If we have a menu, add this index's nodename and range
		 to our list of index_nodenames. */
	      if (menu)
		{
		  add_index_to_index_nodenames (menu, node);

		  /* Concatenate the references found so far. */
		  result = info_concatenate_references (result, menu);
		}
	      free (node);
	    }
	}
    }
  /* If we didn't have an indirect tags table, there might be a nodes list. */
  else if (file_buffer->nodes)
    {
      NODE_ENTRY *entry;

      for (i = 0; entry = file_buffer->nodes[i]; i++)
	{
	  if (string_in_line ("Index", entry->nodename) != -1)
	    {
	      NODE *node;
	      REFERENCE **menu;

	      /* Found one.  Get its menu. */
	      node = info_get_node (file_buffer->filename, entry->nodename);
	      if (!node)
		continue;

	      /* Remember the file and node of this initial index. */
	      initial_index_filename = savestring (file_buffer->filename);
	      initial_index_nodename = savestring (entry->nodename);

	      menu = info_menu_of_node (node);

	      /* If we have a menu, add this index's nodename and range
		 to our list of index_nodenames. */
	      if (menu)
		{
		  add_index_to_index_nodenames (menu, node);

		  /* Concatenate the references found so far. */
		  result = info_concatenate_references (result, menu);
		}
	      free (node);
	    }
	}
    }

  /* If there is a result, clean it up so that every entry has a filename. */
  for (i = 0; result && result[i]; i++)
    if (!result[i]->filename)
      result[i]->filename = savestring (file_buffer->filename);

  return (result);
}

DECLARE_INFO_COMMAND (info_index_search,
   "Look up a string in the index for this file")
{
  FILE_BUFFER *fb;
  char *line;

  /* Reset the index offset, since this is not the info-index-next command. */
  index_offset = 0;

  /* The user is selecting a new search string, so flush the old one. */
  maybe_free (index_search);
  index_search = (char *)NULL;

  /* If this window's file is not the same as the one that we last built an
     index for, build and remember an index now. */
  fb = file_buffer_of_window (window);
  if (!initial_index_filename ||
      (strcmp (initial_index_filename, fb->filename) != 0))
    {
      info_free_references (index_index);
      window_message_in_echo_area ("Finding index entries...");
      index_index = info_indices_of_file_buffer (fb);
    }

  /* If there is no index, quit now. */
  if (!index_index)
    {
      info_error ("No indices found.");
      return;
    }

  /* Okay, there is an index.  Let the user select one of the members of it. */
  line =
    info_read_maybe_completing (window, "Index entry: ", index_index);

  window = active_window;

  /* User aborted? */
  if (!line)
    {
      info_abort_key (active_window, 1, 0);
      return;
    }

  /* Empty line means move to the Index node. */
  if (!*line)
    {
      free (line);

      if (initial_index_filename && initial_index_nodename)
	{
	  NODE *node;

	  node =
	    info_get_node (initial_index_filename, initial_index_nodename);
	  set_remembered_pagetop_and_point (window);
	  window_set_node_of_window (window, node);
	  remember_window_and_node (window, node);
	  window_clear_echo_area ();
	  return;
	}
    }

  /* The user typed either a completed index label, or a partial string.
     Find an exact match, or, failing that, the first index entry containing
     the partial string.  So, we just call info_next_index_match () with minor
     manipulation of INDEX_OFFSET. */
  {
    int old_offset;

    /* Start the search right after/before this index. */
    if (count < 0)
      {
	register int i;
	for (i = 0; index_index[i]; i++);
	index_offset = i;
      }
    else
      index_offset = -1;

    old_offset == index_offset;

    /* The "last" string searched for is this one. */
    index_search = line;

    /* Find it, or error. */
    info_next_index_match (window, count, 0);

    /* If the search failed, return the index offset to where it belongs. */
    if (index_offset == old_offset)
      index_offset = 0;
  }
}

DECLARE_INFO_COMMAND (info_next_index_match,
 "Go to the next matching index item from the last `\\[index-search]' command")
{
  register int i;
  int partial, dir;
  NODE *node;

  /* If there is no previous search string, the user hasn't built an index
     yet. */
  if (!index_search)
    {
      info_error ("No previous index search string.");
      return;
    }

  /* If there is no index, that is an error. */
  if (!index_index)
    {
      info_error ("No index entries.");
      return;
    }

  /* The direction of this search is controlled by the value of the
     numeric argument. */
  if (count < 0)
    dir = -1;
  else
    dir = 1;

  /* Search for the next occurence of index_search.  First try to find
     an exact match. */
  partial = 0;

  for (i = index_offset + dir; (i > -1) && (index_index[i]); i += dir)
    if (strcmp (index_search, index_index[i]->label) == 0)
      break;

  /* If that failed, look for the next substring match. */
  if ((i < 0) || (!index_index[i]))
    {
      for (i = index_offset + dir; (i > -1) && (index_index[i]); i += dir)
	if (string_in_line (index_search, index_index[i]->label) != -1)
	  break;

      if ((i > -1) && (index_index[i]))
	partial = string_in_line (index_search, index_index[i]->label);
    }

  /* If that failed, print an error. */
  if ((i < 0) || (!index_index[i]))
    {
      info_error ("No %sindex entries containing \"%s\".",
		  index_offset > 0 ? "more " : "", index_search);
      return;
    }

  /* Okay, we found the next one.  Move the offset to the current entry. */
  index_offset = i;

  /* Report to the user on what we have found. */
  {
    register int j;
    char *name = "CAN'T SEE THIS";
    char *match;

    for (j = 0; index_nodenames[j]; j++)
      {
	if ((i >= index_nodenames[j]->first) &&
	    (i <= index_nodenames[j]->last))
	  {
	    name = index_nodenames[j]->name;
	    break;
	  }
      }

    /* If we had a partial match, indicate to the user which part of the
       string matched. */
    match = savestring (index_index[i]->label);

    if (partial && show_index_match)
      {
	int j, ls, start, upper;

	ls = strlen (index_search);
	start = partial - ls;
	upper = isupper (match[start]) ? 1 : 0;

	for (j = 0; j < ls; j++)
	  if (upper)
	    match[j + start] = info_tolower (match[j + start]);
	  else
	    match[j + start] = info_toupper (match[j + start]);
      }

    {
      char *format;

      format = replace_in_documentation
	("Found \"%s\" in %s. (`\\[next-index-match]' tries to find next.)");
      
      window_message_in_echo_area (format, match, name);
    }

    free (match);
  }

  /* Select the node corresponding to this index entry. */
  node = info_get_node (index_index[i]->filename, index_index[i]->nodename);

  if (!node)
    {
      info_error (CANT_FILE_NODE,
		  index_index[i]->filename, index_index[i]->nodename);
      return;
    }

  set_remembered_pagetop_and_point (window);
  window_set_node_of_window (window, node);
  remember_window_and_node (window, node);


  /* Try to find an occurence of LABEL in this node. */
  {
    long start, loc;

    start = window->line_starts[1] - window->node->contents;
    loc = info_target_search_node (node, index_index[i]->label, start);

    if (loc != -1)
      {
	window->point = loc;
	window_adjust_pagetop (window);
      }
  }
}
