include_directories(
  ${DBUS_INCLUDE_DIR}
  ${DBUS_ARCH_INCLUDE_DIR}
)

########### next target ###############

if(FREEBSD)
   set (FBSD_SUBR "${PROJECT_SOURCE_DIR}/os/freebsd/memstream.c")
endif(FREEBSD)

SET(gshdbus_STAT_SRCS
   dbus_server.c
   ${FBSD_SUBR}
)

add_library(gshdbus STATIC ${gshdbus_STAT_SRCS})


########### install files ###############
