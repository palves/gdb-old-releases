# Target:  PowerPC using ELF, default to little endian

DEFAULT_VECTOR=bfd_elf32_powerpcle_vec

SELECT_VECS=rs6000coff_vec bfd_elf32_powerpc_vec
SELECT_ARCHITECTURES=bfd_powerpc_arch bfd_rs6000_arch
