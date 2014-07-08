/**
 * @file search.c Package search functions.
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

DbMatchData *
newMatchData(PmPackage *(*firstPackage)(PmDatabase *db, DbMatchData *data),
			 PmPackage *(*nextPackage)(PmDatabase *db, DbMatchData *data),
			 void (*destroyData)(DbMatchData *data))
{
	DbMatchData *matchData;
	
	MEM_CHECK(matchData = (DbMatchData *)malloc(sizeof(DbMatchData)));

	memset(matchData, 0, sizeof(DbMatchData));

	matchData->firstPackage = firstPackage;
	matchData->nextPackage  = nextPackage;
	matchData->destroyData  = destroyData;

	return matchData;
}

void
destroyMatchData(DbMatchData *data)
{
	if (data == NULL)
		return;

	if (data->destroyData != NULL)
		data->destroyData(data);

	free(data);
}

PmStatus
dbFindByConflicts(PmDatabase *db, const char *name, PmMatches *matches)
{
	return PM_NOT_SUPPORTED;
}

PmPackage *
dbFirstPackage(PmDatabase *db, PmMatches *matches)
{
	DbMatchData *matchData = (DbMatchData *)matches->matches;

	if (matchData == NULL)
		return NULL;

	return matchData->firstPackage(db, matchData);
}

PmPackage *
dbNextPackage(PmDatabase *db, PmMatches *matches)
{
	DbMatchData *matchData = (DbMatchData *)matches->matches;

	if (matchData == NULL)
		return NULL;

	return matchData->nextPackage(db, matchData);
}

PmStatus
dbDestroyMatches(PmMatches *matches)
{
	DbMatchData *matchData = (DbMatchData *)matches->matches;

	if (matchData == NULL)
		return PM_FAILED;

	destroyMatchData(matchData);

	return PM_SUCCESS;
}

