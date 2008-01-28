/*
 *
 *  This file is part of the XForms library package.
 *
 * XForms is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1, or
 * (at your option) any later version.
 *
 * XForms is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with XForms; see the file COPYING.  If not, write to
 * the Free Software Foundation, 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 *
 */

/**
 * \file flsnprintf.h
 *
 */

#ifndef FL_SNPRINTF_H
#define FL_SNPRINTF_H

#include <stdio.h>

#ifdef HAVE_SNPRINTF

#ifndef HAVE_DECL_SNPRINTF
int snprintf( char *,
			  size_t,
			  const char *,
			  ...);
#endif
#ifndef HAVE_DECL_VSNPRINTF
int vsnprintf( char *,
			   size_t,
			   const char *,
			   va_list);
#endif

#define fl_snprintf  snprintf
#define fl_vsnprintf vsnprintf

#else /* HAVE_SNPRINTF */

FL_EXPORT int fl_portable_snprintf( char *,
									size_t,
									const char *,
									...);

FL_EXPORT int fl_portable_vsnprintf( char *,
									 size_t,
									 const char *,
									 va_list );

#define fl_snprintf  fl_portable_snprintf
#define fl_vsnprintf fl_portable_vsnprintf

#endif /* HAVE_SNPRINTF */

#endif /* NOT FL_SNPRINTF_H */
