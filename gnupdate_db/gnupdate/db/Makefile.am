# $Id: Makefile.am,v 1.11 2002/12/01 06:48:51 chipx86 Exp $

noinst_LTLIBRARIES = libdb.la

libdb_la_SOURCES = \
	btree.c \
	btree.h \
	btree_delete.c \
	btree_lock.c \
	btree_lock.h \
	btree_header.c \
	btree_header.h \
	btree_insert.c \
	btree_node.c \
	btree_node.h \
	btree_search.c \
	btree_traverse.c \
	db.c \
	db.h \
	db_blocks.c \
	db_blocks.h \
	db_blocklist.c \
	db_blocklist.h \
	db_cache.c \
	db_cache.h \
	db_header.c \
	db_header.h \
	db_internal.h \
	db_types.h \
	db_lock.c \
	db_lock.h \
	db_utils.c \
	db_utils.h \
	hashtable.c \
	hashtable.h \
	offsetlist.c \
	offsetlist.h

libdb_la_LDFLAGS = -static

libdb_la_LIBADD = 

INCLUDES = \
	-I$(top_srcdir)
