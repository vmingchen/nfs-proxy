# NFS Ganesha  Cmake

# Current version as of Fedora 16.  Not tested with earlier.

cmake_minimum_required(VERSION 2.6.3)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

project(nfs-ganesha C)

set(${PROJECT_NAME}_MAJOR_VERSION 2)
set(${PROJECT_NAME}_MINOR_VERSION 0)
set(${PROJECT_NAME}_PATCH_LEVEL 0)
set(VERSION_COMMENT
  "GANESHA 64 bits compliant. FSAL_PROXY re-exports NFSv3. RPCSEC_GSS support (partial). FUSELIKE added"
)

# find out which platform we are building on
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(LINUX ON)
  set(UNIX ON)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

if(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
  set(FREEBSD ON)
  set(UNIX ON)
endif(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  set(WINDOWS ON)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

find_package(Toolchain REQUIRED)

# Add maintainer mode for (mainly) strict builds
include(${CMAKE_SOURCE_DIR}/cmake/maintainer_mode.cmake)

# If we are in a git tree, then this CMakeLists.txt is in "src/" and go .git is in "src/.."
IF( EXISTS ${CMAKE_SOURCE_DIR}/../.git/HEAD  )
  message( STATUS "Compilation from within a git repository. Using GetGitRevisionDescription" )
  include(GetGitRevisionDescription)
  get_git_head_revision(GIT_REFSPEC _GIT_HEAD_COMMIT)
  git_describe(_GIT_DESCRIBE)

ELSE( EXISTS ${CMAKE_SOURCE_DIR}/../.git/HEAD  )
  message( STATUS "Outside a git repository, use saved data" )
  EXEC_PROGRAM(${CMAKE_SOURCE_DIR}/cmake/githead_from_path.sh ARGS ${CMAKE_SOURCE_DIR}
                OUTPUT_VARIABLE  _GIT_HEAD_COMMIT)

   
  EXEC_PROGRAM(${CMAKE_SOURCE_DIR}/cmake/gitdesc_from_path.sh ARGS ${CMAKE_SOURCE_DIR}
                OUTPUT_VARIABLE  _GIT_DESCRIBE)

ENDIF( EXISTS ${CMAKE_SOURCE_DIR}/../.git/HEAD  )

# Using git related information, build PACKNAME
IF(DISTNAME_HAS_GIT_DATA)
	set( PACKNAME "_git${_GIT_HEAD_COMMIT}_desc_${_GIT_DESCRIBE}" )
ELSE(DISTNAME_HAS_GIT_DATA)
	set( PACKNAME "" ) 
ENDIF(DISTNAME_HAS_GIT_DATA)

# Define CPACK component (to deal with sub packages)
set(CPACK_COMPONENTS_ALL daemon fsal headers )
set(CPACK_COMPONENT_DAEMON_DISPLAY_NAME "NFS-Ganesha daemon")
set(CPACK_COMPONENT_FSAL_DISPLAY_NAME "FSAL Libraries")
set(CPACK_COMPONENT_HEADERS_DISPLAY_NAME "Headers")

# Include custom config and cpack module
include(${CMAKE_SOURCE_DIR}/cmake/cpack_config.cmake)
include(CPack)

include(${CMAKE_SOURCE_DIR}/cmake/rpmtools_config.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/modules/UseRPMTools.cmake)
IF(RPMTools_FOUND)
     RPMTools_ADD_RPM_TARGETS(${PROJECT_NAME})
ENDIF(RPMTools_FOUND)

# find out which platform we are building on
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(LINUX ON)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

if(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
  set(FREEBSD ON)
endif(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")

if (FREEBSD)
#default gcc doesn't like using -Wuninitialized without -O on FreeBSD
    set(PLATFORM "FREEBSD")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O -ggdb")
    set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -lexecinfo -rdynamic")
    set(OS_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include/os/freebsd")
    find_library(LIBDL c)  # libc suffices on freebsd 
endif(FREEBSD)

if (LINUX)
    set(PLATFORM "LINUX")
    set(OS_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include/os/linux")
    find_library(LIBDL dl)  # module loader
endif(LINUX)

if (MSVC)
   add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif(MSVC)

# Library destination, 32 bits / 64 bits switch is done here
if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
    set( FSAL_DESTINATION "lib64/ganesha" )
else( CMAKE_SIZEOF_VOID_P EQUAL 8 )
    set( FSAL_DESTINATION "lib/ganesha" )
endif( CMAKE_SIZEOF_VOID_P EQUAL 8 )

# FSAL selection
# FSALs which are enabled by default but could be disabled
# during the build
option(USE_FSAL_PROXY "build PROXY FSAL shared library" ON)
option(USE_FSAL_VFS "build VFS FSAL shared library" ON)
option(USE_FSAL_POSIX "build POSIX FSAL shared library" ON)
option(USE_FSAL_CEPH "build CEPH FSAL shared library" ON)
option(USE_FSAL_GPFS "build GPFS FSAL" ON)
option(USE_FSAL_ZFS "build ZFS FSAL" ON)
option(USE_FSAL_LUSTRE "build LUSTRE FSAL" ON)
option(USE_FSAL_XFS "build XFS support in VFS FSAL" ON)

# FSALs which are disabled by default
option(USE_FSAL_HPSS "build HPSS FSAL" OFF)
option(USE_FSAL_FUSE "build FUSE FSAL" OFF)

# SHOOK is a variant of Lustre FSAL
option(USE_FSAL_SHOOK "build SHOOK FSAL" OFF)

# From TI-RPC
option (USE_GSS "enable RPCSEC_GSS support" ON)
option(TIRPC_EPOLL "platform supports EPOLL or emulation" ON)

# Build configure options
option(USE_DBUS "enable DBUS protocol support" OFF)

# Various DBUS enabled features
option(USE_CB_SIMULATOR "enable callback simulator thread" OFF)
option(USE_DBUS_STATS "enable DBUS interface for server statistics" OFF)

option(USE_NFSIDMAP "Use of libnfsidmap for name resolution" ON)
option(ENABLE_ERROR_INJECTION "enable error injection" OFF)

# Electric Fence (-lefence) link flag
option(USE_EFENCE "link with efence memory debug library" OFF)

# These are -D_FOO options, why ???  should be flags??
option(_NO_TCP_REGISTER "disable registration of tcp services on portmapper" ON)
option(_NO_PORTMAPPER "disable registration on portmapper" ON)
option(_NO_XATTRD "disable ghost xattr directory and files support" ON)
option(DEBUG_SAL "enable debugging of SAL by keeping list of all locks, stateids, and state owners" OFF)
option(USE_NODELIST "enable NodeList support" OFF)
option(_HANDLE_MAPPING "enable NFSv2/3 handle mapping for PROXY FSAL" OFF)

# Debug symbols (-g) build flag 
option(DEBUG_SYMS "include debug symbols to binaries (-g option)" OFF)

# enable code profiling [-g -pg]
option(PROFILING "turn on code profiling (-g and -pg)" OFF)

# MSPAC support -lwbclient link flag
option(_MSPAC_SUPPORT "enable mspac Winbind support" ON)

# CUnit
option(USE_CUNIT "Use Cunit test framework" OFF)

# Enable 9P Support
option(USE_9P "enable 9P support" ON)
option(USE_9P_RDMA "enable 9P_RDMA support" OFF)

# This option will stop cmake compilation if a requested FSAL could not be built
option(STRICT_PACKAGE "Enable strict packaging behavior" OFF )

# This option will trigger "long distro name" aka name that contains git information
option(DISTNAME_HAS_GIT_DATA "Distribution package's name carries git data" OFF )

# Compile with "NO_MOUNT_LIST" [Don't use MOUNT PROTOCOL's  client list]
set(_NO_MOUNT_LIST ON)

# Choose a shortcut build config

IF(BUILD_CONFIG)
  INCLUDE(
  ${CMAKE_SOURCE_DIR}/cmake/build_configurations/${BUILD_CONFIG}.cmake)
ENDIF()

IF(DEBUG_SYMS)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -g")
ENDIF(DEBUG_SYMS)

# Build source locations and parameters
IF(USE_FSAL_CEPH)
  IF(CEPH_PREFIX)
    set(CEPH_PREFIX ${CEPH_PREFIX} CACHE PATH "Path to Ceph installation")
    LIST(APPEND CMAKE_PREFIX_PATH "${CEPH_PREFIX}")
    LIST(APPEND CMAKE_LIBRARY_PATH "${CEPH_PREFIX}/lib")
  ELSE()
    set(CEPH_PREFIX "/usr" CACHE PATH "Path to Ceph installation")
  ENDIF()
ENDIF()

IF(USE_FSAL_HPSS)
  IF(HPSS_PREFIX)
    set(HPSS_PREFIX ${HPSS_PREFIX} CACHE PATH "Path to HPSS installation")
    LIST(APPEND CMAKE_LIBRARY_PATH "${HPSS_PREFIX}/lib")
    include_directories("${HPSS_PREFIX}/include")
  ELSE()
    set(HPSS_PREFIX "/usr" CACHE PATH "Path to HPSS installation")
  ENDIF()
ENDIF()

IF(KRB5_PREFIX)
  set(KRB5_PREFIX ${KRB5_PREFIX} CACHE PATH "Path to Krb5 installation")
  LIST(APPEND CMAKE_PREFIX_PATH "${KRB5_PREFIX}")
  LIST(APPEND CMAKE_LIBRARY_PATH "${KRB5_PREFIX}/lib")
ENDIF()

if(SAMBA4_PREFIX)
  set(SAMBA4_PREFIX ${SAMBA4_PREFIX} CACHE PATH "Path to Samba4 installation")
  LIST(APPEND CMAKE_PREFIX_PATH "${SAMBA4_PREFIX}")
  LIST(APPEND CMAKE_LIBRARY_PATH "${SAMBA4_PREFIX}/lib")
endif()

if(USE_CB_SIMULATOR AND NOT USE_DBUS)
  message(WARNING "The callback simulator needs DBUS.  Enabling DBUS")
  set(USE_DBUS ON)
endif(USE_CB_SIMULATOR AND NOT USE_DBUS)

if(USE_DBUS_STATS AND NOT USE_DBUS)
  message(WARNING "The DBUS server statistics needs DBUS.  Enabling DBUS")
  set(USE_DBUS ON)
endif(USE_DBUS_STATS AND NOT USE_DBUS)

if(USE_9P_RDMA AND NOT USE_9P)
  message(WARNING "The support of 9P/RDMA needs 9P protocol support. Enabling 9P")
  set(USE_9P ON)
endif(USE_9P_RDMA AND NOT USE_9P)

set(ALLOCATOR "jemalloc" CACHE STRING
  "specify the memory allocator to use: jemalloc|tcmalloc|libc")

# Find packages and libs we need for building
include(CheckIncludeFiles)
include(CheckLibraryExists)
include(CheckCSourceCompiles)
include(TestBigEndian)

check_include_files(stdbool.h HAVE_STDBOOL_H)
check_include_files(strings.h HAVE_STRINGS_H)
check_include_files(string.h HAVE_STRING_H)

# X_ATTRD requires the kernel to have xattrs...DBUS_STATS
if(NOT _NO_XATTRD)
  check_include_files("unistd.h;sys/xattr.h" HAVE_XATTR_H)
  if(NOT HAVE_XATTR_H)
    message(WARNING "Cannot find xattr.h. Disabling XATTRD support: ${HAVE_XATTR_H}")
    set(_NO_XATTRD ON)
  endif(NOT HAVE_XATTR_H)
endif(NOT _NO_XATTRD)

TEST_BIG_ENDIAN(BIGENDIAN)
if(${BIGENDIAN})
  set(BIGEND ON)
else()
  set(LITTLEEND ON)
endif(${BIGENDIAN})

find_package(Threads REQUIRED)
find_package(Krb5 REQUIRED gssapi)

if(KRB5_FOUND)
  set(HAVE_KRB5 ON)
  set(KRB5_VERSION 194)  # hand code until we do krb5-config --version magic
  set(_HAVE_GSSAPI ON)
endif(KRB5_FOUND)

# needed by RPCAL/gssd for some reason
find_package(Curses)
find_package(Readline)

find_package(OpenSSL REQUIRED) # md5 needed by  ${CMAKE_THREAD_LIBS_INIT} nfs4 acls (why??)
set(SYSTEM_LIBRARIES ${SYSTEM_LIBRARIES} ${OPENSSL_LIBRARIES})

# Validate fsal dependencies
if(USE_FSAL_CEPH)
  find_library(CEPH_FS cephfs)
  check_library_exists(
    cephfs
    ceph_ll_connectable_x
    ${CEPH_PREFIX}
    CEPH_FS
    )
  if(NOT CEPH_FS)
if(STRICT_PACKAGE)
      message(FATAL_ERROR "STRICT_PACKAGE : Cannot find CEPH runtime.  Disabling CEPH fsal build")
    else(STRICT_PACKAGE)
      message(WARNING "Cannot find CEPH runtime.  Disabling CEPH fsal build")
      set(USE_FSAL_CEPH OFF)
    endif(STRICT_PACKAGE)
 endif(NOT CEPH_FS)
endif(USE_FSAL_CEPH)

if(USE_FSAL_HPSS)
  find_library(HPSS_LIBRARY
    NAMES hpss)
  if(NOT HPSS_LIBRARY)
    message(WARNING "Cannot find HPSS libraries. Disabling HPSS fsal build")
    set(USE_FSAL_HPSS OFF)
  endif(NOT HPSS_LIBRARY)
endif(USE_FSAL_HPSS)

if(USE_FSAL_LUSTRE)
  check_library_exists(
    lustreapi
    llapi_is_lustre_mnt
    ""
    HAVE_LUSTREAPI
    )
  check_include_files("unistd.h;lustre/lustre_user.h" HAVE_LUSTRE_USER_H)
  if((NOT HAVE_LUSTREAPI) OR (NOT HAVE_LUSTRE_USER_H))
   if(STRICT_PACKAGE)
      message(FATAL_ERROR "STRICT_PACKAGE: Cannot find LUSTRE runtime. Disabling LUSTRE fsal build")
    else(STRICT_PACKAGE)
      message(WARNING "Cannot find LUSTRE runtime. Disabling LUSTRE fsal build")
      set(USE_FSAL_LUSTRE OFF)
    endif(STRICT_PACKAGE)
  endif((NOT HAVE_LUSTREAPI) OR (NOT HAVE_LUSTRE_USER_H))
endif(USE_FSAL_LUSTRE)

if(USE_FSAL_LUSTRE AND USE_FSAL_SHOOK)
  check_library_exists(
    shooksvr
    shook_restore
    ""
    HAVE_SHOOK
    )
  check_include_files("unistd.h;attr/xattr.h" HAVE_SHOOK_XATTRS)
  if((NOT HAVE_SHOOK) OR (NOT HAVE_SHOOK_XATTRS))
   if(STRICT_PACKAGE)
      message(FATAL_ERROR "STRICT_PACKAGE: Cannot find SHOOK runtime. Disabling SHOOK build")
    else(STRICT_PACKAGE)
      message(WARNING "Cannot find SHOOK runtime. Disabling SHOOK build")
      set(USE_FSAL_SHOOK OFF)
    endif(STRICT_PACKAGE)
  endif((NOT HAVE_SHOOK) OR (NOT HAVE_SHOOK_XATTRS))
endif(USE_FSAL_LUSTRE AND USE_FSAL_SHOOK)

if(USE_FSAL_XFS)
  check_library_exists(
    handle
    "open_by_handle"
    ""
    HAVE_XFS_LIB
    )
  check_include_files("xfs/xfs.h" HAVE_XFS_H)
  if((NOT HAVE_XFS_LIB) OR (NOT HAVE_XFS_H))
   if(STRICT_PACKAGE)
      message(FATAL_ERROR "STRICT_PACKAGE: Cannot find XFS runtime. Disabling XFS build")
    else(STRICT_PACKAGE)
      message(WARNING "Cannot find XFS runtime. Disabling XFS build")
      set(USE_FSAL_XFS OFF)
    endif(STRICT_PACKAGE)
  endif((NOT HAVE_XFS_LIB) OR (NOT HAVE_XFS_H))
endif(USE_FSAL_XFS)

if(USE_FSAL_ZFS)
  check_library_exists(
    zfswrap
    libzfswrap_init
    ""
    HAVE_ZFS_LIB
    )
  check_include_files("unistd.h;libzfswrap.h" HAVE_LIBZFSWRAP_H)
  if((NOT HAVE_ZFS_LIB) OR (NOT HAVE_LIBZFSWRAP_H))
   if(STRICT_PACKAGE)
      message(FATAL_ERROR "STRICT_PACKAGE: Cannot find ZFS runtime. Disabling ZFS build")
    else(STRICT_PACKAGE)
      message(WARNING "Cannot find ZFS runtime. Disabling ZFS build")
      set(USE_FSAL_ZFS OFF)
    endif(STRICT_PACKAGE)
  endif((NOT HAVE_ZFS_LIB) OR (NOT HAVE_LIBZFSWRAP_H))
endif(USE_FSAL_ZFS)

# sort out which allocator to use
if(${ALLOCATOR} STREQUAL "jemalloc")
  find_package(JeMalloc)
  if(JEMALLOC_FOUND)
    set(SYSTEM_LIBRARIES ${JEMALLOC_LIBRARIES})
  else(JEMALLOC_FOUND)
    message(WARNING "jemalloc not found, falling back to libc")
    set(ALLOCATOR "libc")
  endif(JEMALLOC_FOUND)
elseif(${ALLOCATOR} STREQUAL "tcmalloc")
  find_package(TcMalloc)
  if(TCMALLOC_FOUND)
    set(SYSTEM_LIBRARIES ${TCMALLOC_LIBRARIES})
  else(TCMALLOC_FOUND)
    message(WARNING "tcmalloc not found, falling back to libc")
    set(ALLOCATOR "libc")
  endif(TCMALLOC_FOUND)
else()
  if(NOT ${ALLOCATOR} STREQUAL "libc")
    message(SEND_ERROR "${ALLOCATOR}  is not a valid allocator option.")
  endif()
endif()

# Find optional libraries/packages
if(USE_EFENCE)
  find_package(efence REQUIRED)
  set(SYSTEM_LIBRARIES ${efence_LIBRARIES} ${SYSTEM_LIBRARIES})
endif(USE_EFENCE)

if(USE_DBUS)
  find_package(DBus REQUIRED)
  set(SYSTEM_LIBRARIES ${DBUS_LIBRARIES} ${SYSTEM_LIBRARIES})
endif(USE_DBUS)

if(USE_NFSIDMAP)
  find_package(NfsIdmap REQUIRED)
  set(SYSTEM_LIBRARIES ${NFSIDMAP_LIBRARY} ${SYSTEM_LIBRARIES})
endif(USE_NFSIDMAP)

if(USE_CUNIT)
  find_package(cunit REQUIRED)
  set(SYSTEM_LIBRARIES ${cunit_LIBRARIES} ${SYSTEM_LIBRARIES})
endif(USE_CUNIT)

if(_MSPAC_SUPPORT)
  find_package(WBclient REQUIRED)
  if(WBCLIENT_FOUND AND WBCLIENT4_H)
    set(SYSTEM_LIBRARIES ${WBCLIENT_LIBRARIES} ${SYSTEM_LIBRARIES})
  else(WBCLIENT_FOUND AND WBCLIENT4_H)
    message(WARNING "Samba 4 wbclient not found.  Disabling MSPAC_SUPPORT")
    set(_MSPAC_SUPPORT OFF)
  endif(WBCLIENT_FOUND AND WBCLIENT4_H)
endif(_MSPAC_SUPPORT)

include_directories(
  "${PROJECT_BINARY_DIR}/include"
  "${PROJECT_SOURCE_DIR}/libntirpc/tirpc/"
  "${PROJECT_SOURCE_DIR}/include"
  "${OS_INCLUDE_DIR}"
  "${KRB5_INCLUDE_DIRS}"
)

# Fixup loose bits of autotools legacy
set(_USE_9P ${USE_9P})
set(_USE_9P_RDMA ${USE_9P_RDMA})

if(USE_CB_SIMULATOR)
  set(_USE_CB_SIMULATOR ON)
endif(USE_CB_SIMULATOR)


#callers add:
#  target_link_libraries(your_library ${LIBTIRPC_LIBRARIES})
set(LIBTIRPC_INCLUDE_DIR
  "${PROJECT_BINARY_DIR}/libtirpc/tirpc"
  "${PROJECT_BINARY_DIR}/libtirpc/tirpc/rpc"
)
set(LIBTIRPC_LIBRARIES "${PROJECT_BINARY_DIR}/libntirpc/src/libntirpc.a")

# Find misc system libs

find_library(LIBRT rt)  # extended Pthreads functions

# Roll up required libraries

#Protocols we support
set(PROTOCOLS
  nfsproto
  nlm
  rquota
)

if(USE_9P)
  set(PROTOCOLS ${PROTOCOLS} 9p)
endif(USE_9P)

set(PROTOCOLS
  ${PROTOCOLS}
  nfs_mnt_xdr
  )

# Core subsystems
set(GANESHA_CORE
  sal
  cache_inode
  idmap
  avltree
  weakref
  hashtable
  rpcal
  gssd
  log
  common_utils
  support
  fsalup
  nfs4callbacks
  cidr
  string_utils
  hash
  )

if(USE_DBUS)
  set(GANESHA_CORE ${GANESHA_CORE} gshdbus)
endif(USE_DBUS)

if(USE_NODELIST)
  set(GANESHA_CORE ${GANESHA_CORE} NodeList)
endif(USE_NODELIST)

if(USE_9P_RDMA)
  set(GANESHA_CORE ${GANESHA_CORE} mooshika rdmacm ibverbs)
endif(USE_9P_RDMA)

# All the plumbing in the basement
set(SYSTEM_LIBRARIES
  ${SYSTEM_LIBRARIES}
  gos
  ${LIBDL}
  ${KRB5_LIBRARIES}
  crypto
  ssl
  ${CMAKE_THREAD_LIBS_INIT}
  ${LIBRT}
)

IF( NOT EXISTS ${PROJECT_SOURCE_DIR}/libntirpc/CMakeLists.txt )
message( FATAL_ERROR 
"No CMakeLists.txt found in libntirpc
This probably means that the directory was not properly populated.
Please run the following command to do that and use cmake again:
    git submodule update --init" )
ENDIF( NOT EXISTS ${PROJECT_SOURCE_DIR}/libntirpc/CMakeLists.txt )

add_subdirectory(include)
add_subdirectory(Log)
add_subdirectory(Common)
add_subdirectory(ConfigParsing)
add_subdirectory(cidr)
add_subdirectory(test)
add_subdirectory(avl)
add_subdirectory(HashTable)
if(USE_NODELIST)
  add_subdirectory(NodeList)
endif(USE_NODELIST)
add_subdirectory(Cache_inode)
add_subdirectory(SAL)
add_subdirectory(libntirpc)
add_subdirectory(RPCAL)
add_subdirectory(Protocols)
add_subdirectory(FSAL_UP)
add_subdirectory(support)
add_subdirectory(os)

if(USE_FSAL_POSIX)
  add_subdirectory(NodeDB)
endif(USE_FSAL_POSIX)

add_subdirectory(FSAL)
add_subdirectory(IdMapper)

add_subdirectory(MainNFSD)
add_subdirectory(tools)

if(USE_FSAL_FUSE)
  add_subdirectory(example-fuse)
endif(USE_FSAL_FUSE)

# display configuration vars

message(STATUS)
message(STATUS "-------------------------------------------------------")
message(STATUS "PLATFORM = ${PLATFORM}")
message(STATUS "-------------------------------------------------------")
message(STATUS "USE_FSAL_PROXY = ${USE_FSAL_PROXY}")
message(STATUS "USE_FSAL_VFS = ${USE_FSAL_VFS}")
message(STATUS "USE_FSAL_POSIX = ${USE_FSAL_POSIX}")
message(STATUS "USE_FSAL_CEPH = ${USE_FSAL_CEPH}")
message(STATUS "USE_FSAL_HPSS = ${USE_FSAL_HPSS}")
message(STATUS "USE_FSAL_FUSE = ${USE_FSAL_FUSE}")
message(STATUS "USE_FSAL_XFS = ${USE_FSAL_XFS}")
message(STATUS "USE_FSAL_GPFS = ${USE_FSAL_GPFS}")
message(STATUS "USE_FSAL_ZFS = ${USE_FSAL_ZFS}")
message(STATUS "USE_FSAL_LUSTRE = ${USE_FSAL_LUSTRE}")
message(STATUS "USE_FSAL_SHOOK = ${USE_FSAL_SHOOK}")
message(STATUS "USE_DBUS = ${USE_DBUS}")
message(STATUS "USE_CB_SIMULATOR = ${USE_CB_SIMULATOR}")
message(STATUS "USE_DBUS_STATS = ${USE_DBUS_STATS}")
message(STATUS "USE_NFSIDMAP = ${USE_NFSIDMAP}")
message(STATUS "ENABLE_ERROR_INJECTION = ${ENABLE_ERROR_INJECTION}")
message(STATUS "STRICT_PACKAGE = ${STRICT_PACKAGE}")
message(STATUS "DISTNAME_HAS_GIT_DATA = ${DISTNAME_HAS_GIT_DATA}" )
message(STATUS "_MSPAC_SUPPORT = ${_MSPAC_SUPPORT}")
message(STATUS "USE_EFENCE = ${USE_EFENCE}")
message(STATUS "_NO_TCP_REGISTER = ${_NO_TCP_REGISTER}")
message(STATUS "_NO_PORTMAPPER = ${_NO_PORTMAPPER}")
message(STATUS "_NO_XATTRD = ${_NO_XATTRD}")
message(STATUS "DEBUG_SAL = ${DEBUG_SAL}")
message(STATUS "USE_NODELIST = ${USE_NODELIST}")
message(STATUS "_HANDLE_MAPPING = ${_HANDLE_MAPPING}")
message(STATUS "DEBUG_SYMS = ${DEBUG_SYMS}")
message(STATUS "PROFILING = ${PROFILING}")
message(STATUS "USE_GSS = ${USE_GSS}")
message(STATUS "TIRPC_EPOLL = ${TIRPC_EPOLL}")
message(STATUS "USE_9P = ${USE_9P}")
message(STATUS "_USE_9P = ${_USE_9P}")
message(STATUS "_USE_9P_RDMA = ${_USE_9P_RDMA}")
message(STATUS "KRB5_PREFIX = ${KRB5_PREFIX}")
message(STATUS "CEPH_PREFIX = ${CEPH_PREFIX}")
message(STATUS "HPSS_PREFIX = ${HPSS_PREFIX}")
message(STATUS "CMAKE_PREFIX_PATH = ${CMAKE_PREFIX_PATH}")
message(STATUS "_GIT_HEAD_COMMIT = ${_GIT_HEAD_COMMIT}")
message(STATUS "_GIT_DESCRIBE = ${_GIT_DESCRIBE}")
message(STATUS "ALLOCATOR = ${ALLOCATOR}")
message(STATUS "GOLD_LINKER = ${GOLD_LINKER}")

#force command line options to be stored in cache
set(USE_FSAL_VFS ${USE_FSAL_VFS}
  CACHE BOOL
   "build VFS FSAL shared library"
   FORCE)

set(USE_FSAL_PROXY ${USE_FSAL_PROXY}
  CACHE BOOL
   "build PROXY FSAL shared library"
   FORCE)

set(USE_FSAL_POSIX ${USE_FSAL_POSIX}
  CACHE BOOL
   "build POSIX FSAL shared library"
   FORCE)

set(USE_FSAL_CEPH ${USE_FSAL_CEPH}
  CACHE BOOL
   "build CEPH FSAL shared library"
   FORCE)

set(USE_HPSS_FSAL ${USE_HPSS_FSAL}
  CACHE BOOL
   "build HPSS FSAL"
   FORCE)

set(USE_FSAL_FUSE ${USE_FSAL_FUSE}
  CACHE BOOL
   "build FUSE FSAL"
   FORCE)

set(USE_FSAL_XFS ${USE_FSAL_XFS}
  CACHE BOOL
   "build XFS FSAL"
   FORCE)

set(USE_FSAL_GPFS ${USE_FSAL_GPFS}
  CACHE BOOL
   "build GPFS FSAL"
   FORCE)

set(USE_FSAL_ZFS ${USE_FSAL_ZFS}
  CACHE BOOL
   "build ZFS FSAL"
   FORCE)

set(USE_FSAL_LUSTRE ${USE_FSAL_LUSTRE}
  CACHE BOOL
   "build LUSTRE FSAL"
   FORCE)

set(USE_FSAL_SHOOK ${USE_FSAL_SHOOK}
  CACHE BOOL
   "build SHOOK FSAL"
   FORCE)

set(USE_DBUS ${USE_DBUS}
  CACHE BOOL
   "enable DBUS protocol support"
   FORCE)

set(USE_CB_SIMULATOR ${USE_CB_SIMULATOR}
  CACHE BOOL
   "enable callback simulator thread"
   FORCE)

set(USE_DBUS_STATS ${USE_DBUS_STATS}
  CACHE BOOL
   "enable DBUS interface for server statistics"
   FORCE)

set(USE_NFSIDMAP ${USE_NFSIDMAP}
  CACHE BOOL
   "Use of libnfsidmap for name resolution"
   FORCE)

set(DEBUG_SAL ${DEBUG_SAL}
  CACHE BOOL
   "enable debug SAL"
   FORCE)

set(USE_NODELIST ${USE_NODELIST}
  CACHE BOOL
   "enable NodeList"
   FORCE)

set(ENABLE_ERROR_INJECTION ${ENABLE_ERROR_INJECTION}
  CACHE BOOL
   "enable error injection"
   FORCE)

set(_MSPAC_SUPPORT ${_MSPAC_SUPPORT}
  CACHE BOOL
   "enable mspac winbind support"
   FORCE)

set(STRICT_PACKAGE ${STRICT_PACKAGE}
  CACHE BOOL
   "enable strict packaging behavior"
   FORCE)

set( DISTNAME_HAS_GIT_DATA ${DISTNAME_HAS_GIT_DATA} 
  CACHE BOOL
  "Distribution package's name carries git data" 
  FORCE)


set(USE_9P ${USE_9P}
  CACHE BOOL
   "enable 9P support"
   FORCE)

set(_USE_9P ${_USE_9P}
  CACHE BOOL
   "enable 9P support in config"
   FORCE)

set(_USE_9P_RDMA ${_USE_9P_RDMA}
  CACHE BOOL
   "enable 9P_RDMA support"
   FORCE)

# Now create a useable config.h
configure_file(
  "${PROJECT_SOURCE_DIR}/include/config-h.in.cmake"
  "${PROJECT_BINARY_DIR}/include/config.h"
)

# The fuse library needs a .pc file
if(USE_FSAL_FUSE)
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(LIB_SUFFIX
      "64"
      CACHE STRING "Architecture specific library"
      )
  else(LINUX AND (CMAKE_SIZEOF_VOID_P EQUAL 8))
    set(LIB_SUFFIX
      ""
      CACHE STRING "Architecture specific library"
      )
  endif(LINUX AND (CMAKE_SIZEOF_VOID_P EQUAL 8))

  set(LIB_DESTINATION
    "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}/pkgconfig/"
    CACHE
    STRING "Library directory"
    )

  set(INCLUDE_INSTALL_DIR
    "${CMAKE_INSTALL_PREFIX}/include"
    CACHE PATH "The subdirectory to the header prefix"
    )

  configure_file(
    "${PROJECT_SOURCE_DIR}/libganeshaNFS.pc.cmake"
    "${PROJECT_BINARY_DIR}/libganeshaNFS.pc"
    @ONLY
    )
endif(USE_FSAL_FUSE)


########### install files ###############
if(USE_FSAL_FUSE)
  install(FILES  "${PROJECT_BINARY_DIR}/libganeshaNFS.pc" COMPONENT daemon DESTINATION ${LIB_DESTINATION})
endif(USE_FSAL_FUSE)

########### add a "make dist"  ###############
add_custom_target(dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)

########### add a "make srpm" and a "make rpm" ###############
IF( DISTNAME_HAS_GIT_DATA )
  add_custom_target(rpm COMMAND ${CMAKE_MAKE_PROGRAM}  ${PROJECT_NAME}_rpm )
  add_custom_target(srpm COMMAND ${CMAKE_MAKE_PROGRAM} ${PROJECT_NAME}_srpm)
ELSE( DISTNAME_HAS_GIT_DATA )
  # I need DISTNAME_HAS_GIT_DATA set to ON to carry git data, so I set it, build rpm and set the option back to OFF
  add_custom_target(rpm COMMAND ${CMAKE_COMMAND} -DDISTNAME_HAS_GIT_DATA=ON  ${CMAKE_SOURCE_DIR} 
			COMMAND ${CMAKE_MAKE_PROGRAM}  ${PROJECT_NAME}_rpm 
			COMMAND ${CMAKE_COMMAND} -DDISTNAME_HAS_GIT_DATA=OFF  ${CMAKE_SOURCE_DIR} )

  add_custom_target(srpm  COMMAND ${CMAKE_COMMAND} -DDISTNAME_HAS_GIT_DATA=ON  ${CMAKE_SOURCE_DIR} 
                          COMMAND ${CMAKE_MAKE_PROGRAM} ${PROJECT_NAME}_srpm
			  COMMAND ${CMAKE_COMMAND} -DDISTNAME_HAS_GIT_DATA=OFF ${CMAKE_SOURCE_DIR}  )
ENDIF( DISTNAME_HAS_GIT_DATA )

########### add a "make doc" target to call Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
  add_custom_target(doc
    ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen" VERBATIM
    )
endif(DOXYGEN_FOUND)

