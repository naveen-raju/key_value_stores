/**
 * @file btree_insert.c Insertion functions
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
__splitNode(BTree *tree, BTreeNode *rootNode, char **key,
			offset_t *filePos, char *split, char replaceDup)
{
	char      *temp1, *temp2;
	BTreeNode *tempNode;
	offset_t   offset1 = 0, offset2;
	char       tempSize1, tempSize2;
	int        i, j, div;

	for (i = 0;
		 i < (tree->order - 1) && strcmp(*key, rootNode->keys[i]) > 0;
		 i++)
		;

	if (i < (tree->order - 1) && strcmp(*key, rootNode->keys[i]) == 0)
	{
		if (replaceDup && BTREE_IS_LEAF(rootNode))
		{
			rootNode->children[i] = *filePos;
			GDB_SET_DIRTY(rootNode->block);
			btreeWriteNode(rootNode);
		}

		*split = 0;
		return 0;
	}

	*split = 1;
	
	if (i < (tree->order - 1))
	{
		temp1                 = rootNode->keys[i];
		tempSize1             = rootNode->keySizes[i];
		rootNode->keys[i]     = strdup(*key);
		rootNode->keySizes[i] = strlen(*key) + 1;
		j = i;

		for (i++; i < (tree->order - 1); i++)
		{
			temp2     = rootNode->keys[i];
			tempSize2 = rootNode->keySizes[i];
			
			rootNode->keys[i]     = temp1;
			rootNode->keySizes[i] = tempSize1;
			
			temp1     = temp2;
			tempSize1 = tempSize2;
		}

		if (!BTREE_IS_LEAF(rootNode))
			j++;

		offset1 = rootNode->children[j];
		rootNode->children[j] = *filePos;
		
		for (j++; j <= (tree->order - 1); j++)
		{
			offset2 = rootNode->children[j];
			rootNode->children[j] = offset1;
			offset1 = offset2;
		}
	}
	else
	{
		temp1     = strdup(*key);
		tempSize1 = strlen(temp1) + 1;

		if (BTREE_IS_LEAF(rootNode))
		{
			offset1 = rootNode->children[tree->order - 1];
			rootNode->children[tree->order - 1] = *filePos;
		}
		else
			offset1 = *filePos;
	}

	if (BTREE_IS_LEAF(rootNode))
		div = (int)((tree->order + 1) / 2) - 1;
	else
		div = (int)(tree->order / 2);

	free(*key);
	*key = strdup(rootNode->keys[div]);
	
	tempNode           = btreeNewNode(tree);
	tempNode->keyCount = tree->order - 1 - div;

	if (BTREE_IS_LEAF(rootNode))
		BTREE_SET_LEAF(tempNode);

	i = div + 1;

	for (j = 0; j < tempNode->keyCount - 1; j++, i++)
	{
		tempNode->keys[j]     = rootNode->keys[i];
		tempNode->keySizes[j] = rootNode->keySizes[i];
		tempNode->children[j] = rootNode->children[i];

		rootNode->keys[i]     = NULL;
		rootNode->keySizes[i] = 0;
		rootNode->children[i] = 0;
	}

	tempNode->keys[j]         = temp1;
	tempNode->keySizes[j]     = tempSize1;
	tempNode->children[j]     = rootNode->children[i];
	rootNode->children[i]     = 0;
	tempNode->children[j + 1] = offset1;

	*filePos = btreeWriteNode(tempNode);

	if (BTREE_IS_LEAF(rootNode))
	{
		rootNode->keyCount = div + 1;
		rootNode->children[(int)rootNode->keyCount] = *filePos;
	}
	else
	{
		rootNode->keyCount = div;

		free(rootNode->keys[(int)rootNode->keyCount]);
		rootNode->keys[(int)rootNode->keyCount]     = NULL;
		rootNode->keySizes[(int)rootNode->keyCount] = 0;
	}

	GDB_SET_DIRTY(rootNode->block);
	btreeWriteNode(rootNode);

	btreeDestroyNode(tempNode);

	return 1;
}

static char
__addKey(BTree *tree, BTreeNode *rootNode, char **key, offset_t *filePos,
		 char *split, char replaceDup)
{
	char     *temp1, *temp2;
	offset_t  offset1, offset2;
	char      tempSize1, tempSize2;
	int       i, j;

	*split = 0;

	for (i = 0;
		 i < rootNode->keyCount && strcmp(*key, rootNode->keys[i]) > 0;
		 i++)
		;

	if (i < rootNode->keyCount && strcmp(*key, rootNode->keys[i]) == 0)
	{
		if (replaceDup && BTREE_IS_LEAF(rootNode))
		{
			rootNode->children[i] = *filePos;
			GDB_SET_DIRTY(rootNode->block);
			btreeWriteNode(rootNode);
		}
		
		return 0;
	}

	rootNode->keyCount++;

	if (i < rootNode->keyCount)
	{
		temp1     = rootNode->keys[i];
		tempSize1 = rootNode->keySizes[i];

		rootNode->keys[i]     = strdup(*key);
		rootNode->keySizes[i] = strlen(*key) + 1;
		
		j = i;
		
		for (i++; i < rootNode->keyCount; i++)
		{
			temp2     = rootNode->keys[i];
			tempSize2 = rootNode->keySizes[i];

			rootNode->keys[i]     = temp1;
			rootNode->keySizes[i] = tempSize1;

			temp1     = temp2;
			tempSize1 = tempSize2;
		}

		if (!BTREE_IS_LEAF(rootNode))
			j++;

		offset1 = rootNode->children[j];
		rootNode->children[j] = *filePos;
		
		for (j++; j <= rootNode->keyCount; j++)
		{
			offset2 = rootNode->children[j];
			rootNode->children[j] = offset1;
			offset1 = offset2;
		}
	}
	else
	{
		rootNode->keys[i]     = strdup(*key);
		rootNode->keySizes[i] = strlen(*key) + 1;
	
		if (BTREE_IS_LEAF(rootNode))
		{
			rootNode->children[i + 1] = rootNode->children[i];
			rootNode->children[i]     = *filePos;
		}
		else
			rootNode->children[i + 1] = *filePos;
	}

	GDB_SET_DIRTY(rootNode->block);
	btreeWriteNode(rootNode);

	return 1;
}

static char
__insertKey(BTree *tree, offset_t rootOffset, char **key,
			offset_t *filePos, char *split, char replaceDup)
{
	char success = 0;
	BTreeNode *rootNode;

	if (rootOffset < DB_HEADER_BLOCK_SIZE)
	{
		pmError(PM_ERROR_FATAL,
				_("GNUpdate DB: rootOffset = %ld in __insertKey('%s') in "
				  "%s, line %d\n"),
				rootOffset, *key, __FILE__, __LINE__);
		exit(1);
	}

	rootNode = btreeReadNode(tree, rootOffset);

	if (BTREE_IS_LEAF(rootNode))
	{
		if (rootNode->keyCount < (tree->order - 1))
			success = __addKey(tree, rootNode, key, filePos, split, replaceDup);
		else
			success = __splitNode(tree, rootNode, key, filePos, split,
								  replaceDup);

		btreeDestroyNode(rootNode);

		return success;
	}
	else
	{
		/* Internal node. */
		int i;

		for (i = 0;
			 i < rootNode->keyCount && strcmp(*key, rootNode->keys[i]) > 0;
			 i++)
			;
		
		success = __insertKey(tree, rootNode->children[i], key, filePos,
							  split, replaceDup);
	}

	if (success == 1 && *split == 1)
	{
		if (rootNode->keyCount < (tree->order - 1))
			__addKey(tree, rootNode, key, filePos, split, replaceDup);
		else
			__splitNode(tree, rootNode, key, filePos, split, replaceDup);
	}

	btreeDestroyNode(rootNode);
	
	return success;
}

GdbStatus
btreeInsert(BTree *tree, const char *key, offset_t filePos, char replaceDup)
{
	char  success, split;
	char *newKey;
	
	if (tree == NULL || key == NULL || filePos == 0 ||
		tree->block->db->mode == PM_MODE_READ_ONLY)
	{
		return GDB_ERROR;
	}
	
	if (tree->block->db->mode == PM_MODE_TEST)
	{
		if (!replaceDup && (btreeSearch(tree, key) != 0))
			return GDB_DUPLICATE;

		return GDB_SUCCESS;
	}

	newKey = strdup(key);
	
	success = 0;
	split = 0;

	tree->_insFilePos = filePos;
	
	/* Read in the tree data. */
	tree->root     = btreeGetRootNode(tree);
	tree->leftLeaf = btreeGetLeftLeaf(tree);
	tree->size     = btreeGetTreeSize(tree);

	if (tree->root != 0)
	{
		success = __insertKey(tree, tree->root, &newKey, &tree->_insFilePos,
							  &split, replaceDup);

		if (success == 0)
		{
			free(newKey);
			return (replaceDup ? GDB_SUCCESS : GDB_DUPLICATE);
		}
	}

	btreeSetTreeSize(tree, tree->size + 1);

	if (tree->root == 0 || split == 1)
	{
		BTreeNode *node = btreeNewNode(tree);

		node->keys[0]     = strdup(newKey);
		node->keySizes[0] = strlen(newKey) + 1;
		node->keyCount    = 1;

		if (tree->root == 0)
		{
			node->children[0] = tree->_insFilePos;
			BTREE_SET_LEAF(node);

			btreeWriteNode(node);

			btreeSetLeftLeaf(tree, node->block->offset);
		}
		else
		{
			node->children[0] = tree->root;
			node->children[1] = tree->_insFilePos;

			btreeWriteNode(node);
		}

		btreeSetRootNode(tree, node->block->offset);
		btreeDestroyNode(node);
	}
	
	free(newKey);

	return GDB_SUCCESS;
}

