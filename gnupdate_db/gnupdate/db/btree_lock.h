/**
 * @file btree_lock.h Locking code
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
#ifndef _BTREE_LOCK_H_
#define _BTREE_LOCK_H_

#include "btree.h"
#include "db_lock.h"

/**
 * Locks a node.
 * 
 * If the node is already locked, this will wait until it is unlocked
 * before locking and returning.
 * 
 * @param tree       The active B+Tree.
 * @param nodeOffset The offset of the node to lock.
 * @param type       The type of lock.
 *
 * @return 1 on success, 0 on failure.
 */
char btreeLockNode(BTree *tree, offset_t nodeOffset, GdbLockType type);

/**
 * Unlocks a node.
 *
 * @param tree       The active B+Tree.
 * @param nodeOffset The offset of the locked node.
 *
 * @return 1 on success, 0 on failure.
 */
char btreeUnlockNode(BTree *tree, offset_t nodeOffset);

/**
 * Locks the tree.
 *
 * If the tree is already locked, this will wait until it is unlocked
 * before locking and returning.
 *
 * @param tree The B+Tree to lock.
 * @param type The type of lock.
 *
 * @return 1 on success, 0 on failure.
 */
char btreeLockTree(BTree *tree, GdbLockType type);

/**
 * Unlocks the tree.
 *
 * @param tree The B+Tree to unlock.
 *
 * @return 1 on success, 0 on failure.
 */
char btreeUnlockTree(BTree *tree);

/**
 * Returns the current lock on a node.
 *
 * @param tree       The active B+Tree.
 * @param nodeOffset The offset of the node.
 *
 * @return The current lock on the node (or DB_UNLOCKED if none.)
 */
GdbLockType btreeGetNodeLock(BTree *tree, offset_t nodeOffset);

/**
 * Returns the current lock on the tree.
 *
 * @param tree The active B+Tree.
 *
 * @return The current lock on the tree (or DB_UNLOCKED if none.)
 */
GdbLockType btreeGetTreeLock(BTree *tree);

#endif /* _BTREE_LOCK_H_ */

