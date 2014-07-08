/**
 * @file removepackage.c Package removal functions
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

static PmStatus
__removeFromNames(PmDatabase *db, PmPackage *package, offset_t offset)
{
	DbData *dbData;
	int r;

	dbData = (DbData *)db->db;

	r = btreeDelete(dbData->namesIndex->mainTree, pmGetPackageName(package));

	if (r == 0)
		pmError(PM_ERROR_WARNING, "Unable to delete key from names.idx\n");

	return PM_FAILED;
}

static PmStatus
__removeFromProvDeps(PmDatabase *db, PmPackage *package, offset_t offset)
{
	return PM_FAILED;
}

static PmStatus
__removeFromReqDeps(PmDatabase *db, PmPackage *package, offset_t offset)
{
	return PM_FAILED;
}

static PmStatus
__removeFromFiles(PmDatabase *db, PmPackage *package, offset_t offset)
{
	return PM_FAILED;
}

static PmStatus
__removeFromGroups(PmDatabase *db, PmPackage *package, offset_t offset)
{
	return PM_FAILED;
}

static PmStatus
__removeFromPackages(PmDatabase *db, PmPackage *package, offset_t offset)
{
	return PM_FAILED;
}

PmStatus
dbRemovePackage(PmDatabase *db, PmPackage *pkg)
{
	offset_t offset;
	PmStatus status;
	DbData *dbData;

	dbData = (DbData *)db->db;

	/*
	 * TODO:
	 *
	 * 1) Find the index of the package with this package's name,
	 *    version, release, and branch (later epoch) in packages.db.
	 *    Remove it and all trees/hashtables associated with it.
	 *
	 * 2) Remove entry in names.idx.
	 * 
	 * 3) Remove entries in provdeps.idx.
	 *
	 * 4) Remove entries in reqdeps.idx.
	 *
	 * 5) Remove entries in groups.idx.
	 *
	 * 6) Remove entries in files.idx.
	 */

	offset = btreeSearch(dbData->namesIndex->mainTree, pmGetPackageName(pkg));

	if (offset == 0)
		return PM_FAILED; /* Not found. */

	/*
	 * The package was found.
	 *
	 * Start off by removing from indexes.
	 */
	if ((status = __removeFromNames(db, pkg, offset)) != PM_SUCCESS)
		return status;

	if ((status = __removeFromProvDeps(db, pkg, offset)) != PM_SUCCESS)
		return status;

	if ((status = __removeFromReqDeps(db, pkg, offset)) != PM_SUCCESS)
		return status;

	if ((status = __removeFromFiles(db, pkg, offset)) != PM_SUCCESS)
		return status;

	if ((status = __removeFromGroups(db, pkg, offset)) != PM_SUCCESS)
		return status;

	if ((status = __removeFromPackages(db, pkg, offset)) != PM_SUCCESS)
		return status;

	return PM_SUCCESS;
}
