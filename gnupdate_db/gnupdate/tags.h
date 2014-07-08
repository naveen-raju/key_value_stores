/**
 * @file tags.h Identifier tags
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
#ifndef _GNUPDATE_TAGS_H_
#define _GNUPDATE_TAGS_H_

/**
 * Tags
 */
typedef enum
{
	GDBTAG_NAME              = 100,
	GDBTAG_VERSION           = 101,
	GDBTAG_RELEASE           = 102,
	GDBTAG_ARCH              = 103,
	GDBTAG_BRANCH            = 104,
	GDBTAG_URL               = 105,
	GDBTAG_GROUP             = 106,
	GDBTAG_LICENSE           = 107,
	GDBTAG_SUMMARY           = 108,
	GDBTAG_DESCRIPTION       = 109,
	GDBTAG_FILE_SIZE         = 110,
	GDBTAG_INSTALLED_SIZE    = 111,
	GDBTAG_INSTALL_DATE      = 112,
	GDBTAG_VENDOR            = 113,
	GDBTAG_BUILD_DATE        = 114,
	GDBTAG_BUILD_HOST        = 115,
	GDBTAG_SOURCE_FILE       = 116,
	GDBTAG_PACKAGER          = 117,
	GDBTAG_FILES             = 118,
	GDBTAG_REQ_DEPS          = 119,
	GDBTAG_PROV_DEPS         = 120,
	GDBTAG_TYPE              = 121,
	GDBTAG_CHECKSUM          = 122,
	GDBTAG_MODE              = 123,
	GDBTAG_OWNER             = 124,
	GDBTAG_SYMLINK           = 125,
	GDBTAG_MAJOR_MINOR       = 126,
	GDBTAG_TIMESTAMP         = 127,
	GDBTAG_VERSION_FLAGS     = 128,
	GDBTAG_PREIN_SCRIPT      = 200,
	GDBTAG_POSTIN_SCRIPT     = 201,
	GDBTAG_PREUN_SCRIPT      = 202,
	GDBTAG_POSTUN_SCRIPT     = 203

} GdbTag;

#endif /* _GNUPDATE_TAGS_H_ */

