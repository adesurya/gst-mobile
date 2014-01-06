outfile=glibconfig.h-tmp

###############################
    cat > $outfile <<\_______EOF
/* glibconfig.h
 *
 * This is a generated file.  Please modify 'configure.ac'
 */

#ifndef __GLIBCONFIG_H__
#define __GLIBCONFIG_H__

#include <glib/gmacros.h>

_______EOF


	if test x$glib_limits_h = xyes; then
	  echo '#include <limits.h>' >> $outfile
	fi
	if test x$glib_float_h = xyes; then
	  echo '#include <float.h>' >> $outfile
	fi
	if test x$glib_values_h = xyes; then
	  echo '#include <values.h>' >> $outfile
	fi
	if test "$glib_header_alloca_h" = "yes"; then
	  echo '#define GLIB_HAVE_ALLOCA_H' >> $outfile
	fi
	if test x$glib_sys_poll_h = xyes; then
	  echo '#define GLIB_HAVE_SYS_POLL_H' >> $outfile
	fi
	if test x$glib_included_printf != xyes; then
          echo "
/* Specifies that GLib's g_print*() functions wrap the
 * system printf functions.  This is useful to know, for example,
 * when using glibc's register_printf_function().
 */" >> $outfile
	  echo '#define GLIB_USING_SYSTEM_PRINTF' >> $outfile
	fi

###############################
	cat >> $outfile <<_______EOF

G_BEGIN_DECLS

#define G_MINFLOAT	$glib_mf
#define G_MAXFLOAT	$glib_Mf
#define G_MINDOUBLE	$glib_md
#define G_MAXDOUBLE	$glib_Md
#define G_MINSHORT	$glib_ms
#define G_MAXSHORT	$glib_Ms
#define G_MAXUSHORT	$glib_Mus
#define G_MININT	$glib_mi
#define G_MAXINT	$glib_Mi
#define G_MAXUINT	$glib_Mui
#define G_MINLONG	$glib_ml
#define G_MAXLONG	$glib_Ml
#define G_MAXULONG	$glib_Mul

_______EOF


###############################
	### this should always be true in a modern C/C++ compiler
	cat >>$outfile <<_______EOF
typedef signed char gint8;
typedef unsigned char guint8;
_______EOF


	if test -n "$gint16"; then
	  cat >>$outfile <<_______EOF
typedef signed $gint16 gint16;
typedef unsigned $gint16 guint16;
#define G_GINT16_MODIFIER $gint16_modifier
#define G_GINT16_FORMAT $gint16_format
#define G_GUINT16_FORMAT $guint16_format
_______EOF
	fi


	if test -n "$gint32"; then
	  cat >>$outfile <<_______EOF
typedef signed $gint32 gint32;
typedef unsigned $gint32 guint32;
#define G_GINT32_MODIFIER $gint32_modifier
#define G_GINT32_FORMAT $gint32_format
#define G_GUINT32_FORMAT $guint32_format
_______EOF
	fi

	cat >>$outfile <<_______EOF
#define G_HAVE_GINT64 1          /* deprecated, always true */

${glib_extension}typedef signed $gint64 gint64;
${glib_extension}typedef unsigned $gint64 guint64;

#define G_GINT64_CONSTANT(val)	$gint64_constant
#define G_GUINT64_CONSTANT(val)	$guint64_constant
_______EOF

	if test x$gint64_format != x ; then
	  cat >>$outfile <<_______EOF
#define G_GINT64_MODIFIER $gint64_modifier
#define G_GINT64_FORMAT $gint64_format
#define G_GUINT64_FORMAT $guint64_format
_______EOF
        else
	  cat >>$outfile <<_______EOF
#undef G_GINT64_MODIFIER
#undef G_GINT64_FORMAT
#undef G_GUINT64_FORMAT
_______EOF
        fi

        cat >>$outfile <<_______EOF

#define GLIB_SIZEOF_VOID_P $glib_void_p
#define GLIB_SIZEOF_LONG   $glib_long
#define GLIB_SIZEOF_SIZE_T $glib_size_t

_______EOF

        cat >>$outfile <<_______EOF
typedef signed $glib_size_type_define gssize;
typedef unsigned $glib_size_type_define gsize;
#define G_GSIZE_MODIFIER $gsize_modifier
#define G_GSSIZE_FORMAT $gssize_format
#define G_GSIZE_FORMAT $gsize_format

#define G_MAXSIZE	G_MAXU$glib_msize_type
#define G_MINSSIZE	G_MIN$glib_msize_type
#define G_MAXSSIZE	G_MAX$glib_msize_type

typedef gint64 goffset;
#define G_MINOFFSET	G_MININT64
#define G_MAXOFFSET	G_MAXINT64

#define G_GOFFSET_MODIFIER      G_GINT64_MODIFIER
#define G_GOFFSET_FORMAT        G_GINT64_FORMAT
#define G_GOFFSET_CONSTANT(val) G_GINT64_CONSTANT(val)

_______EOF

	if test -z "$glib_unknown_void_p"; then
	  cat >>$outfile <<_______EOF

#define GPOINTER_TO_INT(p)	((gint)  ${glib_gpi_cast} (p))
#define GPOINTER_TO_UINT(p)	((guint) ${glib_gpui_cast} (p))

#define GINT_TO_POINTER(i)	((gpointer) ${glib_gpi_cast} (i))
#define GUINT_TO_POINTER(u)	((gpointer) ${glib_gpui_cast} (u))

typedef signed $glib_intptr_type_define gintptr;
typedef unsigned $glib_intptr_type_define guintptr;

#define G_GINTPTR_MODIFIER      $gintptr_modifier
#define G_GINTPTR_FORMAT        $gintptr_format
#define G_GUINTPTR_FORMAT       $guintptr_format
_______EOF
	else
	  echo '#error SIZEOF_VOID_P unknown - This should never happen' >>$outfile
	fi



###############################
	cat >>$outfile <<_______EOF
$glib_atexit
$glib_memmove
$glib_defines
$glib_os
$glib_static_compilation

$glib_vacopy

#ifdef	__cplusplus
#define	G_HAVE_INLINE	1
#else	/* !__cplusplus */
$glib_inline
#endif	/* !__cplusplus */

#ifdef	__cplusplus
#define G_CAN_INLINE	1
_______EOF

	if test x$g_can_inline = xyes ; then
		cat >>$outfile <<_______EOF
#else	/* !__cplusplus */
#define G_CAN_INLINE	1
_______EOF
	fi

	cat >>$outfile <<_______EOF
#endif

_______EOF

	if test x$g_have_iso_c_varargs = xyes ; then
		cat >>$outfile <<_______EOF
#ifndef __cplusplus
# define G_HAVE_ISO_VARARGS 1
#endif
_______EOF
	fi
	if test x$g_have_iso_cxx_varargs = xyes ; then
		cat >>$outfile <<_______EOF
#ifdef __cplusplus
# define G_HAVE_ISO_VARARGS 1
#endif
_______EOF
	fi
	if test x$g_have_gnuc_varargs = xyes ; then
		cat >>$outfile <<_______EOF

/* gcc-2.95.x supports both gnu style and ISO varargs, but if -ansi
 * is passed ISO vararg support is turned off, and there is no work
 * around to turn it on, so we unconditionally turn it off.
 */
#if __GNUC__ == 2 && __GNUC_MINOR__ == 95
#  undef G_HAVE_ISO_VARARGS
#endif

#define G_HAVE_GNUC_VARARGS 1
_______EOF
	fi

	case x$g_stack_grows in
	xyes) echo "#define G_HAVE_GROWING_STACK 1" >>$outfile ;;
	*)    echo "#define G_HAVE_GROWING_STACK 0" >>$outfile ;;
	esac


	echo >>$outfile
	if test x$g_have_eilseq = xno; then
		cat >>$outfile <<_______EOF
#ifndef EILSEQ
/* On some systems, like SunOS and NetBSD, EILSEQ is not defined.
 * The correspondence between this and the corresponding definition
 * in libiconv is essential.
 */
#  define EILSEQ ENOENT
#endif
_______EOF

	fi

	if test x$g_have_gnuc_visibility = xyes; then
		cat >>$outfile <<_______EOF
#define G_HAVE_GNUC_VISIBILITY 1
_______EOF
	fi
		cat >>$outfile <<_______EOF
#if defined(__SUNPRO_C) && (__SUNPRO_C >= 0x590)
#define G_GNUC_INTERNAL __attribute__((visibility("hidden")))
#elif defined(__SUNPRO_C) && (__SUNPRO_C >= 0x550)
#define G_GNUC_INTERNAL __hidden
#elif defined (__GNUC__) && defined (G_HAVE_GNUC_VISIBILITY)
#define G_GNUC_INTERNAL __attribute__((visibility("hidden")))
#else
#define G_GNUC_INTERNAL
#endif
_______EOF

	echo >>$outfile
	cat >>$outfile <<_______EOF
#define G_THREADS_ENABLED
#define G_THREADS_IMPL_$g_threads_impl_def
_______EOF

	if test x"$g_memory_barrier_needed" != xno; then
	  echo >>$outfile
	  echo "#define G_ATOMIC_OP_MEMORY_BARRIER_NEEDED 1" >>$outfile
	fi
	if test x"$g_atomic_lock_free" = xyes; then
          echo >>$outfile
          echo "#define G_ATOMIC_LOCK_FREE" >>$outfile
        fi
	echo >>$outfile
	g_bit_sizes="16 32 64"
	for bits in $g_bit_sizes; do
	  cat >>$outfile <<_______EOF
#define GINT${bits}_TO_${g_bs_native}(val)	((gint${bits}) (val))
#define GUINT${bits}_TO_${g_bs_native}(val)	((guint${bits}) (val))
#define GINT${bits}_TO_${g_bs_alien}(val)	((gint${bits}) GUINT${bits}_SWAP_LE_BE (val))
#define GUINT${bits}_TO_${g_bs_alien}(val)	(GUINT${bits}_SWAP_LE_BE (val))
_______EOF
	done

	cat >>$outfile <<_______EOF
#define GLONG_TO_LE(val)	((glong) GINT${glongbits}_TO_LE (val))
#define GULONG_TO_LE(val)	((gulong) GUINT${glongbits}_TO_LE (val))
#define GLONG_TO_BE(val)	((glong) GINT${glongbits}_TO_BE (val))
#define GULONG_TO_BE(val)	((gulong) GUINT${glongbits}_TO_BE (val))
#define GINT_TO_LE(val)		((gint) GINT${gintbits}_TO_LE (val))
#define GUINT_TO_LE(val)	((guint) GUINT${gintbits}_TO_LE (val))
#define GINT_TO_BE(val)		((gint) GINT${gintbits}_TO_BE (val))
#define GUINT_TO_BE(val)	((guint) GUINT${gintbits}_TO_BE (val))
#define GSIZE_TO_LE(val)	((gsize) GUINT${gsizebits}_TO_LE (val))
#define GSSIZE_TO_LE(val)	((gssize) GINT${gsizebits}_TO_LE (val))
#define GSIZE_TO_BE(val)	((gsize) GUINT${gsizebits}_TO_BE (val))
#define GSSIZE_TO_BE(val)	((gssize) GINT${gsizebits}_TO_BE (val))
#define G_BYTE_ORDER $g_byte_order

#define GLIB_SYSDEF_POLLIN =$g_pollin
#define GLIB_SYSDEF_POLLOUT =$g_pollout
#define GLIB_SYSDEF_POLLPRI =$g_pollpri
#define GLIB_SYSDEF_POLLHUP =$g_pollhup
#define GLIB_SYSDEF_POLLERR =$g_pollerr
#define GLIB_SYSDEF_POLLNVAL =$g_pollnval

#define G_MODULE_SUFFIX "$g_module_suffix"

typedef $g_pid_type GPid;

#define GLIB_SYSDEF_AF_UNIX $g_af_unix
#define GLIB_SYSDEF_AF_INET $g_af_inet
#define GLIB_SYSDEF_AF_INET6 $g_af_inet6

#define GLIB_SYSDEF_MSG_OOB $g_msg_oob
#define GLIB_SYSDEF_MSG_PEEK $g_msg_peek
#define GLIB_SYSDEF_MSG_DONTROUTE $g_msg_dontroute

G_END_DECLS

#endif /* __GLIBCONFIG_H__ */
_______EOF


	if cmp -s $outfile glib/glibconfig.h; then
	  { $as_echo "$as_me:${as_lineno-$LINENO}: glib/glibconfig.h is unchanged" >&5
$as_echo "$as_me: glib/glibconfig.h is unchanged" >&6;}
	  rm -f $outfile
	else
	  mv $outfile glib/glibconfig.h
	fi


