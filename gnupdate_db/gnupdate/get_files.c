/**
 * @file get_files.c Returns a list of files from a package in the database.
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

static char *
__decompressMd5(const char *md5)
{
	char newMd5[33];
	int i, j;

	if (md5 == NULL)
		return NULL;

	for (i = 0, j = 0; i < 16; i++, j += 2)
	{
		newMd5[j]     = ((newMd5[i] >> 4) & 0x0F);
		newMd5[j + 1] = ((newMd5[j]       & 0x0F));
	}

	newMd5[32] = '\0';

	return strdup(newMd5);
}

void
dbGetFiles(PmDatabase *db, PmPackage *pkg)
{
	GdbHashTable *table;
	offset_t      filesOffset, nextOffset;
	DbData       *data;
	char         *str;
	long          l;
	CxArchive    *archive;
	CxDirectory  *root;

	table = (GdbHashTable *)PM_PACKAGE_DB_DATA(pkg);

	if (table == NULL)
	{
		pmError(PM_ERROR_WARNING,
				_("GNUpdate DB: "
				  "dbData == NULL. Unable to add files in %s, line %d\n"),
				__FILE__, __LINE__);
		return;
	}

	data = (DbData *)db->db;

	archive = cxNewArchive();
	pmSetPackageArchive(pkg, archive);

	root = cxGetArchiveRoot(archive);
	cxSetDirName(root, "/");

	for (filesOffset = htGetOffset(table, GDBTAG_FILES);
		 filesOffset != 0;
		 filesOffset = nextOffset)
	{
		GdbHashTable *fileTable;
		PmFile *file;

		fileTable = htOpen(data->packageDb, filesOffset);

		if (fileTable == NULL)
		{
			pmError(PM_ERROR_FATAL,
					_("Unable to open file hashtable at %ld in %s, line %d\n"),
					filesOffset, __FILE__, __LINE__);
			abort();
		}

		nextOffset = fileTable->block->listNext;

		file = pmNewFile();

		if ((str = htGetString(fileTable, GDBTAG_NAME)) != NULL)
		{
			char *dirname, *basename;
			CxDirectory *dir;

			pmSplitPath(str, &dirname, &basename);

			dir = cxMkDir(root, dirname);

			cxDirAddFile(dir, PM_FILE_DATA(file)->file);
			pmSetFileName(file, basename);

			free(str);
			free(dirname);
			free(basename);
		}
		else
		{
			/* This should never happen. */
			continue;
		}

		if ((str = htGetString(fileTable, GDBTAG_CHECKSUM)) != NULL)
		{
			if (strlen(str) == 16)
			{
				char *md5 = __decompressMd5(str);

				pmSetFileChecksum(file, md5);

				free(md5);
			}
			else
			{
				pmSetFileChecksum(file, str);
			}

			free(str);
		}

		if ((l = htGetLong(fileTable, GDBTAG_MODE)) != 0)
		{
			pmSetFileMode(file, l);
		}

		if ((str = htGetString(fileTable, GDBTAG_OWNER)) != NULL)
		{
			pmSetFileOwner(file, str);
			free(str);
		}

		if ((str = htGetString(fileTable, GDBTAG_GROUP)) != NULL)
		{
			pmSetFileGroup(file, str);
			free(str);
		}

		if ((str = htGetString(fileTable, GDBTAG_SYMLINK)) != NULL)
		{
			pmSetFileSymlink(file, str);
			free(str);
		}

		if ((l = htGetLong(fileTable, GDBTAG_MAJOR_MINOR)) != 0)
		{
			pmSetFileMajorMinor(file, l);
		}

		if ((l = htGetLong(fileTable, GDBTAG_FILE_SIZE)) != 0)
		{
			pmSetFileSize(file, l);
		}

		if ((l = htGetLong(fileTable, GDBTAG_TIMESTAMP)) != 0)
		{
			pmSetFileDate(file, l);
		}
		
		gdbDestroyBlock(fileTable->block);

		pmPackageAddFile(pkg, file);
	}
}
