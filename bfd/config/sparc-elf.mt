# Target:  SPARC CPU using ELF format.
TDEFAULTS = -DDEFAULT_VECTOR=bfd_elf32_sparc_vec
TARGET_BACKENDS = elf32-sparc.o elf32.o elf.o
