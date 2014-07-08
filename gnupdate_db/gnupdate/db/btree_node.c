/**
 * @file btree_node.c Node functions
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

static unsigned long
__getNodeSize(BTreeNode *node, const unsigned short *keySizes)
{
	long l, i;

	if (node == NULL)
		return 0;

	if (keySizes == NULL)
		keySizes = node->keySizes;
	
	l = sizeof(char) +
		(node->tree->order * sizeof(offset_t)) +
		((node->tree->order - 1) * sizeof(unsigned short));

	for (i = 0; i < node->tree->order - 1; i++)
		l += keySizes[i];

	return l;
}

static void
__compressNode(BTreeNode *node, char ***newKeys, unsigned short **newKeySizes)
{
	int i;

	if (node == NULL || node->keyCount < 2)
		return;

	MEM_CHECK(*newKeys = (char **)malloc((node->tree->order - 1) *
										 sizeof(char *)));
	memset(*newKeys, 0, (node->tree->order - 1) * sizeof(char *));

	
	MEM_CHECK(*newKeySizes =
			  (unsigned short *)malloc((node->tree->order - 1) *
									   sizeof(unsigned short)));
	memset(*newKeySizes, 0, (node->tree->order - 1) * sizeof(unsigned short));

	(*newKeys)[0]     = strdup(node->keys[0]);
	(*newKeySizes)[0] = node->keySizes[0];
	
	for (i = node->keyCount - 1; i > 0; i--)
	{
		gdbCompressString(node->keys[i - 1], node->keySizes[i - 1],
						  node->keys[i], node->keySizes[i],
						  &(*newKeys)[i], &(*newKeySizes)[i]);
	}
}

static void
__uncompressNode(BTreeNode *node, char ***newKeys,
				 unsigned short **newKeySizes)
{
	int i;
	
	if (node == NULL || node->keyCount < 2)
		return;

	MEM_CHECK(*newKeys = (char **)malloc((node->tree->order - 1) *
										 sizeof(char *)));
	memset(*newKeys, 0, (node->tree->order - 1) * sizeof(char *));


	MEM_CHECK(*newKeySizes =
			  (unsigned short *)malloc((node->tree->order - 1) *
									   sizeof(unsigned short)));
	memset(*newKeySizes, 0, (node->tree->order - 1) * sizeof(unsigned short));
       	
	(*newKeys)[0]     = strdup(node->keys[0]);
	(*newKeySizes)[0] = node->keySizes[0];

	for (i = 1; i <= node->keyCount - 1; i++)
	{
		gdbUncompressString((*newKeys)[i - 1], (*newKeySizes)[i - 1],
							node->keys[i], node->keySizes[i],
							&(*newKeys)[i], &(*newKeySizes)[i]);
	}
}

void *
btreeReadNodeBlock(GdbBlock *block, const char *buffer, void *extra)
{
	BTreeNode *node;
	int i, counter = 0;

	node = btreeCreateNodeBlock(block, extra);

	node->tree  = (BTree *)extra;
	node->block = block;

	node->keyCount = gdbGet8(buffer, &counter);

	for (i = 0; i < node->tree->order; i++)
		node->children[i] = gdbGet32(buffer, &counter);

	for (i = 0; i < node->tree->order - 1; i++)
		node->keySizes[i] = gdbGet16(buffer, &counter);

	for (i = 0; i < node->tree->order - 1; i++)
	{
		if (node->keySizes[i] > 0)
		{
			MEM_CHECK(node->keys[i] = (char *)malloc(node->keySizes[i]));
			memcpy(node->keys[i], buffer + counter, node->keySizes[i]);

			counter += node->keySizes[i];
		}
	}

	if (node->keyCount >= 2)
	{
		char **newKeys;
		unsigned short *newKeySizes;

		__uncompressNode(node, &newKeys, &newKeySizes);

		/* Free up the compressed keys. */
		for (i = 0; i < node->keyCount; i++)
		{
			if (node->keys[i] != NULL)
				free(node->keys[i]);
		}

		free(node->keys);
		free(node->keySizes);

		/* Move over the new arrays. */
		node->keys     = newKeys;
		node->keySizes = newKeySizes;
	}

	return node;
}

void
btreeWriteNodeBlock(GdbBlock *block, char **buffer, unsigned long *size)
{
	BTreeNode *node;
	int i, counter = 0;
	char **newKeys;
	unsigned short *newKeySizes;

	node = (BTreeNode *)block->detail;

	/* Compress the node. */
	if (node->keyCount >= 2)
		__compressNode(node, &newKeys, &newKeySizes);
	else
	{
		newKeys     = node->keys;
		newKeySizes = node->keySizes;
	}
	
	*size = __getNodeSize(node, newKeySizes);

	MEM_CHECK(*buffer = (char *)malloc(*size));
			
	gdbPut8(*buffer, &counter, node->keyCount);

	for (i = 0; i < node->tree->order; i++)
		gdbPut32(*buffer, &counter, node->children[i]);

	for (i = 0; i < node->tree->order - 1; i++)
		gdbPut16(*buffer, &counter, newKeySizes[i]);

	for (i = 0; i < node->tree->order - 1; i++)
	{
		if (newKeySizes[i] > 0)
		{
			memcpy(*buffer + counter, newKeys[i], newKeySizes[i]);

			counter += newKeySizes[i];
		}
	}

	if (node->keyCount >= 2)
	{
		/* Free up the arrays. */
		for (i = 0; i < node->keyCount; i++)
		{
			if (newKeys[i] != NULL)
				free(newKeys[i]);
		}

		free(newKeys);
		free(newKeySizes);
	}
}

void *
btreeCreateNodeBlock(GdbBlock *block, void *extra)
{
	BTreeNode *node;
	BTree *tree;
	
	tree = (BTree *)extra;
	
	MEM_CHECK(node = (BTreeNode *)malloc(sizeof(BTreeNode)));
	memset(node, 0, sizeof(BTreeNode));

	node->tree  = tree;
	node->block = block;

	MEM_CHECK(node->children = (offset_t *)malloc(tree->order *
												  sizeof(offset_t)));
	memset(node->children, 0, tree->order * sizeof(offset_t));
	
	
	MEM_CHECK(node->keySizes =
			  (unsigned short *)malloc((tree->order - 1) *
									   sizeof(unsigned short)));
	memset(node->keySizes, 0, (tree->order -1)  * sizeof(unsigned short));

	
	MEM_CHECK(node->keys = (char **)malloc((tree->order - 1) *
										   sizeof(char *)));
	memset(node->keys, 0, (tree->order - 1) * sizeof(char *));
	
	return node;
}

void
btreeDestroyNodeBlock(void *data)
{
	BTreeNode *node = (BTreeNode *)data;
	int i;
	
	if (node == NULL)
		return;

	for (i = 0; i < node->keyCount; i++)
	{
		if (node->keys[i] != NULL)
			free(node->keys[i]);
	}

	if (GDB_IS_DIRTY(node->block))
	{
		pmError(PM_ERROR_WARNING,
				_("GNUpdate DB: Dirty node at offset %ld has not been "
				  "written to disk.\n"),
				node->block->offset);
	}

	free(node->children);
	free(node->keySizes);
	free(node->keys);

	free(node);
}

BTreeNode *
btreeNewNode(BTree *tree)
{
	GdbBlock *block;
	
	if (tree == NULL)
		return NULL;

	block = gdbNewBlock(tree->block->db, GDB_BLOCK_BTREE_NODE, tree);

	if (block == NULL)
		return NULL;

	/* gdbWriteBlock(block); */

	return (BTreeNode *)block->detail;
}

void
btreeDestroyNode(BTreeNode *node)
{
	if (node == NULL)
		return;

	gdbDestroyBlock(node->block);
}

BTreeNode *
btreeReadNode(BTree *tree, offset_t offset)
{
	GdbBlock *block;
	
	if (tree == NULL || offset < DB_HEADER_BLOCK_SIZE)
		return NULL;

	block = gdbReadBlock(tree->block->db, offset, GDB_BLOCK_BTREE_NODE, tree);

	if (block == NULL)
		return NULL;

	return (BTreeNode *)block->detail;
}

offset_t
btreeWriteNode(BTreeNode *node)
{
	if (node == NULL)
		return 0;

	gdbWriteBlock(node->block);

	return node->block->offset;
}

void
btreeEraseNode(BTreeNode *node)
{
	GdbBlock *block;
	
	if (node == NULL || node->block->offset == 0)
		return;

	block = node->block;
	
	gdbFreeBlock(block->db, block->offset, block->type);

#if 0
	fseek(block->db->fp, block->offset, SEEK_SET);
	gdbPad(block->db->fp, block->size);
#endif
}

