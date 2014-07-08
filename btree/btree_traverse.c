/**
 * @file btree_traverse.c Traversal functions
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

void
btreeTraverse(BTree *tree, void (*process)(offset_t filePos))
{
	BTreeTraversal *trav;
	offset_t offset;
	
	if (tree == NULL || process == NULL)
		return;

	trav = btreeInitTraversal(tree);

	for (offset = btreeGetFirstOffset(trav);
		 offset != -1;
		 offset = btreeGetNextOffset(trav))
	{
		process(offset);
	}

	btreeDestroyTraversal(trav);
}

BTreeTraversal *
btreeInitTraversal(BTree *tree)
{
	BTreeTraversal *trav;

	if (tree == NULL)
		return NULL;

	MEM_CHECK(trav = (BTreeTraversal *)malloc(sizeof(BTreeTraversal)));
	memset(trav, 0, sizeof(BTreeTraversal));

	trav->tree = tree;

	return trav;
}

BTreeTraversal *
btreeDestroyTraversal(BTreeTraversal *trav)
{
	if (trav == NULL)
		return NULL;

	if (trav->node != NULL)
		btreeDestroyNode(trav->node);

	free(trav);

	return NULL;
}

offset_t
btreeGetFirstOffset(BTreeTraversal *trav)
{
	if (trav == NULL)
		return -1;

	if (trav->node != NULL)
		return btreeGetNextOffset(trav);

	trav->tree->leftLeaf = btreeGetLeftLeaf(trav->tree);

	trav->node = btreeReadNode(trav->tree, trav->tree->leftLeaf);

	if (trav->node == NULL)
		return -1;

	trav->pos = 1;

	return trav->node->children[0];
}

offset_t
btreeGetNextOffset(BTreeTraversal *trav)
{
	offset_t offset;
	
	if (trav == NULL)
		return -1;

	if (trav->node == NULL)
		return btreeGetNextOffset(trav);

	if (trav->pos == trav->node->keyCount)
	{
		offset_t nextNodeOffset = trav->node->children[trav->pos];
		
		btreeDestroyNode(trav->node);

		trav->node = NULL;

		if (nextNodeOffset == 0)
			return -1;
		
		trav->node = btreeReadNode(trav->tree, nextNodeOffset);

		trav->pos = 0;
	}

	offset = trav->node->children[trav->pos];

	trav->pos++;

	return offset;
}

void
btreePrettyPrint(BTree *tree, offset_t rootOffset, int i)
{
	int j;
	BTreeNode *rootNode;
	
	rootNode = btreeReadNode(tree, rootOffset);

	if (rootNode == NULL)
	{
		pmError(PM_ERROR_FATAL,
				_("GNUpdate DB: rootNode (%ld) is NULL in %s, line %d\n"),
				rootOffset, __FILE__, __LINE__);
		exit(1);
	}

	for (j = i; j > 0; j--)
		printf("    ");

	printf("[.");

	for (j = 0; j < rootNode->keyCount; j++)
		printf(" %s .", rootNode->keys[j]);

	for (j = tree->order - rootNode->keyCount; j > 1; j--)
		printf(" _____ .");
	
	printf("] - %ld\n", rootOffset);

	if (BTREE_IS_LEAF(rootNode))
	{
		btreeDestroyNode(rootNode);
		return;
	}
	
	for (j = 0; j <= rootNode->keyCount; j++)
		btreePrettyPrint(tree, rootNode->children[j], i + 1);

	btreeDestroyNode(rootNode);
}
