/**
 * @file btree.h B+Tree implementation
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
#ifndef _BTREE_H_
#define _BTREE_H_

#include <stdio.h>

typedef struct _BTree          BTree;          /**< A B+Tree.           */
typedef struct _BTreeNode      BTreeNode;      /**< A node in a B+Tree. */
typedef struct _BTreeTraversal BTreeTraversal; /**< A traversal.        */

#include "db.h"

/**
 * A node in the B+Tree.
 */
struct _BTreeNode
{
	BTree *tree;               /**< Parent B+Tree.                     */
	GdbBlock *block;           /**< Parent block.                      */

	char keyCount;             /**< The number of keys in the node.    */

	offset_t *children;        /**< An array of children node offsets. */
	unsigned short *keySizes;  /**< An array of key sizes.             */
	char **keys;               /**< An array of keys.                  */
};

/**
 * A B+Tree.
 */
struct _BTree
{
	GdbBlock *block;         /**< The B+Tree's block.                      */

	unsigned char order;     /**< The order of this tree.                  */
	unsigned long size;      /**< The size of the tree.                    */

	unsigned char minLeaf;   /**< Minimum key count in a leaf              */
	unsigned char minInt;    /**< Minimum key count in an internal node.   */
	
	offset_t root;           /**< The root node's offset.                  */
	offset_t leftLeaf;       /**< The left-most leaf's offset.             */

	offset_t _insFilePos;    /**< Current filePos on inserts. Don't touch! */
};

/**
 * A traversal.
 */
struct _BTreeTraversal
{
	BTree     *tree;       /**< The active B+Tree.               */
	BTreeNode *node;       /**< The current node.                */
	short      pos;        /**< The position of the current key. */
};

/**
 * Opens a B+Tree from inside a database.
 *
 * @param db     The active database.
 * @param offset The offset of the tree.
 *
 * @return A BTree structure.
 */
BTree *btreeOpen(GDatabase *db, offset_t offset);

/**
 * Closes a B+Tree.
 *
 * @param tree The BTree structure to close.
 */
void btreeClose(BTree *tree);

/**
 * Creates a B+Tree inside a database.
 *
 * @param db     The active database.
 * @param order  The order of the tree.
 *
 * @return A BTree structure.
 */
BTree *btreeCreate(GDatabase *db, char order);

/**
 * Inserts an offset to a value with the specified key in a B+Tree.
 *
 * @param tree       The tree to insert into.
 * @param key        The key.
 * @param filePos    The file position containing the data.
 * @param replaceDup Replaces an entry if it already exists.
 *
 * @return The status of the insert operation.
 */
GdbStatus btreeInsert(BTree *tree, const char *key, offset_t filePos,
					  char replaceDup);

/**
 * Deletes a value from a B+Tree.
 *
 * @param tree The tree to delete the value in.
 * @param key  The key associated with the value to delete.
 *
 * @return 1 on success, 0 on failure.
 */
int btreeDelete(BTree *tree, const char *key);

/**
 * Traverses the tree with the specified user-defined function.
 *
 * @param tree    The tree to traverse.
 * @param process The function to call on each value.
 */
void btreeTraverse(BTree *tree, void (*process)(offset_t filePos));

/**
 * Searches the tree for a value with the specified key.
 *
 * @param tree The tree to search.
 * @param key  The associated key.
 *
 * @return The offset data on the node if found, or 0 if not found.
 */
offset_t btreeSearch(BTree *tree, const char *key);

/**
 * Returns whether or not the tree is empty.
 *
 * @param tree The tree.
 *
 * @return 1 if empty, 0 otherwise.
 */
char btreeIsEmpty(BTree *tree);

/**
 * Returns the size of the tree.
 *
 * @param tree The tree.
 *
 * @return The size of the tree.
 */
unsigned long btreeGetSize(BTree *tree);

/**
 * Pretty-prints the tree (for debugging purposes).
 *
 * @param tree       The tree to print.
 * @param rootOffset The root node offset.
 * @param i          The current indent level.
 */
void btreePrettyPrint(BTree *tree, offset_t rootOffset, int i);

/**
 * Prepares a traversal.
 *
 * @param tree The tree.
 *
 * @return A BTreeTraversal structure.
 */
BTreeTraversal *btreeInitTraversal(BTree *tree);

/**
 * Destroys a traversal.
 *
 * @param trav The traversal.
 *
 * @return NULL.
 */
BTreeTraversal *btreeDestroyTraversal(BTreeTraversal *trav);

/**
 * Returns the first offset in a traversal.
 *
 * @param trav The active traversal.
 *
 * @return The first offset, or -1 if empty.
 */
offset_t btreeGetFirstOffset(BTreeTraversal *trav);

/**
 * Returns the next offset in a traversal.
 *
 * @param trav The active traversal.
 *
 * @return The next offset, or -1 when done.
 */
offset_t btreeGetNextOffset(BTreeTraversal *trav);

#endif /* _BTREE_H_ */

