/**
 * @file addpackage.c Package addition functions
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

#define HEX_TO_DEC(x) (((x) >= '0' && (x) <= '9') ? (x) - '0' : \
					   ((x) >= 'a' && (x) <= 'f') ? ((x) - 'a') + 10 : 0)

static char *
__compressMd5(const char *md5)
{
	char newMd5[17];
	int i, j;

	if (md5 == NULL)
		return NULL;

	for (i = 0, j = 0; i < 32; i += 2, j++)
	{
		newMd5[j] = (((HEX_TO_DEC(md5[i]) << 4) & 0xF0) |
					 (HEX_TO_DEC(md5[i + 1]) & 0x0F));
	}

	newMd5[16] = '\0';

	return strdup(newMd5);
}

static offset_t
__addPackageFiles(PmDatabase *db, PmPackage *pkg, offset_t pkgOffset)
{
	GdbHashTable  *table, *prevTable = NULL;
	GdbOffsetList *list;
	DbData        *data;
	PmFile        *file;
	offset_t       firstOffset = 0, prevOffset = 0;
	offset_t       listOffset;

	data = (DbData *)db->db;

	for (file = pmFirstFile(pkg);
		 file != NULL;
		 file = pmNextFile(file))
	{
		const char *fileName = pmGetFileName(file);
		char *md5;

		table = htCreate(data->packageDb);

		if (table == NULL)
		{
			pmError(PM_ERROR_FATAL,
					_("GNUpdate DB: "
					  "Unable to create Hash Table in %s, line %d\n"),
					__FILE__, __LINE__);
			abort();
		}

		htAddString(table, GDBTAG_NAME,        fileName);
		htAddString(table, GDBTAG_OWNER,       pmGetFileOwner(file));
		htAddString(table, GDBTAG_GROUP,       pmGetFileGroup(file));
		htAddString(table, GDBTAG_SYMLINK,     pmGetFileSymlink(file));
		htAddLong(table,   GDBTAG_MAJOR_MINOR, pmGetFileMajorMinor(file));
		htAddLong(table,   GDBTAG_MODE,        pmGetFileMode(file));
		htAddLong(table,   GDBTAG_FILE_SIZE,   pmGetFileSize(file));
		htAddLong(table,   GDBTAG_TIMESTAMP,   pmGetFileDate(file));

		if ((md5 = __compressMd5(pmGetFileChecksum(file))) != NULL)
		{
			htAddString(table, GDBTAG_CHECKSUM, md5);

			free(md5);
		}

		gdbWriteBlock(table->block);

		/* Update the previous table. */
		if (prevTable != NULL)
		{
			prevTable->block->listNext = table->block->offset;

			GDB_SET_DIRTY(prevTable->block);

			gdbWriteBlockHeader(prevTable->block);

			gdbDestroyBlock(prevTable->block);
		}

		prevOffset = table->block->offset;

		if (firstOffset == 0)
			firstOffset = prevOffset;

		/* Add to the index file */
		listOffset = btreeSearch(data->filesIndex->mainTree, fileName);

		if (listOffset == 0)
		{
			btreeInsert(data->filesIndex->mainTree, fileName, pkgOffset, 0);
#if 0
			list = olCreate(data->filesIndex);

			olAddOffset(list, pkgOffset);

			GDB_SET_DIRTY(list->block);
			gdbWriteBlock(list->block);

			btreeInsert(data->filesIndex->mainTree, fileName,
						list->block->offset);
#endif
		}
		else
		{
			blocktype_t type;

			type = gdbBlockTypeAt(data->filesIndex, listOffset);

			if (type == GDB_BLOCK_OFFSET_LIST)
			{
				list = olOpen(data->filesIndex, listOffset);

				olAddOffset(list, pkgOffset);

				GDB_SET_DIRTY(list->block);
				gdbWriteBlock(list->block);
			}
			else
			{
				/*
				 * The index is (probably) pointing to a BTreeNode. Convert it.
				 */
				list = olCreate(data->filesIndex);

				olAddOffset(list, listOffset);
				olAddOffset(list, pkgOffset);

				GDB_SET_DIRTY(list->block);
				gdbWriteBlock(list->block);

				btreeInsert(data->filesIndex->mainTree, fileName,
							list->block->offset, 1);
			}

			gdbDestroyBlock(list->block);

#if 0
			list = olOpen(data->filesIndex, listOffset);

			olAddOffset(list, pkgOffset);

			GDB_SET_DIRTY(list->block);
			gdbWriteBlock(list->block);
#endif
		}

		/* Update the progress */
		pmPackageUpdateProgress(pkg);

		prevTable = table;
	}

	if (prevTable != NULL)
		gdbDestroyBlock(prevTable->block);

	return firstOffset;
}

static offset_t
__addRequiredDeps(PmDatabase *db, PmPackage *pkg, offset_t pkgOffset)
{
	DbData       *data;
	GdbHashTable *table, *prevTable = NULL;
	BTree        *indexTree;
	PmDependency *dep;
	offset_t      firstOffset = 0, prevOffset = 0;

	data = (DbData *)db->db;

	for (dep = pmFirstRequirement(pkg);
		 dep != NULL;
		 dep = pmNextRequirement(dep))
	{
		const char *version;
		PmRelationship rel;
		
		table = htCreate(data->packageDb);

		if (table == NULL)
		{
			pmError(PM_ERROR_FATAL,
					_("GNUpdate DB: "
					  "Unable to create Hash Table in %s, line %d\n"),
					__FILE__, __LINE__);
			abort();
		}

		htAddString(table, GDBTAG_NAME,  pmGetDependencyName(dep));
		htAddString(table, GDBTAG_OWNER, pmGetDependencyOwner(dep));
		htAddLong(table,   GDBTAG_TYPE,  pmGetDependencyType(dep));

		pmGetDependencyVersion(dep, &version, &rel);

		if (version != NULL)
		{
			htAddString(table, GDBTAG_VERSION,       version);
			htAddLong(table,   GDBTAG_VERSION_FLAGS, rel);
		}

		gdbWriteBlock(table->block);

		/* Update the previous table. */
		if (prevTable != NULL)
		{
			prevTable->block->listNext = table->block->offset;

			GDB_SET_DIRTY(prevTable->block);

			gdbWriteBlockHeader(prevTable->block);

			gdbDestroyBlock(prevTable->block);
		}

		prevOffset = table->block->offset;
		
		if (firstOffset == 0)
			firstOffset = prevOffset;

		/* Add to the index file */
		gdbAddTree(data->reqDepsIndex, data->reqDepsIndex->mainTree,
				   pmGetDependencyName(dep), &indexTree);

		if (indexTree == NULL)
		{
			/* Shouldn't happen. */
			pmError(PM_ERROR_FATAL,
					_("GNUpdate DB: "
					  "indexTree == NULL in %s, line %d\n"),
					__FILE__, __LINE__);
			abort();
		}

		gdbAddIndexEntry(data->reqDepsIndex, indexTree,
						 pmGetPackageName(pkg), pkgOffset);

		btreeClose(indexTree);

		/* Update the progress */
		pmPackageUpdateProgress(pkg);

		prevTable = table;
	}

	if (prevTable != NULL)
		gdbDestroyBlock(prevTable->block);

	return firstOffset;
}

static offset_t
__addProvidedDeps(PmDatabase *db, PmPackage *pkg, offset_t pkgOffset)
{
	DbData       *data;
	GdbHashTable *table, *prevTable = NULL;
	BTree        *indexTree;
	PmDependency *dep;
	offset_t      firstOffset = 0, prevOffset = 0;

	data = (DbData *)db->db;

	for (dep = pmFirstProvide(pkg);
		 dep != NULL;
		 dep = pmNextProvide(dep))
	{
		const char *version;
		PmRelationship rel;
		
		table = htCreate(data->packageDb);

		if (table == NULL)
		{
			pmError(PM_ERROR_FATAL,
					_("GNUpdate DB: "
					  "Unable to create Hash Table in %s, line %d\n"),
					__FILE__, __LINE__);
			abort();
		}

		htAddString(table, GDBTAG_NAME,  pmGetDependencyName(dep));
		htAddString(table, GDBTAG_OWNER, pmGetDependencyOwner(dep));
		htAddLong(table,   GDBTAG_TYPE,  pmGetDependencyType(dep));

		pmGetDependencyVersion(dep, &version, &rel);

		if (version != NULL)
		{
			htAddString(table, GDBTAG_VERSION,       version);
			htAddLong(table,   GDBTAG_VERSION_FLAGS, rel);
		}

		gdbWriteBlock(table->block);

		/* Update the previous table. */
		if (prevTable != NULL)
		{
			prevTable->block->listNext = table->block->offset;

			GDB_SET_DIRTY(prevTable->block);

			gdbWriteBlockHeader(prevTable->block);

			gdbDestroyBlock(prevTable->block);
		}

		prevOffset = table->block->offset;
		
		if (firstOffset == 0)
			firstOffset = prevOffset;

		/* Add to the index file */
		gdbAddTree(data->provDepsIndex, data->provDepsIndex->mainTree,
				   pmGetDependencyName(dep), &indexTree);

		if (indexTree == NULL)
		{
			/* Shouldn't happen. */
			pmError(PM_ERROR_FATAL,
					_("GNUpdate DB: "
					  "indexTree == NULL in %s, line %d\n"),
					__FILE__, __LINE__);
			abort();
		}

		gdbAddIndexEntry(data->provDepsIndex, indexTree,
						 pmGetPackageName(pkg), pkgOffset);
		
		btreeClose(indexTree);

		/* Update the progress */
		pmPackageUpdateProgress(pkg);

		prevTable = table;
	}

	if (prevTable != NULL)
		gdbDestroyBlock(prevTable->block);

	return firstOffset;
}

/**
 * Writes an entry in the main package data file.
 */
static offset_t
__writePackageEntry(PmDatabase *db, PmPackage *pkg)
{
	DbData *data;
	GdbHashTable *table;
	char *timestamp;
	offset_t offset;
	offset_t childOffset;
	char *script;

	data = (DbData *)db->db;

	/* XXX */
	table = htCreate(data->packageDb);

	/* Add info about the package. */
	htAddString(table, GDBTAG_NAME,           pmGetPackageName(pkg));
	htAddString(table, GDBTAG_VERSION,        pmGetPackageVersion(pkg));
	htAddString(table, GDBTAG_RELEASE,        pmGetPackageRelease(pkg));
	htAddString(table, GDBTAG_ARCH,           pmGetPackageArch(pkg));
	htAddString(table, GDBTAG_BRANCH,         pmGetPackageBranch(pkg));
	htAddString(table, GDBTAG_URL,            pmGetPackageUrl(pkg));
	htAddString(table, GDBTAG_GROUP,          pmGetPackageGroup(pkg));
	htAddString(table, GDBTAG_LICENSE,        pmGetPackageLicense(pkg));
	htAddString(table, GDBTAG_SUMMARY,        pmGetPackageSummary(pkg));
	htAddString(table, GDBTAG_DESCRIPTION,    pmGetPackageDescription(pkg));
	htAddLong(table,   GDBTAG_FILE_SIZE,      pmGetPackageFileSize(pkg));
	htAddLong(table,   GDBTAG_INSTALLED_SIZE, pmGetPackageInstalledSize(pkg));

	/* Add the scripts. */
	if ((script = pmGetScript(pkg, PM_SCRIPT_PRE, PM_SCRIPTEVT_INSTALL,
							  NULL)) != NULL)
	{
		htAddString(table, GDBTAG_PREIN_SCRIPT, script);
		free(script);
	}

	if ((script = pmGetScript(pkg, PM_SCRIPT_POST, PM_SCRIPTEVT_INSTALL,
							  NULL)) != NULL)
	{
		htAddString(table, GDBTAG_POSTIN_SCRIPT, script);
		free(script);
	}

	if ((script = pmGetScript(pkg, PM_SCRIPT_PRE, PM_SCRIPTEVT_UNINSTALL,
							  NULL)) != NULL)
	{
		htAddString(table, GDBTAG_PREUN_SCRIPT, script);
		free(script);
	}

	if ((script = pmGetScript(pkg, PM_SCRIPT_POST, PM_SCRIPTEVT_UNINSTALL,
							  NULL)) != NULL)
	{
		htAddString(table, GDBTAG_POSTUN_SCRIPT, script);
		free(script);
	}

	/* Ugly, but... add dud offsets, which we'll update in a second. */
	if (pmFirstFile(pkg) != NULL)
		htAddOffset(table, GDBTAG_FILES, 1234);

	if (pmFirstRequirement(pkg) != NULL)
		htAddOffset(table, GDBTAG_REQ_DEPS, 1234);

	if (pmFirstProvide(pkg) != NULL)
		htAddOffset(table, GDBTAG_PROV_DEPS, 1234);

	/* And write.. */
	gdbWriteBlock(table->block);
	
	/* Add a files B+Tree and populate it. */
	if (pmFirstFile(pkg) != NULL)
	{
		childOffset = __addPackageFiles(db, pkg, table->block->offset);
		htAddOffset(table, GDBTAG_FILES, childOffset);
	}

	/* Add a required dependencies B+Tree and populate it. */
	if (pmFirstRequirement(pkg) != NULL)
	{
		childOffset = __addRequiredDeps(db, pkg, table->block->offset);
		htAddOffset(table, GDBTAG_REQ_DEPS, childOffset);
	}

	/* Add a provided dependencies B+Tree and populate it. */
	if (pmFirstProvide(pkg) != NULL)
	{
		childOffset = __addProvidedDeps(db, pkg, table->block->offset);
		htAddOffset(table, GDBTAG_PROV_DEPS, childOffset);
	}

	/* Write the hashtable to disk again. */
	gdbWriteBlock(table->block);

	offset = table->block->offset;

	/* Add the index to the tree. */
	timestamp = dbPackTimestamp();

	btreeInsert(data->packageDb->mainTree, timestamp, offset, 0);
	free(timestamp);

	gdbDestroyBlock(table->block);

	/* Update the progress. */
	pmPackageUpdateProgress(pkg);
	
	return offset;
}

/**
 * Writes the Name index file.
 */
static PmStatus
__writeNameIndexFile(PmDatabase *db, PmPackage *pkg, offset_t offset)
{
	const char *pkgName;
	GdbStatus status;
	DbData *data;

	data = (DbData *)db->db;
	
	pkgName = pmGetPackageName(pkg);
	
	status = gdbAddIndexEntry(data->namesIndex, data->namesIndex->mainTree,
							  pkgName, offset);
	
	return PM_SUCCESS;
}

static PmStatus
__writeGroupIndexFile(PmDatabase *db, PmPackage *pkg, offset_t offset)
{
	BTree *indexTree;
	GdbStatus status;
	DbData *data;

	data = (DbData *)db->db;

	status = gdbAddTree(data->groupsIndex, data->groupsIndex->mainTree,
						pmGetPackageGroup(pkg), &indexTree);

	if (status != PM_SUCCESS)
	{
		/* Shouldn't happen. */
		pmError(PM_ERROR_FATAL,
				_("GNUpdate DB: "
				  "Unable to add groups index tree in %s, line %d\n"),
				__FILE__, __LINE__);
		abort();
	}

	status = gdbAddIndexEntry(data->groupsIndex, indexTree,
							  pmGetPackageName(pkg), offset);

	btreeClose(indexTree);

	return status;
}

PmStatus
dbAddPackage(PmDatabase *db, PmPackage *pkg)
{
	PmStatus status;
	offset_t offset;

	/* XXX This should be moved elsewhere I think? */
	if (pmGetPackageBranch(pkg) == NULL)
		pmSetPackageBranch(pkg, "default");

	if ((offset = __writePackageEntry(db, pkg)) == 0)
		return PM_FAILED;

	if ((status = __writeNameIndexFile(db, pkg, offset)) != PM_SUCCESS)
		return status;

	if ((status = __writeGroupIndexFile(db, pkg, offset)) != PM_SUCCESS)
		return status;

	return PM_SUCCESS;
}

