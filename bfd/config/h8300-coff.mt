# Target: Hitachi H8/300 using COFF and S-Record object file format
# Avoid dragging in a lot of other architectures and formats.
TDEFAULTS=-DBFD -DSELECT_ARCHITECTURES=bfd_h8300_arch -DSELECT_VECS='&h8300coff_vec,&srec_vec,&symbolsrec_vec'
CC=gcc
