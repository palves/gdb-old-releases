# Target:  Intel 386 running DOS
# Use a.out as the default vector,
# because that's what we want to use for everything but linking
# (i.e. for object files and libraries and all that).
# Producing the final executable in MS-DOS format is handled by the linker.
DEFAULT_VECTOR=i386aout_vec
SELECT_VECS=i386msdos_vec
SELECT_ARCHITECTURES=bfd_i386_arch
