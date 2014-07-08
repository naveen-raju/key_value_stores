/**
 * @file db_lock.h Database Locking code
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
#ifndef _DB_LOCK_H_
#define _DB_LOCK_H_

/**
 * Types of locks.
 */
typedef enum
{
	/** Node is unlocked. Used for IsLocked() functions. */
	DB_UNLOCKED = 0x00,

	/** Lock for writing. Nobody else can read or write. */
	DB_WRITE_LOCK = 0x01,

	/** Lock for reading. Nobody else can write. */
	DB_READ_LOCK = 0x02

} GdbLockType;

#include "db.h"


/**
 * Locks the free block list.
 *
 * If the free block list is already locked, this will wait until it is
 * unlocked before locking and returning.
 *
 * @param db   The active database.
 * @param type The type of lock.
 *
 * @return 1 on success, 0 on failure.
 */
char gdbLockFreeBlockList(GDatabase *db, GdbLockType type);

/**
 * Unlocks the free block list.
 *
 * @param db The active database.
 *
 * @return 1 on success, 0 on failure.
 */
char gdbUnlockFreeBlockList(GDatabase *db);

/**
 * Returns the current lock on the free block list.
 *
 * @param db The active database.
 * 
 * @return The current lock on the free blocks list (or DB_UNLOCKED if none.)
 */
GdbLockType gdbGetFreeBlockListLock(GDatabase *db);

#endif /* _DB_LOCK_H_ */

