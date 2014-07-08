/**
 * @file db_lock.c Database locking code
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

char
gdbLockFreeBlockList(GDatabase *db, GdbLockType type)
{
	if (db == NULL)
		return 0;

	if (type == DB_UNLOCKED)
		return gdbUnlockFreeBlockList(db);

	return 0;
}

char
gdbUnlockFreeBlockList(GDatabase *db)
{
	return 0;
}

GdbLockType
gdbGetFreeBlockListLock(GDatabase *db)
{
	return DB_UNLOCKED;
}

