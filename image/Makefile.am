include $(top_srcdir)/config/common.am

INCLUDES = -DMAKING_FORMS $(X_CFLAGS)

include_HEADERS	= flimage.h

lib_LTLIBRARIES = libflimage.la

libflimage_la_LDFLAGS = -no-undefined -version-info @SO_VERSION@

libflimage_la_LIBADD = ../lib/libforms.la $(JPEG_LIB) $(X_LIBS) -lX11

libflimage_la_SOURCES = \
	flimage.h \
	flimage_int.h \
	image.c \
	image_bmp.c \
	image_combine.c \
	image_convolve.c \
	image_crop.c \
	image_disp.c \
	image_fits.c \
	image_genesis.c \
	image_gif.c \
	image_gzip.c \
	image_io_filter.c \
	image_jpeg.c \
	image_jquant.c \
	image_marker.c \
	image_png.c \
	image_pnm.c \
	image_postscript.c \
	image_proc.c \
	image_replace.c \
	image_rotate.c \
	image_scale.c \
	image_sgi.c \
	image_text.c \
	image_tiff.c \
	image_type.c \
	image_warp.c \
	image_xbm.c \
	image_xpm.c \
	image_xwd.c \
	postscript.c \
	ps_core.c \
	ps_draw.c \
	ps_text.c \
	rgb_db.c \
	matrix.c
