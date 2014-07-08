/**
 * @file main.c B+Tree test program.
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

static void
__print(long filePos)
{
	printf("%ld ", filePos);
}

int
main(int argc, char **argv)
{
	BTree *tree;
	BTree *inTree;
	
	if (argc < 2)
	{
		printf("Specify an operation: create, print\n");
		exit(1);
	}
	
	tree = btreeOpen("test.db");

	if (tree == NULL)
	{
		printf("Unable to open the tree!\n");
		exit(1);
	}

	if (!strcmp(argv[1], "create"))
	{
		btreeInsert(tree, "A",  1);
		btreeInsert(tree, "F",  6);
		btreeInsert(tree, "H",  8);
		btreeInsert(tree, "G",  7);
		btreeInsert(tree, "D",  4);
		btreeInsert(tree, "C",  3);
		btreeInsert(tree, "Z", 26);
		btreeInsert(tree, "O", 15);
		btreeInsert(tree, "W", 23);
		btreeInsert(tree, "M", 13);
		btreeInsert(tree, "Y", 25);
		btreeInsert(tree, "S", 19);
		btreeInsert(tree, "X", 24);
		btreeInsert(tree, "B",  2);
		btreeInsert(tree, "K", 11);
		btreeInsert(tree, "J", 10);
		btreeInsert(tree, "I",  9);
		btreeInsert(tree, "T", 20);
		btreeInsert(tree, "N", 14);
		btreeInsert(tree, "P", 16);
		btreeInsert(tree, "L", 12);
		btreeInsert(tree, "Q", 17);
		btreeInsert(tree, "U", 21);
		btreeInsert(tree, "V", 22);
		btreeInsert(tree, "R", 18);

		/* We'll be adding an internal B+Tree here. */
		inTree = btreeCreateInternal(tree, 3);
		
		if (inTree == NULL)
		{
			printf("Unable to create the internal B+Tree!\n");
			btreeDestroy(tree);
			exit(1);
		}
		
		btreeInsert(tree, "E", inTree->offset);

		/* Populate the internal tree. */
		btreeInsert(inTree, "cat",      1);
		btreeInsert(inTree, "dog",      2);
		btreeInsert(inTree, "bird",     3);
		btreeInsert(inTree, "mouse",    4);
		btreeInsert(inTree, "donkey",   5);
		btreeInsert(inTree, "zebra",    6);
		btreeInsert(inTree, "goldfish", 7);
		
		btreeClose(inTree);
	}
	else if (!strcmp(argv[1], "print"))
	{
		long offset;
		
		btreeTraverse(tree, __print);
	
		printf("\n\n");
		btreePrettyPrint(tree, tree->root, 0);

		offset = btreeSearch(tree, "E");

		printf("\n");
		
		if (offset == 0)
			printf("Unable to find a node with a key of 'E'\n");
		else
		{
			printf("\nInternal tree at %ld:\n", offset);

			inTree = btreeOpenInternal(tree, offset);

			if (inTree == NULL)
				printf("Unable to find the tree!\n");
			else
			{
				btreePrettyPrint(inTree, inTree->root, 0);
				
				btreeClose(inTree);
			}
		}
	}
	else
	{
		printf("Specify an operation: create, print\n");
	}

	btreeClose(tree);

#ifdef WITH_LEAKBUG
	lbDumpLeaks();
#endif

	return 0;
}
