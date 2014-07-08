/**
 * @file btree_search.c Searching functions
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
__search(BTree *tree, offset_t rootOffset, const char *key, offset_t *filePos)
{
	int i;
	BTreeNode *rootNode;
	char result;

	rootNode = btreeReadNode(tree, rootOffset);
	
	for (i = 0;
		 i < rootNode->keyCount && strcmp(rootNode->keys[i], key) < 0;
		 i++)
		;

	if (BTREE_IS_LEAF(rootNode))
	{
		if (i < rootNode->keyCount && strcmp(rootNode->keys[i], key) == 0)
		{
			*filePos = rootNode->children[i];

			btreeDestroyNode(rootNode);

			return 1;
		}

		btreeDestroyNode(rootNode);
		
		return 0;
	}

	result = __search(tree, rootNode->children[i], key, filePos);

	btreeDestroyNode(rootNode);

	return result;
}

offset_t
btreeSearch(BTree *tree, const char *key)
{
	offset_t filePos;
	char found;
	
	if (tree == NULL || key == NULL)
		return 0;

	filePos = 0;
	found   = 0;

	/* Read in the tree data. */
	tree->root     = btreeGetRootNode(tree);
	tree->leftLeaf = btreeGetLeftLeaf(tree);

	if (btreeIsEmpty(tree) == 1)
		return 0;

	found = __search(tree, tree->root, key, &filePos);

	if (found != 0)
		return filePos;

	return 0;
}

