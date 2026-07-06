/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <https://www.gnu.org/licenses/old-licenses/gpl-2.0>.
 */

/** @file ios_stdafx.h iOS replacement for osx_stdafx.h; iOS has no CoreServices/ApplicationServices. */

#ifndef IOS_STDAFX_H
#define IOS_STDAFX_H

#include <TargetConditionals.h>

#define __STDC_LIMIT_MACROS
#include <stdint.h>

/* Check for mismatching 'architectures' */
#if !defined(STRGEN) && !defined(SETTINGSGEN) && ((defined(__LP64__) && !defined(POINTER_IS_64BIT)) || (!defined(__LP64__) && defined(POINTER_IS_64BIT)))
#	error "Compiling 64 bits without POINTER_IS_64BIT set! (or vice versa)"
#endif

/* CoreFoundation types are needed by the shared Apple code in os/macosx/macos.h. */
#include <CoreFoundation/CoreFoundation.h>

#undef bool
#undef false
#undef true

/* The iOS SDK has a non-const iconv. */
#define HAVE_NON_CONST_ICONV

#endif /* IOS_STDAFX_H */
