/**
 * @file db_cache.h Block caching functions
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
#ifndef _DB_CACHE_H_
#define _DB_CACHE_H_

/**
 * Adds a block to the cache.
 * 
 * If the block is already in the cache, the reference count will
 * be incremented.
 * 
 * @param db    The database.
 * @param block The block to add to the cache.
 */
void gdbCacheAddBlock(GDatabase *db, GdbBlock *block);

/**
 * Removes a block from the cache.
 *
 * If the block's reference count is greater than 1, the block will
 * stay in the cache and the reference count will be decremented.
 * If the reference count is 1, the block will be removed from the
 * cache.
 * 
 * @param db    The database.
 * @param block The block to remove from the cache.
 *
 * @return The reference count on the block.
 */
unsigned short gdbCacheRemoveBlock(GDatabase *db, GdbBlock *block);

/**
 * Returns a block from the cache.
 *
 * @param db     The database.
 * @param offset The offset of the block.
 *
 * @return The block at @a offset, or @c NULL if it's not in the cache.
 */
GdbBlock *gdbCacheGetBlock(GDatabase *db, offset_t offset);

#endif /* _DB_CACHE_H_ */

