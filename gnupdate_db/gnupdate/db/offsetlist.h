/**
 * @file offsetlist.h A list of offsets.
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
#ifndef _OFFSETLIST_H_
#define _OFFSETLIST_H_

/**
 * An offset list.
 */
typedef struct
{
	GdbBlock *block;          /**< The associated block.              */

	unsigned short arraySize; /**< The size of the offsets array.     */
	unsigned short count;     /**< The number of offsets in the list. */
	offset_t *offsets;        /**< The offsets array.                 */

} GdbOffsetList;

/**
 * Reads an offset list from a buffer.
 *
 * This is meant to be called by the block functions. Don't call this
 * directly.
 *
 * @param block  The block.
 * @param buffer The buffer to read from.
 * @param extra  NULL.
 * 
 * @return A GdbOffsetList, or NULL on error.
 */
void *olReadBlock(GdbBlock *block, const char *buffer, void *extra);

/**
 * Writes an offset list to a buffer.
 *
 * This is meant to be called by the block functions. Don't call this
 * directly.
 *
 * @param block  The block.
 * @param buffer The returned buffer.
 * @param size   The returned buffer size.
 */
void olWriteBlock(GdbBlock *block, char **buffer, unsigned long *size);

/**
 * Creates an offset list.
 *
 * This is meant to be called by the block functions. Don't call this
 * directly.
 *
 * @param block The block.
 * @param extra NULL
 *
 * @return A GdbOffsetList structure.
 */
void *olCreateBlock(GdbBlock *block, void *extra);

/**
 * Destroys a GdbOffsetList structure in memory.
 * 
 * This is meant to be called by the block functions. Don't call this
 * directly.
 *
 * @param data The offset list to destroy.
 */
void olDestroyBlock(void *data);

/**
 * Opens an offset list.
 *
 * @param db     The database.
 * @param offset The offset of the list.
 * 
 * @return The offsets list.
 */
GdbOffsetList *olOpen(GDatabase *db, offset_t offset);

/**
 * Closes an offset list.
 *
 * @param list The GdbOffsetList structure to close.
 */
void olClose(GdbOffsetList *list);

/**
 * Creates an offset list.
 *
 * @param db The database.
 * 
 * @return The offsets list.
 */
GdbOffsetList *olCreate(GDatabase *db);

/**
 * Opens an offset list from inside a database.
 *
 * @param db     The active database.
 * @param offset The offset of the list.
 *
 * @return A GdbOffsetList structure.
 */
GdbOffsetList *olOpen(GDatabase *db, offset_t offset);

/**
 * Adds an offset to the list.
 *
 * @param list   The offsets list.
 * @param offset The offset to add.
 */
void olAddOffset(GdbOffsetList *list, offset_t offset);

/**
 * Returns an offset in the list.
 *
 * @param list  The offset list.
 * @param index The index in the list.
 *
 * @return The offset, or 0 if @a index is out of bounds.
 */
offset_t olGetOffset(GdbOffsetList *list, unsigned short index);

/**
 * Returns the number of offsets in the list.
 *
 * @param list The offset list.
 *
 * @return The number of offsets in the list.
 */
unsigned short olGetCount(GdbOffsetList *list);

#endif /* _OFFSETLIST_H_ */

