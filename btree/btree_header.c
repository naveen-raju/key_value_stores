/**
 * @file btree_header.c B+Tree header functions
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

void *
btreeReadHeader(GdbBlock *block, const char *buffer, void *extra)
{
	BTree *tree;
	int counter = 0;

	MEM_CHECK(tree = (BTree *)malloc(sizeof(BTree)));
	memset(tree, 0, sizeof(BTree));

	tree->block = block;
	
	tree->order    = gdbGet8(buffer,  &counter);
	tree->size     = gdbGet32(buffer, &counter);
	tree->root     = gdbGet32(buffer, &counter);
	tree->leftLeaf = gdbGet32(buffer, &counter);

	tree->minLeaf = (tree->order / 2);
	tree->minInt  = ((tree->order + 1) / 2) - 1;

	return tree;
}

void
btreeWriteHeader(GdbBlock *block, char **buffer, unsigned long *size)
{
	int counter = 0;
	BTree *tree;
	
	tree = (BTree *)block->detail;

	*size = BTREE_HEADER_DATA_SIZE;

	MEM_CHECK(*buffer = (char *)malloc(BTREE_HEADER_DATA_SIZE));
	
	gdbPut8(*buffer,  &counter, tree->order);
	gdbPut32(*buffer, &counter, tree->size);
	gdbPut32(*buffer, &counter, tree->root);
	gdbPut32(*buffer, &counter, tree->leftLeaf);
}

void *
btreeCreateHeader(GdbBlock *block, void *extra)
{
	BTree *tree;

	MEM_CHECK(tree = (BTree *)malloc(sizeof(BTree)));
	memset(tree, 0, sizeof(BTree));
	
	tree->block = block;
	tree->order = 5;
	
	tree->minLeaf = (tree->order / 2);
	tree->minInt  = ((tree->order + 1) / 2) - 1;

	return tree;
}

void
btreeDestroyHeader(void *tree)
{
	if (tree == NULL)
		return;

	free(tree);
}

void
btreeSetRootNode(BTree *tree, offset_t offset)
{
	FILE *fp;
	GdbBlock *block;
	
	if (tree == NULL)
		return;

	block = tree->block;
	
	fp = block->db->fp;
	
	tree->root = offset;

	fseek(fp, block->offset + GDB_BLOCK_HEADER_SIZE + BTREE_ROOT_OFFSET,
		  SEEK_SET);
	
	offset = htonl(offset);

	fwrite(&offset, sizeof(offset_t), 1, fp);

	fflush(fp);
}

void
btreeSetLeftLeaf(BTree *tree, offset_t offset)
{
	FILE *fp;
	GdbBlock *block;
	
	if (tree == NULL)
		return;

	block = tree->block;

	fp = block->db->fp;
	
	tree->leftLeaf = offset;

	fseek(fp, block->offset + GDB_BLOCK_HEADER_SIZE + BTREE_LEFT_LEAF_OFFSET,
		  SEEK_SET);

	offset = htonl(offset);

	fwrite(&offset, sizeof(offset_t), 1, fp);

	fflush(fp);
}

void
btreeSetTreeSize(BTree *tree, unsigned long size)
{
	FILE *fp;
	GdbBlock *block;
	
	if (tree == NULL)
		return;

	block = tree->block;

	fp = block->db->fp;
	
	tree->size = size;

	fseek(fp, block->offset + GDB_BLOCK_HEADER_SIZE + BTREE_SIZE_OFFSET,
		  SEEK_SET);

	size = htonl(size);

	fwrite(&size, sizeof(unsigned long), 1, fp);

	fflush(fp);
}

offset_t
btreeGetRootNode(BTree *tree)
{
	FILE *fp;
	GdbBlock *block;
	
	if (tree == NULL)
		return 0;
	
	block = tree->block;

	fp = block->db->fp;
	
	fseek(fp, block->offset + GDB_BLOCK_HEADER_SIZE + BTREE_ROOT_OFFSET,
		  SEEK_SET);

	if (fread(&tree->root, sizeof(offset_t), 1, fp) != 1)
	{
		pmError(PM_ERROR_FATAL,
				_("GNUpdate DB: B+Tree: Unable to read the root node offset "
				  "at %ld in %s, line %d\n"),
				block->offset + GDB_BLOCK_HEADER_SIZE + BTREE_ROOT_OFFSET,
				__FILE__, __LINE__);
		exit(1);
	}

	tree->root = ntohl(tree->root);

	return tree->root;
}

offset_t
btreeGetLeftLeaf(BTree *tree)
{
	FILE *fp;
	GdbBlock *block;
	
	if (tree == NULL)
		return 0;

	block = tree->block;

	fp = block->db->fp;
	
	fseek(fp, block->offset + GDB_BLOCK_HEADER_SIZE + BTREE_LEFT_LEAF_OFFSET,
		  SEEK_SET);

	if (fread(&tree->leftLeaf, sizeof(offset_t), 1, fp) != 1)
	{
		pmError(PM_ERROR_FATAL,
				_("GNUpdate DB: B+Tree: Unable to read the left leaf offset "
				  "at %ld in %s, line %d\n"),
				block->offset + GDB_BLOCK_HEADER_SIZE + BTREE_LEFT_LEAF_OFFSET,
				__FILE__, __LINE__);
		exit(1);
	}

	tree->leftLeaf = ntohl(tree->leftLeaf);

	return tree->leftLeaf;
}

unsigned long
btreeGetTreeSize(BTree *tree)
{
	FILE *fp;
	GdbBlock *block;
	
	if (tree == NULL)
		return 0;

	block = tree->block;

	fp = block->db->fp;
	
	fseek(fp, block->offset + GDB_BLOCK_HEADER_SIZE + BTREE_SIZE_OFFSET,
		  SEEK_SET);

	if (fread(&tree->size, sizeof(unsigned long), 1, fp) != 1)
	{
		pmError(PM_ERROR_FATAL,
				_("GNUpdate DB: B+Tree: Unable to read the tree size at "
				  "offset (%ld) in %s, line %d\n"),
				block->offset + GDB_BLOCK_HEADER_SIZE + BTREE_SIZE_OFFSET,
				__FILE__, __LINE__);
		exit(1);
	}

	tree->size = ntohl(tree->size);

	return tree->size;
}

