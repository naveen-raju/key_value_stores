# $Id: Makefile.am,v 1.35 2004/01/01 08:34:07 chipx86 Exp $

dbdir = $(libdir)/packman/database

SUBDIRS = db

DBSOURCES = \
	addpackage.c \
	get_all_packages.c \
	get_dependencies.c \
	get_files.c \
	gnupdate.c \
	gnupdate.h \
	removepackage.c \
	search.c \
	search_by_file.c \
	search_by_group.c \
	search_by_name.c \
	search_by_provides.c \
	search_by_requires.c \
	tags.h \
	utils.c

DBLIBS = db/libdb.la

AM_CFLAGS = $(st)

libgnupdatedb_s_la_LDFLAGS = -static
libgnupdatedb_la_LDFLAGS   = -no-undefined -module -avoid-version

if STATIC_DB_GNUPDATE
st = -DSTATIC_MODULE

noinst_LTLIBRARIES         = libgnupdatedb_s.la
libgnupdatedb_s_la_SOURCES = $(DBSOURCES)
libgnupdatedb_s_la_LIBADD  = $(DBLIBS)

else
st =

db_LTLIBRARIES           = libgnupdatedb.la
libgnupdatedb_la_SOURCES = $(DBSOURCES)
libgnupdatedb_la_LIBADD  = $(DBLIBS)
endif

INCLUDES = \
	-I$(top_srcdir) \
	$(COMPREX_CFLAGS)
