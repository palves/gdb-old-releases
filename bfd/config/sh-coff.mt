# Target: Hitachi SH using COFF and Srecords
# Avoid dragging in a lot of other architectures and formats.
TDEFAULTS=-DBFD -DSELECT_ARCHITECTURES=bfd_sh_arch -DSELECT_VECS='&shcoff_vec,&srec_vec'
TDEPFILES=cpu-sh.o coff-sh.o
