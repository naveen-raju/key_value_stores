/**
 * @file btree_delete.c Deletion functions
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

static char
__removeKey(BTree *tree, BTreeNode *rootNode, const char *key,
			offset_t *filePos)
{
	int i;
	
	for (i = 0;
		 i < rootNode->keyCount && strcmp(rootNode->keys[i], key) < 0;
		 i++)
		;

	if (BTREE_IS_LEAF(rootNode) && i < rootNode->keyCount &&
		strcmp(rootNode->keys[i], key) == 0)
	{
		*filePos = rootNode->children[i];

		free(rootNode->keys[i]);

		for (; i < rootNode->keyCount - 1; i++)
		{
			rootNode->keys[i]     = rootNode->keys[i + 1];
			rootNode->keySizes[i] = rootNode->keySizes[i + 1];
			rootNode->children[i] = rootNode->children[i + 1];
		}

		rootNode->keys[i]         = NULL;
		rootNode->keySizes[i]     = 0;
		rootNode->children[i]     = rootNode->children[i + 1];
		rootNode->children[i + 1] = 0;

		rootNode->keyCount--;

		GDB_SET_DIRTY(rootNode->block);

		btreeWriteNode(rootNode);

		return 1;
	}

	return 0;
}

static void
__removeKey2(BTree *tree, BTreeNode *rootNode, int index)
{
	int i;

	free(rootNode->keys[index]);
	
	for (i = index; i < rootNode->keyCount - 1; i++)
	{
		rootNode->keys[i]     = rootNode->keys[i + 1];
		rootNode->keySizes[i] = rootNode->keySizes[i + 1];
		rootNode->children[i] = rootNode->children[i + 1];
	}

	rootNode->keys[i]         = NULL;
	rootNode->keySizes[i]     = 0;
	rootNode->children[i]     = rootNode->children[i + 1];
	rootNode->children[i + 1] = 0;

	rootNode->keyCount--;

	GDB_SET_DIRTY(rootNode->block);

	btreeWriteNode(rootNode);
}

static char
__borrowRight(BTree *tree, BTreeNode *rootNode, BTreeNode *prevNode, int div)
{
	BTreeNode *node;

	if (div >= prevNode->keyCount)
		return 0;

	node = btreeReadNode(tree, prevNode->children[div + 1]);

	if (BTREE_IS_LEAF(node) && node->keyCount > tree->minLeaf)
	{
		rootNode->children[rootNode->keyCount + 1] =
			rootNode->children[(int)rootNode->keyCount];
		
		free(rootNode->keys[(int)rootNode->keyCount]);
		rootNode->keys[(int)rootNode->keyCount]     = strdup(node->keys[0]);
		rootNode->keySizes[(int)rootNode->keyCount] = node->keySizes[0];
		rootNode->children[(int)rootNode->keyCount] = node->children[0];

		free(rootNode->keys[div]);
		prevNode->keys[div] = strdup(rootNode->keys[(int)rootNode->keyCount]);
		prevNode->keySizes[div] = rootNode->keySizes[(int)rootNode->keyCount];
	}
	else if (!BTREE_IS_LEAF(node) && node->keyCount > tree->minInt)
	{
		free(rootNode->keys[(int)rootNode->keyCount]);
		rootNode->keys[(int)rootNode->keyCount] = strdup(prevNode->keys[div]);
		rootNode->keySizes[(int)rootNode->keyCount] = prevNode->keySizes[div];

		free(rootNode->keys[div]);
		prevNode->keys[div]                         = strdup(node->keys[0]);
		prevNode->keySizes[div]                     = node->keySizes[0];

		rootNode->children[rootNode->keyCount + 1] = node->children[0];
	}
	else
	{
		btreeDestroyNode(node);

		return 0;
	}

	GDB_SET_DIRTY(rootNode->block);
	GDB_SET_DIRTY(prevNode->block);

	rootNode->keyCount++;

	__removeKey2(tree, node, 0);

	btreeDestroyNode(node);
	
	return 1;
}

static char
__borrowLeft(BTree *tree, BTreeNode *rootNode, BTreeNode *prevNode, int div)
{
	int i;
	BTreeNode *node;

	if (div == 0)
		return 0;

	node = btreeReadNode(tree, prevNode->children[div - 1]);

	if (BTREE_IS_LEAF(node) && node->keyCount > tree->minLeaf)
	{
		for (i = rootNode->keyCount; i > 0; i--)
		{
			rootNode->keys[i]         = rootNode->keys[i - 1];
			rootNode->keySizes[i]     = rootNode->keySizes[i - 1];
			rootNode->children[i + 1] = rootNode->children[i];
		}

		rootNode->children[1] = rootNode->children[0];
		rootNode->keys[0]     = strdup(node->keys[node->keyCount - 1]);
		rootNode->keySizes[0] = node->keySizes[node->keyCount - 1];
		rootNode->children[0] = node->children[node->keyCount - 1];

		rootNode->keyCount++;

		free(prevNode->keys[div - 1]);
		prevNode->keys[div - 1]     = strdup(node->keys[node->keyCount - 2]);
		prevNode->keySizes[div - 1] = node->keySizes[node->keyCount - 2];

		node->children[node->keyCount - 1] =
			node->children[(int)node->keyCount];

		node->children[(int)node->keyCount] = 0;

		free(node->keys[node->keyCount - 1]);
		node->keys[node->keyCount - 1]     = NULL;
		node->keySizes[node->keyCount - 1] = 0;
	}
	else if (!BTREE_IS_LEAF(node) && node->keyCount > tree->minInt)
	{
		for (i = rootNode->keyCount; i > 0; i--)
		{
			rootNode->keys[i]         = rootNode->keys[i - 1];
			rootNode->keySizes[i]     = rootNode->keySizes[i - 1];
			rootNode->children[i + 1] = rootNode->children[i];
		}

		rootNode->children[1] = rootNode->children[0];
		rootNode->keys[0]     = strdup(prevNode->keys[div - 1]);
		rootNode->keySizes[0] = prevNode->keySizes[div - 1];
		rootNode->children[0] = node->children[(int)node->keyCount];

		rootNode->keyCount++;

		free(prevNode->keys[div - 1]);
		prevNode->keys[div - 1]     = strdup(node->keys[node->keyCount - 1]);
		prevNode->keySizes[div - 1] = node->keySizes[node->keyCount - 1];
		
		node->children[(int)node->keyCount] = 0;

		free(node->keys[node->keyCount - 1]);
		node->keys[node->keyCount - 1]     = NULL;
		node->keySizes[node->keyCount - 1] = 0;
	}
	else
	{
		btreeDestroyNode(node);
		
		return 0;
	}

	node->keyCount--;

	GDB_SET_DIRTY(rootNode->block);
	GDB_SET_DIRTY(prevNode->block);
	GDB_SET_DIRTY(node->block);

	btreeWriteNode(node);
	btreeDestroyNode(node);

	return 1;
}

static char
__mergeNode(BTree *tree, BTreeNode *rootNode, BTreeNode *prevNode, int div)
{
	int i, j;
	BTreeNode *node;

	/* Try to merge the node with its left sibling. */
	if (div > 0)
	{
		node = btreeReadNode(tree, prevNode->children[div - 1]);
		i    = node->keyCount;

		if (!BTREE_IS_LEAF(rootNode))
		{
			free(node->keys[i]);
			
			node->keys[i]     = strdup(prevNode->keys[div - 1]);
			node->keySizes[i] = prevNode->keySizes[div - 1];
			node->keyCount++;
			
			i++;
		}

		for (j = 0; j < rootNode->keyCount; j++, i++)
		{
			free(node->keys[i]);

			node->keys[i]     = strdup(rootNode->keys[j]);
			node->keySizes[i] = rootNode->keySizes[j];
			node->children[i] = rootNode->children[j];
			node->keyCount++;
		}

		node->children[i] = rootNode->children[j];

		GDB_SET_DIRTY(node->block);

		btreeWriteNode(node);
		
		prevNode->children[div] = node->block->offset;

		GDB_SET_DIRTY(prevNode->block);

		btreeEraseNode(rootNode);
		__removeKey2(tree, prevNode, div - 1);
	}
	else
	{
		/* Must merge the node with its right sibling. */
		node = btreeReadNode(tree, prevNode->children[div + 1]);
		i    = rootNode->keyCount;

		if (!BTREE_IS_LEAF(rootNode))
		{
			free(node->keys[i]);
			
			rootNode->keys[i]     = strdup(prevNode->keys[div]);
			rootNode->keySizes[i] = prevNode->keySizes[div];
			rootNode->keyCount++;
			
			i++;
		}

		for (j = 0; j < node->keyCount; j++, i++)
		{
			free(node->keys[i]);
			rootNode->keys[i]     = strdup(node->keys[j]);
			rootNode->keySizes[i] = node->keySizes[j];
			rootNode->children[i] = node->children[j];
			rootNode->keyCount++;
		}
		
		rootNode->children[i]       = node->children[j];
		prevNode->children[div + 1] = rootNode->block->offset;

		GDB_SET_DIRTY(rootNode->block);
		GDB_SET_DIRTY(prevNode->block);

		btreeEraseNode(node);

		__removeKey2(tree, prevNode, div);
	}

	btreeWriteNode(node);
	btreeWriteNode(prevNode);
	btreeWriteNode(rootNode);

	btreeDestroyNode(node);

	return 1;
}

static char
__delete(BTree *tree, offset_t rootOffset, BTreeNode *prevNode,
		 const char *key, int index, offset_t *filePos, char *merged)
{
	char success = 0;
	BTreeNode *rootNode;

	rootNode = btreeReadNode(tree, rootOffset);

	if (BTREE_IS_LEAF(rootNode))
	{
		success = __removeKey(tree, rootNode, key, filePos);
	}
	else
	{
		int i;
		
		for (i = 0;
			 i < rootNode->keyCount && strcmp(rootNode->keys[i], key) < 0;
			 i++)
			;

		success = __delete(tree, rootNode->children[i], rootNode, key, i,
						   filePos, merged);
	}

	if (success == 0)
	{
		btreeDestroyNode(rootNode);
		
		return 0;
	}
	else if ((rootNode->block->offset == tree->root) ||
			 (BTREE_IS_LEAF(rootNode)  && rootNode->keyCount >= tree->minLeaf) ||
			 (!BTREE_IS_LEAF(rootNode) && rootNode->keyCount >= tree->minInt))
	{
		btreeDestroyNode(rootNode);
		
		return 1;
	}
	else
	{
		if (__borrowRight(tree, rootNode, prevNode, index) ||
			__borrowLeft(tree, rootNode, prevNode, index))
		{
			*merged = 0;
		}
		else
		{
			*merged = 1;
			__mergeNode(tree, rootNode, prevNode, index);
		}

		btreeWriteNode(rootNode);
		btreeWriteNode(prevNode);
	}

	btreeDestroyNode(rootNode);
	
	return 1;
}

int
btreeDelete(BTree *tree, const char *key)
{
	int i;
	offset_t filePos;
	char merged, success;
	BTreeNode *rootNode;

	if (tree == NULL || key == NULL ||
		tree->block->db->mode == PM_MODE_READ_ONLY)
	{
		return 0;
	}

	if (tree->block->db->mode == PM_MODE_TEST)
	{
		return btreeSearch(tree, key);
	}

	filePos = 0;
	merged  = 0;
	success = 0;

	/* Read in the tree data. */
	tree->root     = btreeGetRootNode(tree);
	tree->leftLeaf = btreeGetLeftLeaf(tree);
	tree->size     = btreeGetTreeSize(tree);

	/* Read in the root node. */
	rootNode = btreeReadNode(tree, tree->root);
	
	for (i = 0;
		 i < rootNode->keyCount && strcmp(rootNode->keys[i], key) < 0;
		 i++)
		;

	success = __delete(tree, tree->root, NULL, key, i, &filePos, &merged);

	if (success == 0)
	{
		btreeDestroyNode(rootNode);
		return 0;
	}
	
	btreeSetTreeSize(tree, tree->size - 1);

	if (BTREE_IS_LEAF(rootNode) && rootNode->keyCount == 0)
	{
		btreeSetRootNode(tree, 0);
		btreeEraseNode(rootNode);
	}
	else if (merged == 1 && rootNode->keyCount == 0)
	{
		BTreeNode *tempNode;
		
		btreeSetRootNode(tree, rootNode->children[0]);

		tempNode = btreeReadNode(tree, tree->root);

		GDB_SET_DIRTY(tempNode->block);

		btreeWriteNode(tempNode);
		btreeDestroyNode(tempNode);

		btreeEraseNode(rootNode);
	}

	btreeDestroyNode(rootNode);

	return filePos;
}

