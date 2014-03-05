plugin_LTLIBRARIES = libgstopenh264.la

noinst_HEADERS = gstopenh264dec.h

libgstopenh264_la_SOURCES = gstopenh264.c \
			gstopenh264dec.c

libgstopenh264_la_CFLAGS = $(GST_PLUGINS_BASE_CFLAGS) $(GST_BASE_CFLAGS) $(GST_CFLAGS) $(OPENH264_CFLAGS)
libgstopenh264_la_LIBADD = \
	$(GST_PLUGINS_BASE_LIBS) \
	-lgstvideo-$(GST_API_VERSION) \
	$(GST_BASE_LIBS) \
	$(GST_LIBS) \
	$(OPENH264_LIBS)
libgstopenh264_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
libgstopenh264_la_LIBTOOLFLAGS = $(GST_PLUGIN_LIBTOOLFLAGS)

