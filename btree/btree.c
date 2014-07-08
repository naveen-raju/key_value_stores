/**
 * @file btree.c B+Tree implementation
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

BTree *
btreeOpen(GDatabase *db, offset_t offset)
{
	GdbBlock *block;

	if (db == NULL || db->fp == NULL || offset < DB_HEADER_BLOCK_SIZE)
		return NULL;

	block = gdbReadBlock(db, offset, GDB_BLOCK_BTREE_HEADER, NULL);

	if (block == NULL)
		return NULL;

	return (BTree *)block->detail;
}

void
btreeClose(BTree *tree)
{
	if (tree == NULL)
		return;

	gdbDestroyBlock(tree->block);
}

BTree *
btreeCreate(GDatabase *db, char order)
{
	GdbBlock *block;

	if (db == NULL || db->fp == NULL)
		return NULL;

	block = gdbNewBlock(db, GDB_BLOCK_BTREE_HEADER, NULL);

	if (block == NULL)
		return NULL;
	
	gdbWriteBlock(block);

	return (BTree *)block->detail;
}

char
btreeIsEmpty(BTree *tree)
{
	if (tree == NULL)
		return 1;

	return (btreeGetSize(tree) == 0);
}

unsigned long
btreeGetSize(BTree *tree)
{
	if (tree == NULL)
		return 0;

	tree->size = btreeGetTreeSize(tree);

	return tree->size;
}
