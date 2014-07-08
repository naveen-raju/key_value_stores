/**
 * @file db.h Main GNUpdate database include file
 *
 * @Copyright (C) 1999-2004 The GNUpdate Project.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA  02111-1307, USA.
 */
#ifndef _GNUPDATEDB_DB_H_
#define _GNUPDATEDB_DB_H_

#include <libpackman/types.h>
#include <libpackman/error.h>
#include <libcomprex/debug.h>

#undef fseek
#define fseek(a,b,c) \
	cxDebug(CX_DEBUG_MISC, "db", "Seeking to %ld (%d)\n", (b), (c)); \
	fseek(a, b, c)

typedef struct _GDatabase GDatabase;   /**< GNUpdate database. */

/**
 * Database types.
 */
typedef enum
{
	GDB_INDEX_FILE = 0x00,  /**< Index file (for searching).   */
	GDB_DATA_FILE  = 0x01   /**< Data file (for storing data). */

} GdbType;

/**
 * Status types.
 */
typedef enum
{
	GDB_SUCCESS,    /**< Success.         */
	GDB_DUPLICATE,  /**< Duplicate entry. */
	GDB_ERROR       /**< Error.           */

} GdbStatus;


#include "db_types.h"
#include "db_blocks.h"
#include "btree.h"
#include "hashtable.h"
#include "offsetlist.h"


/**
 * GNUpdate database.
 */
struct _GDatabase
{
	PmAccessMode mode;      /**< Access mode.                    */

	char *filename;         /**< Filename of the database.       */
	FILE *fp;               /**< Active file pointer.            */
	
	GdbType type;           /**< Database type.                  */

	long freeBlockCount;    /**< Number of free blocks.          */

	BTree *mainTree;        /**< Main B+Tree.                    */

	int openBlockCount;     /**< Number of open blocks.          */
	int openBlockSize;      /**< Size of the open blocks array.  */
	GdbBlock **openBlocks;  /**< Open blocks array.              */
};

/**
 * Opens a database from a file.
 *
 * If the file does not exist, it will be created through a call to
 * gdbCreate().
 *
 * If a type is specified that does not match the type of database, this
 * will return NULL.
 *
 * @param filename The name of the database file.
 * @param type     The type of database to open.
 * @param mode     The access mode.
 *
 * @return A GDatabase structure.
 */
GDatabase *gdbOpen(const char *filename, GdbType type, PmAccessMode mode);

/**
 * Closes a database.
 *
 * @param db The database to close.
 */
void gdbClose(GDatabase *db);

/**
 * Creates a database.
 *
 * gdbOpen() automatically calls this if the specified file does not
 * exist. Calling this instead of gdbOpen() will overwrite an existing
 * file.
 *
 * @param filename The name of the file to store the database in.
 * @param type     The type of database to create.
 *
 * @return A GDatabase structure.
 */
GDatabase *gdbCreate(const char *filename, GdbType type);

/**
 * Destroys a GDatabase structure in memory.
 *
 * Calling gdbClose() will automatically call this to free up the
 * memory for the database.
 *
 * @param db The database structure to destroy.
 *
 * @return NULL
 */
GDatabase *gdbDestroy(GDatabase *db);

/**
 * Adds a data entry to the database with the specified key.
 *
 * @param db    The active database.
 * @param table The hashtable to add to.
 * @param key   The key to associate with.
 * @param data  The data to add to the database.
 * @param size  The size of the data.
 *
 * @return The status of the operation.
 */
GdbStatus gdbAddDataEntry(GDatabase *db, GdbHashTable *table,
						  unsigned short key, const void *data, long size);

/**
 * Adds an index entry to the database with the specified key.
 *
 * @param db     The active database.
 * @param tree   The tree to add to.
 * @param key    The key to associate with.
 * @param offset The offset to store.
 *
 * @return The status of the operation.
 */
GdbStatus gdbAddIndexEntry(GDatabase *db, BTree *tree, const char *key,
						   offset_t offset);

/**
 * Adds an internal B+Tree with the specified key.
 *
 * @param db      The active database.
 * @param tree    The tree to add to.
 * @param key     The key to associate with.
 * @param newTree The resulting tree.
 *
 * @return The status of the operation.
 */
GdbStatus gdbAddTree(GDatabase *db, BTree *tree, const char *key,
					 BTree **newTree);

/**
 * Adds a hashtable to the database.
 *
 * @param db       The active database.
 * @param tree     The tree to add to.
 * @param key      The key to associate with.
 * @param newTable The resulting hashtable.
 *
 * @return The status of the operation.
 */
GdbStatus gdbAddHashTable(GDatabase *db, BTree *tree, const char *key,
						  GdbHashTable **newTable);

#endif /* _GNUPDATEDB_DB_H_ */
