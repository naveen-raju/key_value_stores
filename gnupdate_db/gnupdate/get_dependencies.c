/**
 * @file get_dependencies.c Returns a list of dependencies from a package
 *                          in the database.
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

void
dbGetRequiredDeps(PmDatabase *db, PmPackage *pkg)
{
	GdbHashTable *table;
	offset_t      depsOffset, nextOffset;
	DbData       *data;
	char         *str;
	long          l;

	table = (GdbHashTable *)PM_PACKAGE_DB_DATA(pkg);

	if (table == NULL)
	{
		pmError(PM_ERROR_WARNING,
				_("GNUpdate DB: "
				  "dbData == NULL. Unable to add reqDeps in %s, line %d\n"),
				__FILE__, __LINE__);
		return;
	}

	data = (DbData *)db->db;
	
	for (depsOffset = htGetOffset(table, GDBTAG_REQ_DEPS);
		 depsOffset != 0;
		 depsOffset = nextOffset)
	{
		GdbHashTable *depTable;
		PmDependency *dep;
		
		depTable = htOpen(data->packageDb, depsOffset);

		if (depTable == NULL)
		{
			pmError(PM_ERROR_FATAL,
					_("GNUpdate DB: "
					  "Unable to open reqdeps hashtable at %ld "
					  "in %s, line %d\n"),
					depsOffset, __FILE__, __LINE__);
			abort();
		}

		nextOffset = depTable->block->listNext;

		dep = pmNewDependency();
		
		if ((str = htGetString(depTable, GDBTAG_NAME)) != NULL)
		{
			pmSetDependencyName(dep, str);
			free(str);
		}

		if ((str = htGetString(depTable, GDBTAG_VERSION)) != NULL)
		{
			l = htGetLong(depTable, GDBTAG_VERSION_FLAGS);

			pmSetDependencyVersion(dep, str, l);

			free(str);
		}

		if ((str = htGetString(depTable, GDBTAG_OWNER)) != NULL)
		{
			pmSetDependencyOwner(dep, str);
			free(str);
		}

		if ((l = htGetLong(depTable, GDBTAG_TYPE)) != 0)
		{
			pmSetDependencyType(dep, l);
		}
		
		gdbDestroyBlock(depTable->block);

		pmPackageAddRequirement(pkg, dep);
	}
}

void
dbGetProvidedDeps(PmDatabase *db, PmPackage *pkg)
{
	GdbHashTable *table;
	offset_t      depsOffset, nextOffset;
	DbData       *data;
	char         *str;
	long          l;

	table = (GdbHashTable *)PM_PACKAGE_DB_DATA(pkg);

	if (table == NULL)
	{
		pmError(PM_ERROR_WARNING,
				_("GNUpdate DB: "
				  "dbData == NULL. Unable to add reqDeps in %s, line %d\n"),
				__FILE__, __LINE__);
		return;
	}

	data = (DbData *)db->db;
	
	for (depsOffset = htGetOffset(table, GDBTAG_PROV_DEPS);
		 depsOffset != 0;
		 depsOffset = nextOffset)
	{
		GdbHashTable *depTable;
		PmDependency *dep;
		
		depTable = htOpen(data->packageDb, depsOffset);

		if (depTable == NULL)
		{
			pmError(PM_ERROR_FATAL,
					_("Unable to open reqdeps hashtable at %ld\n"),
					depsOffset);
			abort();
		}

		nextOffset = depTable->block->listNext;

		dep = pmNewDependency();
		
		if ((str = htGetString(depTable, GDBTAG_NAME)) != NULL)
		{
			pmSetDependencyName(dep, str);
			free(str);
		}

		if ((str = htGetString(depTable, GDBTAG_VERSION)) != NULL)
		{
			l = htGetLong(depTable, GDBTAG_VERSION_FLAGS);

			pmSetDependencyVersion(dep, str, l);

			free(str);
		}

		if ((str = htGetString(depTable, GDBTAG_OWNER)) != NULL)
		{
			pmSetDependencyOwner(dep, str);
			free(str);
		}

		if ((l = htGetLong(depTable, GDBTAG_TYPE)) != 0)
		{
			pmSetDependencyType(dep, l);
		}

		gdbDestroyBlock(depTable->block);

		pmPackageAddProvide(pkg, dep);
	}
}

