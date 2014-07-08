/**
 * @file gnupdate.c GNUpdate Database Module
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

static GDatabase *
__loadDatabase(const char *baseName, GdbType type, PmAccessMode mode)
{
	struct stat sb;
	size_t len;
	char *filename;
	GDatabase *db;

	len = strlen(GNUPDATE_DB_PATH) + strlen(baseName) + 2;

	MEM_CHECK(filename = (char *)malloc(len));

	snprintf(filename, len, "%s/%s", GNUPDATE_DB_PATH, baseName);

	if (stat(filename, &sb) != 0 || !S_ISREG(sb.st_mode))
	{
		free(filename);
		return NULL;
	}

	db = gdbOpen(filename, type, mode);

	free(filename);

	return db;
}

static GDatabase *
__createDatabase(const char *baseName, GdbType type)
{
	char *filename;
	GDatabase *db;
	size_t len;

	len = strlen(GNUPDATE_DB_PATH) + strlen(baseName) + 2;

	MEM_CHECK(filename = (char *)malloc(len));

	snprintf(filename, len, "%s/%s", GNUPDATE_DB_PATH, baseName);

	db = gdbCreate(filename, type);

	free(filename);

	return db;
}

PmStatus
dbOpen(PmDatabase *db)
{
	struct stat sb;
	DbData *data;
	PmAccessMode mode;

	if (stat(GNUPDATE_DB_PATH, &sb) != 0)
		return PM_FAILED;

	mode = pmGetDbAccessMode(db);

	/* Create the DbData structure */
	MEM_CHECK(data = (DbData *)malloc(sizeof(DbData)));
	memset(data, 0, sizeof(DbData));

	/* Open the package data file. */
	data->packageDb = __loadDatabase("packages.db", GDB_DATA_FILE, mode);

	if (data->packageDb == NULL)
	{
		free(data);

		return PM_FAILED;
	}
	
	/* Open the names index file. */
	data->namesIndex = __loadDatabase("names.idx", GDB_INDEX_FILE, mode);

	if (data->namesIndex == NULL)
	{
		gdbClose(data->packageDb);

		free(data);

		return PM_FAILED;
	}

	/* Open the files index file. */
	data->filesIndex = __loadDatabase("files.idx", GDB_INDEX_FILE, mode);

	if (data->filesIndex == NULL)
	{
		gdbClose(data->packageDb);
		gdbClose(data->namesIndex);

		free(data);

		return PM_FAILED;
	}

	/* Open the groups index file. */
	data->groupsIndex = __loadDatabase("groups.idx", GDB_INDEX_FILE, mode);

	if (data->groupsIndex == NULL)
	{
		gdbClose(data->packageDb);
		gdbClose(data->namesIndex);
		gdbClose(data->filesIndex);

		free(data);

		return PM_FAILED;
	}

	/* Open the required dependencies index file. */
	data->reqDepsIndex = __loadDatabase("reqdeps.idx", GDB_INDEX_FILE, mode);

	if (data->reqDepsIndex == NULL)
	{
		gdbClose(data->packageDb);
		gdbClose(data->namesIndex);
		gdbClose(data->filesIndex);
		gdbClose(data->groupsIndex);

		free(data);

		return PM_FAILED;
	}

	/* Open the provided dependencies index file. */
	data->provDepsIndex = __loadDatabase("provdeps.idx", GDB_INDEX_FILE, mode);
	
	if (data->provDepsIndex == NULL)
	{
		gdbClose(data->packageDb);
		gdbClose(data->namesIndex);
		gdbClose(data->filesIndex);
		gdbClose(data->groupsIndex);
		gdbClose(data->reqDepsIndex);

		free(data);

		return PM_FAILED;
	}

	db->db = data;

	return PM_SUCCESS;
}

PmStatus
dbCreate(PmDatabase *db)
{
	struct stat sb;
	DbData *data;
	
	if (stat(GNUPDATE_DB_PATH, &sb) != 0)
		mkdir(GNUPDATE_DB_PATH, 0755);

	/* Create the DbData structure */
	MEM_CHECK(data = (DbData *)malloc(sizeof(DbData)));
	memset(data, 0, sizeof(DbData));
	
	/* Create the package data file. */
	data->packageDb = __createDatabase("packages.db", GDB_DATA_FILE);

	if (data->packageDb == NULL)
	{
		free(data);

		return PM_FAILED;
	}

	/* Create the names index file. */
	data->namesIndex = __createDatabase("names.idx", GDB_INDEX_FILE);

	if (data->namesIndex == NULL)
	{
		gdbClose(data->packageDb);

		free(data);

		return PM_FAILED;
	}

	/* Create the files index file. */
	data->filesIndex = __createDatabase("files.idx", GDB_INDEX_FILE);

	if (data->filesIndex == NULL)
	{
		gdbClose(data->packageDb);
		gdbClose(data->namesIndex);

		free(data);

		return PM_FAILED;
	}

	/* Create the groups index file. */
	data->groupsIndex = __createDatabase("groups.idx", GDB_INDEX_FILE);

	if (data->groupsIndex == NULL)
	{
		gdbClose(data->packageDb);
		gdbClose(data->namesIndex);
		gdbClose(data->filesIndex);

		free(data);

		return PM_FAILED;
	}

	/* Create the required dependencies index file. */
	data->reqDepsIndex = __createDatabase("reqdeps.idx", GDB_INDEX_FILE);

	if (data->reqDepsIndex == NULL)
	{
		gdbClose(data->packageDb);
		gdbClose(data->namesIndex);
		gdbClose(data->filesIndex);
		gdbClose(data->groupsIndex);

		free(data);

		return PM_FAILED;
	}

	/* Create the provided dependencies index file. */
	data->provDepsIndex = __createDatabase("provdeps.idx", GDB_INDEX_FILE);
	
	if (data->provDepsIndex == NULL)
	{
		gdbClose(data->packageDb);
		gdbClose(data->namesIndex);
		gdbClose(data->filesIndex);
		gdbClose(data->groupsIndex);
		gdbClose(data->reqDepsIndex);

		free(data);

		return PM_FAILED;
	}

	db->db = data;

	return PM_SUCCESS;
}

PmStatus
dbClose(PmDatabase *db)
{
	DbData *data = (DbData *)db->db;

	gdbClose(data->packageDb);
	gdbClose(data->namesIndex);
	gdbClose(data->filesIndex);
	gdbClose(data->groupsIndex);
	gdbClose(data->reqDepsIndex);
	gdbClose(data->provDepsIndex);

	free(data);

	return PM_SUCCESS;
}

PmStatus
dbRebuild(PmDatabase *db)
{
	return PM_NOT_SUPPORTED;
}

unsigned long
dbGetPackageCount(PmDatabase *db)
{
	return btreeGetSize(((DbData *)db->db)->packageDb->mainTree);
}

void
dbDestroyPkgData(void *data)
{
	gdbDestroyBlock(((GdbHashTable *)data)->block);
}

char *
dbGetScript(PmDatabase *db, PmPackage *pkg, PmScriptType type,
			PmScriptEvent event, const char *trigger)
{
	GdbHashTable *table;
	GdbTag tag;

	table = PM_PACKAGE_DB_DATA(pkg);

	if (event == PM_SCRIPTEVT_INSTALL)
	{
		switch (type)
		{
			case PM_SCRIPT_PRE:  tag = GDBTAG_PREIN_SCRIPT;  break;
			case PM_SCRIPT_POST: tag = GDBTAG_POSTIN_SCRIPT; break;
			default:
				return NULL;
		}
	}
	else if (event == PM_SCRIPTEVT_UNINSTALL)
	{
		switch (type)
		{
			case PM_SCRIPT_PRE:  tag = GDBTAG_PREUN_SCRIPT;  break;
			case PM_SCRIPT_POST: tag = GDBTAG_POSTUN_SCRIPT; break;
			default:
				return NULL;
		}
	}
	else
		return NULL;

	return htGetString(table, tag);
}

static PmDatabaseOps ops =
{
	dbOpen,              /* open             */
	dbCreate,            /* create           */
	dbClose,             /* close            */
	dbRebuild,           /* rebuild          */
	dbGetPackageCount,   /* getPackageCount  */
	dbAddPackage,        /* addPackage       */
	dbRemovePackage,     /* removePackage    */
	dbFindByName,        /* findByName       */
	dbFindByGroup,       /* findByGroup      */
	dbFindByFile,        /* findByFile       */
	dbFindByRequires,    /* findByRequires   */
	dbFindByProvides,    /* findByProvides   */
	NULL,                /* findByConflicts  */
	dbGetAllPackages,    /* getAllPackages   */
	dbFirstPackage,      /* firstPackage     */
	dbNextPackage,       /* nextPackage      */
	dbDestroyMatches,    /* destroyMatches   */
	dbDestroyPkgData,    /* destroyPkgData   */
	dbGetFiles,          /* getFiles         */
	dbGetRequiredDeps,   /* getRequiredDeps  */
	dbGetProvidedDeps,   /* getProvidedDeps  */
	dbGetScript          /* getScript        */
};

static void
__moduleInit(PmModuleType type)
{
}

PM_INIT_DB_MODULE(gnupdate, __moduleInit, ops)
