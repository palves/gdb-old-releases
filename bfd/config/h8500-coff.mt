# Target: Hitachi H8/500 using COFF and Srecords
# Avoid dragging in a lot of other architectures and formats.
TDEFAULTS=-DBFD -DSELECT_ARCHITECTURES=bfd_h8500_arch -DSELECT_VECS='&h8500coff_vec,&srec_vec'
