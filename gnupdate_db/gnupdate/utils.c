/**
 * @file utils.c Utility functions
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
#include "gnupdate.h"
#include "db/db_blocks.h"

#include <sys/timeb.h>

char *
dbPackToString(void *data, size_t size)
{
	char *str;

	MEM_CHECK(str = (char *)malloc(size + 1));
	memcpy(str, data, size);

	str[size - 1] = '\0';

	return str;
}

char *
dbPackTimestamp(void)
{
	struct timeb tp;
	char *str;
	int size;

	ftime(&tp);

	size = sizeof(time_t) + sizeof(unsigned short) + 1;
	
	MEM_CHECK(str = (char *)malloc(size));
	memcpy(str, &tp.time, sizeof(time_t));
	memcpy(str + sizeof(time_t), &tp.millitm, sizeof(unsigned short));

	str[size - 1] = '\0';

	return str;
}

#if 0
static BTree *
__getDbTree(BTree *tree, GdbTag tag)
{
	long offset;
	char *key;

	key = dbPackToString(&tag, sizeof(GdbTag));

	offset = btreeSearch(tree, key);

	free(key);

	if (offset == 0)
		return NULL;

	return btreeOpen(tree->db, offset);
}
#endif

PmPackage *
dbReadPackage(PmDatabase *db, offset_t offset)
{
	GdbHashTable *table;
	PmPackage    *package;
	DbData       *data;
	char         *str;
	long          l;

	data = (DbData *)db->db;

	table = htOpen(data->packageDb, offset);

	if (table == NULL)
	{
		pmError(PM_ERROR_FATAL,
				_("GNUpdate DB: "
				  "Unable to open package table at %ld in %s, line %d\n"),
				offset, __FILE__, __LINE__);
		exit(1);
	}

	package = pmNewPackage();

	PM_PACKAGE_DB_DATA(package) = table;

	if ((str = htGetString(table, GDBTAG_NAME)) != NULL)
	{
		pmSetPackageName(package, str);
		free(str);
	}

	if ((str = htGetString(table, GDBTAG_VERSION)) != NULL)
	{
		pmSetPackageVersion(package, str);
		free(str);
	}
	
	if ((str = htGetString(table, GDBTAG_RELEASE)) != NULL)
	{
		pmSetPackageRelease(package, str);
		free(str);
	}
	
	if ((str = htGetString(table, GDBTAG_ARCH)) != NULL)
	{
		pmSetPackageArch(package, str);
		free(str);
	}

	if ((str = htGetString(table, GDBTAG_BRANCH)) != NULL)
	{
		pmSetPackageBranch(package, str);
		free(str);
	}

	if ((str = htGetString(table, GDBTAG_URL)) != NULL)
	{
		pmSetPackageUrl(package, str);
		free(str);
	}

	if ((str = htGetString(table, GDBTAG_GROUP)) != NULL)
	{
		pmSetPackageGroup(package, str);
		free(str);
	}

	if ((str = htGetString(table, GDBTAG_LICENSE)) != NULL)
	{
		pmSetPackageLicense(package, str);
		free(str);
	}

	if ((str = htGetString(table, GDBTAG_SUMMARY)) != NULL)
	{
		pmSetPackageSummary(package, str);
		free(str);
	}

	if ((str = htGetString(table, GDBTAG_DESCRIPTION)) != NULL)
	{
		pmSetPackageDescription(package, str);
		free(str);
	}

	if ((l = htGetLong(table, GDBTAG_FILE_SIZE)) != 0)
	{
		pmSetPackageFileSize(package, l);
	}
	
	if ((l = htGetLong(table, GDBTAG_INSTALLED_SIZE)) != 0)
	{
		pmSetPackageInstalledSize(package, l);
	}

	return package;
}

