/**
 * @file gnupdate.h GNUpdate Database Module
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
#ifndef _GNUPDATE_H_
#define _GNUPDATE_H_

#include <libpackman/packman.h>
#include <libpackman/internal.h>
#include "db/db.h"
#include "tags.h"

/* XXX Hard-code the database path.. for now. */
#define GNUPDATE_DB_PATH "/var/lib/gnupdate"


/**************************************************************************
 * A search type.
 **************************************************************************/
typedef enum
{
	SEARCH_TYPE_NAME,  /**< A search by name. */
	SEARCH_TYPE_FILE   /**< A search by file. */

} DbSearchType;

typedef struct
{
	GDatabase *packageDb;      /**< Package data file.                */

	GDatabase *namesIndex;     /**< Names index file.                 */
	GDatabase *filesIndex;     /**< Files index file.                 */
	GDatabase *groupsIndex;    /**< Groups index file.                */
	GDatabase *reqDepsIndex;   /**< Required dependencies index file. */
	GDatabase *provDepsIndex;  /**< Provided dependencies index file. */

} DbData;

/**************************************************************************
 * A search match.
 **************************************************************************/
typedef struct _DbMatchData DbMatchData;

struct _DbMatchData
{
	PmPackage *(*firstPackage)(PmDatabase *db, DbMatchData *data);
	PmPackage *(*nextPackage)(PmDatabase *db, DbMatchData *data);
	void (*destroyData)(DbMatchData *data);

	void *data;
	offset_t firstOffset;
	unsigned short index;
};

/**************************************************************************
 * Database Access Functions
 **************************************************************************/
PmStatus dbOpen(PmDatabase *db);
PmStatus dbClose(PmDatabase *db);


/**************************************************************************
 * Database Modification Functions
 **************************************************************************/
PmStatus dbAddPackage(PmDatabase *db, PmPackage *pkg);
PmStatus dbRemovePackage(PmDatabase *db, PmPackage *pkg);


/**************************************************************************
 * Misc. Database Functions
 **************************************************************************/
PmStatus dbDestroyMatches(PmMatches *matches);
void dbDestroyPkgData(void *data);

/**************************************************************************
 * Search Functions
 **************************************************************************/
PmStatus dbFindByName(PmDatabase *db, const char *name, PmMatches *matches);
PmStatus dbFindByGroup(PmDatabase *db, const char *name, PmMatches *matches);
PmStatus dbFindByFile(PmDatabase *db, const char *name, PmMatches *matches);
PmStatus dbFindByRequires(PmDatabase *db, const char *name,
						  PmMatches *matches);
PmStatus dbFindByProvides(PmDatabase *db, const char *name,
						  PmMatches *matches);
PmStatus dbFindByConflicts(PmDatabase *db, const char *name,
						   PmMatches *matches);
PmStatus dbGetAllPackages(PmDatabase *db, PmMatches *matches);
PmPackage *dbFirstPackage(PmDatabase *db, PmMatches *matches);
PmPackage *dbNextPackage(PmDatabase *db, PmMatches *matches);


/**************************************************************************
 * Extra Data Retrieval Functions
 **************************************************************************/
void dbGetFiles(PmDatabase *db, PmPackage *pkg);
void dbGetRequiredDeps(PmDatabase *db, PmPackage *pkg);
void dbGetProvidedDeps(PmDatabase *db, PmPackage *pkg);

/**************************************************************************
 * Search Helper Functions
 **************************************************************************/
DbMatchData *newMatchData(
	PmPackage *(*firstPackage)(PmDatabase *db, DbMatchData *data),
	PmPackage *(*nextPackage)(PmDatabase *db, DbMatchData *data),
	void (*destroyData)(DbMatchData *data));

void destroyMatchData(DbMatchData *data);

/**************************************************************************
 * Utility Functions
 **************************************************************************/
char *dbPackToString(void *data, size_t size);
char *dbPackTimestamp(void);
PmPackage *dbReadPackage(PmDatabase *db, offset_t offset);

#endif /* _GNUPDATE_H_ */
