/* infomap.h -- Description of a keymap in Info and related functions. */

#if !defined (_INFOMAP_H_)
#define _INFOMAP_H_

#include "general.h"

#define ESC '\033'
#define DEL '\177'
#define TAB '\011'	
#define RET '\r'
#define LFD '\n'
#define SPC ' '

#define meta_character_threshold (DEL + 1)
#define control_character_threshold (SPC)

#define meta_character_bit 0x80
#define control_character_bit 0x40

#define Meta_p(c) (((c) > meta_character_threshold))
#define Control_p(c) ((c) < control_character_threshold)

#define Meta(c) ((c) | (meta_character_bit))
#define UnMeta(c) ((c) & (~meta_character_bit))
#define Control(c) ((toupper (c)) & (~control_character_bit))
#define UnControl(c) (tolower ((c) | control_character_bit))

/* A keymap contains one entry for each key in the ASCII set.
   Each entry consists of a type and a pointer.
   FUNCTION is the address of a function to run, or the
   address of a keymap to indirect through.
   TYPE says which kind of thing FUNCTION is. */
typedef struct {
  char type;
  VFunction *function;
} KEYMAP_ENTRY;

typedef KEYMAP_ENTRY *Keymap;

/* The values that TYPE can have in a keymap entry. */
#define ISFUNC 0
#define ISKMAP 1

extern Keymap info_keymap;
extern Keymap echo_area_keymap;

/* Return a new keymap which has all the uppercase letters mapped to run
   the function info_do_lowercase_version (). */
extern Keymap keymap_make_keymap ();

/* Return a new keymap which is a copy of MAP. */
extern Keymap keymap_copy_keymap ();

/* Free MAP and it's descendents. */
extern void keymap_discard_keymap ();

/* Initialize the info keymaps. */
extern void initialize_info_keymaps ();

#endif /* !_INFOMAP_H_ */
