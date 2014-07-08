/**
 * @file db_blocklist.h Free blocks list
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
#ifndef _DB_BLOCKLIST_H_
#define _DB_BLOCKLIST_H_

/**
 * Offset of the free block list.
 */
#define DB_FREE_BLOCK_LIST_OFFSET DB_HEADER_BLOCK_SIZE

/**
 * Size of the free block list.
 */
#define DB_FREE_BLOCK_LIST_SIZE 2048

/**
 * A free block.
 */
typedef struct
{
	unsigned short size; /**< The size of the block    */
	offset_t offset;     /**< The offset of the block. */

} GdbFreeBlock;

/**
 * Returns the free block list.
 *
 * @param db     The active database.
 * @param blocks A pointer to an array of GdbBlocks for holding the blocks.
 * @param count  A pointer to the number of blocks.
 *
 * @return 1 if a free block list is found, or 0 otherwise.
 */
char gdbGetFreeBlockList(GDatabase *db, GdbFreeBlock **blocks, long *count);

/**
 * Writes a block list.
 *
 * @param db     The active database.
 * @param blocks The blocks list.
 * @param count  The number of blocks in the list.
 */
void gdbWriteFreeBlockList(GDatabase *db, GdbFreeBlock *blocks, long count);

/**
 * Frees a block list.
 *
 * @param blocks The block list to free.
 */
void gdbFreeBlockList(GdbFreeBlock *blocks);

#endif /* _DB_BLOCKLIST_H_ */

