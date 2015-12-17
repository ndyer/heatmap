AM_CFLAGS = $(MORE_CFLAGS)
AM_CPPFLAGS = $(MORE_CPPFLAGS)
AM_LDFLAGS = $(MORE_LDFLAGS)

bin_PROGRAMS = heatmap
dist_man_MANS = heatmap.8

heatmap_SOURCES  = log.c log.h \
	heatmap.h heatmap.c \
	retrieve.c display.c debugfs.c
heatmap_CFLAGS   = $(AM_CFLAGS)
heatmap_LDFLAGS  = $(AM_LDFLAGS) @CURSES_LIB@
