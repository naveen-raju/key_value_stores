/**
 * @file db_blocks.h Blocks management
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
#ifndef _DB_BLOCKS_H_
#define _DB_BLOCKS_H_

/** @name Block types */
/*@{*/
#define GDB_BLOCK_ANY           0x00  /**< Any block type.       */
#define GDB_BLOCK_DATA          0x01  /**< Raw data block.       */
#define GDB_BLOCK_BTREE_HEADER  0x02  /**< B+Tree header block.  */
#define GDB_BLOCK_BTREE_NODE    0x03  /**< B+Tree node block.    */
#define GDB_BLOCK_HASHTABLE     0x04  /**< Hashtable block.      */
#define GDB_BLOCK_OFFSET_LIST   0x05  /**< An offset list block. */

#define GDB_BLOCK_MIN_TYPE  GDB_BLOCK_DATA
#define GDB_BLOCK_MAX_TYPE  GDB_BLOCK_OFFSET_LIST

#define GDB_VALID_BLOCK_TYPE(type) ((type) >= GDB_BLOCK_MIN_TYPE && \
									(type) <= GDB_BLOCK_MAX_TYPE)
/*@}*/

/** @name Block structure definitions */
/*@{*/
#define GDB_BLOCK_TYPE_OFFSET       0 /**< Offset of the type.               */
#define GDB_BLOCK_SIZE_OFFSET       1 /**< Offset of the data size.          */
#define GDB_BLOCK_FLAGS_OFFSET      5 /**< Offset of the flags.              */
#define GDB_BLOCK_NEXT_OFFSET       7 /**< Offset of the continuation block. */
#define GDB_BLOCK_LIST_NEXT_OFFSET 11 /**< Offset of the next linked block.  */

#define GDB_BLOCK_HEADER_SIZE      15 /**< Size of the block header.         */
/*@}*/

/** @name Block flags */
/*@{*/
#define GDB_FLAG_LOCKED 1  /**< Locked block. */
/*@}*/

/** @name Utility macros */
/*@{*/
#define GDB_IS_LOCKED(block) \
	(((block)->flags & (GDB_FLAG_LOCKED)) == GDB_FLAG_LOCKED)

#define GDB_IS_DIRTY(block)    (((block)->dirty) == 1)
#define GDB_SET_DIRTY(block)   (block)->dirty = 1
#define GDB_CLEAR_DIRTY(block) (block)->dirty = 0

#define GDB_SET_FLAG(block, flag)    (block)->flags |= ((flag) << 4)
#define GDB_CLEAR_FLAG(block, flag)  (block)->flags &= ~((flag) << 4)
#define GDB_GET_FLAG(block, flag)    (((block)->flags >> 4) & (flag))

/*@}*/

/**
 * A block of data.
 */
typedef struct
{
	GDatabase *db;           /**< The database the block is part of.  */

	blocktype_t type;        /**< The type of block.                  */

	unsigned short flags;    /**< Flags.                              */
	unsigned long dataSize;  /**< The size of the data.               */

	unsigned short multiple; /**< The block multiple.                 */
	
	offset_t offset;         /**< The offset of the block.            */
	offset_t next;           /**< The next overflow block's offset.   */
	offset_t listNext;       /**< The next block's offset in a list.  */

	offset_t *chain;         /**< The offset chain.                   */
	unsigned int chainCount; /**< The number of blocks in the chain.  */

	void *detail;            /**< The detailed data (BTreeNode, etc.) */

	char dirty;              /**< The dirty state of the block.       */
	char inList;             /**< 1 if in the open blocks list.       */ 
	char refCount;           /**< Reference count.                    */
	
} GdbBlock;

/**
 * Creates a block in memory.
 *
 * If @a blockType is a \c GDB_BLOCK_TREE_NODE, then @a extra must be
 * a BTree structure. Otherwise, it should be @a NULL, but will be
 * ignored anyway.
 *
 * Note that the data type passed to @a extra is not checked, so make
 * sure the right value is being passed, or the database will most likely
 * segfault.
 *
 * @param db        The active database.
 * @param blockType The type of block.
 * @param extra     Block-specific extra data.
 *
 * @return A new block.
 */
GdbBlock *gdbNewBlock(GDatabase *db, blocktype_t blockType, void *extra);

/**
 * Frees up a block in memory.
 *
 * @param block The block to free.
 */
void gdbDestroyBlock(GdbBlock *block);

/**
 * Reads a block's header from disk.
 *
 * @param db        The active database.
 * @param offset    The offset of the block.
 * @param blockType The block type to read.
 *
 * @return The block header, or NULL on error.
 */
GdbBlock *gdbReadBlockHeader(GDatabase *db, offset_t offset,
							 blocktype_t blockType);

/**
 * Writes a block header to disk.
 *
 * @param db    The active database.
 * @param block The block containing the header to write.
 */
void gdbWriteBlockHeader(GdbBlock *block);

/**
 * Reads a block from disk.
 *
 * If @a blockType is a \c GDB_BLOCK_TREE_NODE, then @a extra must be
 * a BTree structure. Otherwise, it should be @a NULL, but will be
 * ignored anyway.
 *
 * Note that the data type passed to @a extra is not checked, so make
 * sure the right value is being passed, or the database will most likely
 * segfault.
 *
 * @param db        The active database.
 * @param offset    The offset of the block.
 * @param blockType The block type to read.
 * @param extra     Block-specific extra data.
 *
 * @return The block, or NULL on error.
 */
GdbBlock *gdbReadBlock(GDatabase *db, offset_t offset, blocktype_t blockType,
					   void *extra);

/**
 * Writes a block to disk.
 *
 * @param db    The active database.
 * @param block The block to write.
 */
void gdbWriteBlock(GdbBlock *block);

/**
 * Determines the block type at the specified offset.
 *
 * @param db     The active database.
 * @param offset The offset of the block.
 *
 * @return The block type at the offset specified.
 */
blocktype_t gdbBlockTypeAt(GDatabase *db, offset_t offset);

/**
 * Reserves a chain of free blocks of the specified type.
 *
 * @param db        The active database.
 * @param count     The number of blocks to reserve.
 * @param blockType The type of block.
 *
 * @return The array of reserved block offsets.
 */
offset_t *gdbReserveBlockChain(GDatabase *db, unsigned short count,
							   blocktype_t blockType);

/**
 * Frees a chain of blocks up so that they can be reclaimed later.
 *
 * @param db        The active database.
 * @param chain     The chain of blocks to free.
 * @param count     The number of blocks in the chain.
 * @param blockType The type of block.
 */
void gdbFreeBlockChain(GDatabase *db, offset_t *chain, unsigned short count,
					   blocktype_t blockType);

/**
 * Reserves a free block of the specified type.
 *
 * @param db        The active database.
 * @param blockType The type of block.
 *
 * @return The offset of the block.
 */
offset_t gdbReserveBlock(GDatabase *db, blocktype_t blockType);

/**
 * Frees a block up so that it can be reclaimed later.
 *
 * @param db        The active database.
 * @param offset    The offset of the block.
 * @param blockType The type of block.
 */
void gdbFreeBlock(GDatabase *db, offset_t offset, blocktype_t blockType);

/**
 * Returnes the number of required blocks to fit the specified amount
 * of data.
 *
 * @param dataSize The size of the data.
 * @param multiple The block multiple.
 *
 * @return The number of needed blocks.
 */
unsigned long gdbGetNeededBlockCount(unsigned long dataSize, short multiple);

#endif /* _DB_BLOCKS_H_ */

