
########### next target ###############

SET(fsalfuse_STAT_SRCS
   fsal_access.c
   fsal_context.c
   fsal_dirs.c
   fsal_fsinfo.c
   fsal_rcp.c
   fsal_compat.c
   fsal_attrs.c
   fsal_convert.c
   fsal_init.c
   fsal_lookup.c
   fsal_rename.c
   fsal_symlinks.c
   fsal_unlink.c
   fsal_local_op.c
   fsal_common.c
   fsal_create.c
   fsal_fileop.c
   fsal_internal.c
   fsal_stats.c
   fsal_tools.c
   fsal_xattrs.c
   fsal_common.h
   fsal_convert.h
   fsal_internal.h
   fsal_fuse_helper.c
   namespace.c
   namespace.h
   ../../include/fsal.h
   ../../include/fsal_types.h
   ../../include/FSAL/FSAL_FUSELIKE/fsal_types.h
   ../../include/ganesha_fuse_wrap.h
)

add_library(fsalfuse STATIC ${fsalfuse_STAT_SRCS})


########### next target ###############

SET(test_namespace_fuse_SRCS
   test_ns.c
)

add_executable(test_namespace_fuse ${test_namespace_fuse_SRCS})

target_link_libraries(test_namespace_fuse ${SYSTEM_LIBRARIES})
