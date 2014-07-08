/**
 * @file search_by_name.c Search by name support.
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

static PmPackage *
__firstPackage(PmDatabase *db, DbMatchData *matchData)
{
	return dbReadPackage(db, matchData->firstOffset);
}

static PmPackage *
__nextPackage(PmDatabase *db, DbMatchData *data)
{
	return NULL;
}

static void
__destroyData(DbMatchData *data)
{
}

PmStatus
dbFindByName(PmDatabase *db, const char *name, PmMatches *matches)
{
	DbMatchData *data;
	DbData *dbData;
	offset_t offset;

	dbData = (DbData *)db->db;
	
	/* Search for the name. */
	offset = btreeSearch(dbData->namesIndex->mainTree, name);

	if (offset == 0)
		return PM_FAILED; /* Not found. */
	
	/* The package was found. */
	data = newMatchData(__firstPackage, __nextPackage, __destroyData);
	matches->matches = data;

	data->firstOffset = offset;

	return PM_SUCCESS;
}

