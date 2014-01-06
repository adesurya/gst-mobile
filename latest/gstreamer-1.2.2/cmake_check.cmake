#CheckFunctionExists
#CheckIncludeFile
#CheckIncludeFileCXX
#CheckIncludeFiles
#CheckLibraryExists
#CheckStructHasMember
#CheckSymbolExists
#CheckTypeSize
#CheckVariableExists


include(CheckFunctionExists)
include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckLibraryExists)
include(CheckSymbolExists)
include(CheckStructHasMember)
include(CheckTypeSize)
include(CheckVariableExists)


#set(AC_APPLE_UNIVERSAL_BUILD) 
#set(BROKEN_POLL)

set(ENABLE_NLS 1)
#set(ENABLE_SUBUNIT)
set(GETTEXT_PACKAGE \"gstreamer-1.0\")
set(GIO_LIBDIR "groot/")
set(GIO_MODULE_DIR "groot/gio/modules")
set(GST_API_VERSION \"1.0\")
set(GST_DATADIR "groot/gstdata")
set(GST_DISABLE_OPTION_PARSING)
set(GST_DISABLE_PARSE)
set(GST_EXTRA_MODULE_SUFFIX)
set(GST_FUNCTION __PRETTY_FUNCTION__)
set(GST_GCOV_ENABLED)
set(GST_HAVE_UNSAFE_FORK)
set(GST_LEVEL_DEFAULT GST_LEVEL_NONE)
set(GST_LICENSE \"LGPL\")
set(GST_PACKAGE_NAME \"GStreamer\ source\ release\")
set(GST_PACKAGE_ORIGIN \"Unknown\ package\ origin\")
set(GST_PACKAGE_RELEASE_DATETIME \"2013-12-26\")
set(GST_PLUGIN_BUILD_STATIC)
set(GST_PLUGIN_SCANNER_INSTALLED \"groot/gstreamer-1.0/gst-plugin-scanner\")

# For MacOS X
check_function_exists(CFLocaleCopyCurrent HAVE_CFLOCALECOPYCURRENT)
check_function_exists(CFPreferencesCopyAppValue HAVE_CFPREFERENCESCOPYAPPVALUE)

check_function_exists(clock_gettime HAVE_CLOCK_GETTIME)

# For CPU
#set(HAVE_CPU_ALPHA)
#set(HAVE_CPU_ARM)
#set(HAVE_CPU_CRIS)
#set(HAVE_CPU_CRISV32)
#set(HAVE_CPU_HPPA)
set(HAVE_CPU_I386 1)
#set(HAVE_CPU_IA64)
#set(HAVE_CPU_M68K)
#set(HAVE_CPU_MIPS)
#set(HAVE_CPU_PPC)
#set(HAVE_CPU_PPC64)
#set(HAVE_CPU_S390)
#set(HAVE_CPU_SPARC)
#set(HAVE_CPU_X86_64)


check_function_exists(dcgettext HAVE_DCGETTEXT)
check_function_exists(dladdr HAVE_DLADDR)
check_include_files(dlfcn.h HAVE_DLFCN_H)
check_function_exists(fgetpos HAVE_FGETPOS)
check_function_exists(fseeko HAVE_FSEEKO)
check_function_exists(fsetpos HAVE_FSETPOS)
check_function_exists(ftello HAVE_FTELLO)

# For __func__ and __FUNCTION__ macros
set(HAVE_FUNC 1)
set(HAVE_FUNCTION 1)

check_function_exists(getpagesize HAVE_GETPAGESIZE)
check_function_exists(gettext HAVE_GETTEXT)

# Have GMP library
#set(HAVE_GMP)

check_function_exists(gmtime_r HAVE_GMTIME_R)

# Have GSL library
#set(HAVE_GSL)

check_function_exists(iconv HAVE_ICONV)

# For type intmax_t
#set(HAVE_INTMAX_T 1)
check_type_size(intmax_t INTMAX_T)

# Define if <inttypes.h> exists, doesn't clash with <sys/types.h>, and declares uintmax_t.
check_include_files(inttypes.h HAVE_INTTYPES_H_WITH_UINTMAX)

check_function_exists(localtime_r HAVE_LOCALTIME_R)

# Define to 1 if the system has the type long long and long long int
#set(HAVE_LONG_LONG 1)
#set(HAVE_LONG_LONG_INT 1)
check_type_size("long long" LONG_LONG)
check_type_size("long long int" LONG_LONG_INT)

check_include_files(memory.h HAVE_MEMORY_H)

check_function_exists(mmap HAVE_MMAP)

# Have a monotonic clock
set(HAVE_MONOTONIC_CLOCK 1)

# Defined if compiling for OSX
#set(HAVE_OSX)

check_function_exists(poll HAVE_POLL)

check_include_files(poll.h HAVE_POLL_H)

check_function_exists(posix_memalign HAVE_POSIX_MEMALIGN)

# Have posix timers
set(HAVE_POSIX_TIMERS 1)

check_function_exists(ppoll HAVE_PPOLL)

# For __PRETTY_FUNCTION__
set(HAVE_PRETTY_FUNCTION 1)

check_include_files(process.h HAVE_PROCESS_H)

check_function_exists(pselect HAVE_PSELECT)

# Define if you have POSIX threads libraries and header files.
#set(HAVE_PTHREAD)
#set(PTHREAD_PRIO_INHERIT)

# For type ptrdiff_t
#set(HAVE_PTRDIFF_T 1)
check_type_size(ptrdiff_t PTRDIFF_T)

set(HAVE_RDTSC 1)

check_function_exists(sigaction HAVE_SIGACTION)

check_include_files(stdint.h HAVE_STDINT_H)

# Define if <stdint.h> exists, doesn't clash with <sys/types.h>, and declares uintmax_t.
check_include_files(stdint.h HAVE_STDINT_H_WITH_UINTMAX)

check_include_files(stdio_ext.h HAVE_STDIO_EXT_H)

check_include_files(stdlib.h HAVE_STDLIB_H)

check_include_files(strings.h HAVE_STRINGS_H)

check_include_files(string.h HAVE_STRING_H)

check_include_files(sys/param.h HAVE_SYS_PARAM_H)

check_include_files(sys/poll.h HAVE_SYS_POLL_H)

check_include_files(sys/prctl.h HAVE_SYS_PRCTL_H)

check_include_files(sys/socket.h HAVE_SYS_SOCKET_H)

check_include_files(sys/stat.h HAVE_SYS_STAT_H)

check_include_files(sys/times.h HAVE_SYS_TIMES_H)

check_include_files(sys/types.h HAVE_SYS_TYPES_H)

check_include_files(sys/utsname.h HAVE_SYS_UTSNAME_H)

check_include_files(sys/wait.h HAVE_SYS_WAIT_H)

check_struct_has_member("struct tm" tm_gmtoff time.h HAVE_TM_GMTOFF)

check_include_files(ucontext.h HAVE_UCONTEXT_H)

# For __uint128_t type
#set(HAVE_UINT128_T)

set(HAVE_UNALIGNED_ACCESS 1)

check_include_files(unistd.h HAVE_UNISTD_H)

# For type: unsigned long long int
#set(HAVE_UNSIGNED_LONG_LONG_INT 1)
check_type_size("unsigned long long int" UNSIGNED_LONG_LONG_INT)

set(HAVE_VALGRIND 1)
check_include_files(valgrind/valgrind.h HAVE_VALGRIND_VALGRIND_H)

# Defined if compiling for Windows
#set(HAVE_WIN32)
if (HAVE_WIN32)
check_include_files(winsock2.h HAVE_WINSOCK2_H)
endif()

set(HOST_CPU "i686")
set(LIBDIR)
set(LOCALEDIR)
set(LT_OBJDIR)

#set(MEMORY_ALIGNMENT)
set(MEMORY_ALIGNMENT_MALLOC 1)
#set(MEMORY_ALIGNMENT_PAGESIZE)

# For package setting
set(PACKAGE \"gstreamer\")
set(PACKAGE_BUGREPORT \"http://bugzilla.gnome.org/enter_bug.cgi?product=GStreamer\")
set(PACKAGE_NAME \"GStreamer\")
set(PACKAGE_STRING \"GStreamer\ 1.2.2\")
set(PACKAGE_TARNAME \"gstreamer\")
set(PACKAGE_URL \"\")
set(PACKAGE_VERSION \"1.2.2\")
set(PLUGINDIR \"groot/gstreamer-1.0\")

#set(PTHREAD_CREATE_JOINABLE)

check_type_size(char SIZEOF_CHAR)
check_type_size(int SIZEOF_INT)
check_type_size(long SIZEOF_LONG)
check_type_size(short SIZEOF_SHORT)
check_type_size(voidp SIZEOF_VOIDP)

set(STDC_HEADERS 1)
set(TARGET_CPU \"i686\")
#set(USE_POISONING)
set(VERSION \"1.2.2\")
#set(WORDS_BIGENDIAN)

# Number of bits in a file offset, on hosts where this is settable.
set(_FILE_OFFSET_BITS 64)
# Define to 1 to make fseeko visible on some hosts (e.g. glibc 2.2).
set(_LARGEFILE_SOURCE)
# Define for large files, on AIX-style hosts.
set(_LARGE_FILES)
# We need at least WinXP SP2 for __stat64
set(__MSVCRT_VERSION__)
# Define to the widest signed integer type if <stdint.h> and <inttypes.h> do not define.
set(intmax_t)
# Define to `unsigned int' if <sys/types.h> does not define. 
set(size_t)


configure_file(config.h.cmake.in config.h)
include_directories(${PROJECT_BINARY_DIR}/include)
