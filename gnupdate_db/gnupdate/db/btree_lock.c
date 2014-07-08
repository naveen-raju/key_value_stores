/**
 * @file btree_lock.c Locking code
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
#include "db_internal.h"

char
btreeLockNode(BTree *tree, offset_t nodeOffset, GdbLockType type)
{
	if (tree == NULL || nodeOffset < DB_HEADER_BLOCK_SIZE)
		return 0;

	if (type == DB_UNLOCKED)
		return btreeUnlockNode(tree, nodeOffset);
	
	return 1;
}

char
btreeUnlockNode(BTree *tree, offset_t nodeOffset)
{
	return 0;
}

char
btreeLockTree(BTree *tree, GdbLockType type)
{
	if (tree == NULL)
		return 0;

	if (type == DB_UNLOCKED)
		return btreeUnlockTree(tree);

	return 0;
}

char
btreeUnlockTree(BTree *tree)
{
	return 0;
}

GdbLockType
btreeGetNodeLock(BTree *tree, offset_t nodeOffset)
{
	return DB_UNLOCKED;
}

GdbLockType
btreeGetTreeLock(BTree *tree)
{
	return DB_UNLOCKED;
}

