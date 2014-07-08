/**
 * @file btree_header.h B+Tree header functions
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
#ifndef _BTREE_HEADER_H_
#define _BTREE_HEADER_H_

#include <stdio.h>

#include "db_blocks.h"

#define BTREE_HEADER_DATA_SIZE   13   /**< Header data size.               */

/** @name B+Tree header offsets */
/*@{*/
#define BTREE_ORDER_OFFSET         0  /**< B+Tree order.                   */
#define BTREE_SIZE_OFFSET          1  /**< Size of the B+Tree.             */
#define BTREE_ROOT_OFFSET          5  /**< Offset of the root node.        */
#define BTREE_LEFT_LEAF_OFFSET     9  /**< Offset of the left-most leaf.   */
/*@}*/

/**
 * Reads a B+Tree header from a buffer.
 *
 * This is meant to be called by the block functions. Don't call this
 * directly.
 *
 * @param block  The block.
 * @param buffer The buffer to read from.
 * @param extra  NULL.
 * 
 * @return A BTree, or NULL on error.
 */
void *btreeReadHeader(GdbBlock *block, const char *buffer, void *extra);

/**
 * Writes a B+Tree header to a buffer.
 *
 * This is meant to be called by the block functions. Don't call this
 * directly.
 *
 * @param block  The block.
 * @param buffer The returned buffer.
 * @param size   The returned buffer size.
 */
void btreeWriteHeader(GdbBlock *block, char **buffer, unsigned long *size);

/**
 * Creates a B+Tree header.
 *
 * This is meant to be called by the block functions. Don't call this
 * directly.
 *
 * @param block The block.
 * @param extra NULL
 *
 * @return A BTree structure.
 */
void *btreeCreateHeader(GdbBlock *block, void *extra);

/**
 * Destroys a BTree structure in memory.
 * 
 * This is meant to be called by the block functions. Don't call this
 * directly.
 *
 * @param tree The tree to destroy.
 */
void btreeDestroyHeader(void *tree);

/**
 * Sets the root node offset in the header.
 *
 * @param tree   The active B+Tree.
 * @param offset The offset of the root node.
 *
 * @see btreeGetRootNode();
 */
void btreeSetRootNode(BTree *tree, offset_t offset);

/**
 * Sets the left-most leaf's offset in the header.
 *
 * @param tree   The active B+Tree.
 * @param offset The offset of the left-most leaf.
 *
 * @see btreeGetLeftLeaf()
 */
void btreeSetLeftLeaf(BTree *tree, offset_t offset);

/**
 * Sets the size of the B+Tree in the header.
 * 
 * @param tree The active B+Tree.
 * @param size The size of the B+Tree.
 *
 * @see btreeGetTreeSize()
 */
void btreeSetTreeSize(BTree *tree, unsigned long size);

/**
 * Returns the root node offset in the header.
 *
 * @param tree The active B+Tree.
 *
 * @return The root node offset.
 *
 * @see btreeSetRootNode()
 */
offset_t btreeGetRootNode(BTree *tree);

/**
 * Returns the left-most leaf's offset in the header.
 *
 * @param tree The active B+Tree.
 *
 * @return The left-most leaf's offset.
 *
 * @see btreeSetLeftLeaf()
 */
offset_t btreeGetLeftLeaf(BTree *Tree);

/**
 * Returns the size of the B+Tree in the header.
 *
 * @param tree The active B+Tree.
 *
 * @return The tree's size.
 *
 * @see btreeSetTreeSize()
 */
unsigned long btreeGetTreeSize(BTree *tree);

#endif /* _BTREE_HEADER_H_ */
