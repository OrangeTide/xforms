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


/*
 *  $Id: rgb_db.c,v 1.5 2003/05/30 11:04:57 leeming Exp $
 *
 *  Copyright (c) 1999-2002 T.C. Zhao
 *
 *  search the rgb.txt database for a specific color
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "include/forms.h"
#include "flimage.h"

int
fl_init_RGBdatabase(const char *f)
{
    return 1;
}


/* A new implementation from Rouben Rostamian. */
int fl_lookup_RGBcolor(const char *colname, int *r, int *g, int *b)
{
    XColor xc;
    unsigned int M = (1U<<fl_state[fl_vmode].depth)-1;

    if (XParseColor(fl_display, fl_state[fl_vmode].colormap,
		    colname,  &xc) == 0)
	return -1;

    *r = 255 * xc.red   / M;
    *g = 255 * xc.green / M;
    *b = 255 * xc.blue  / M;

    return 0;
}
