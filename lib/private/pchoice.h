/*
 *  This file is part of the XForms library package.
 *
 *  XForms is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU Lesser General Public License as
 *  published by the Free Software Foundation; either version 2.1, or
 *  (at your option) any later version.
 *
 *  XForms is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XForms.  If not, see <http://www.gnu.org/licenses/>.
 */


/**
 * \file pchoice.h
 *.
 *  This file is part of the XForms library package.
 *  Copyright T.C. Zhao and Mark Overmars
 *  All rights reserved.
 *.
 */

#ifndef PCHOICE_H_
#define PCHOICE_H_

/* make sure that the first 5 items are the same as FL_MENU.
 * fdesign assumes this
 */

typedef struct
{
    int             numitems;		/* number of items in choice */
    int             val;			/* last menu item selected */
    char          * items[ FL_CHOICE_MAXITEMS + 1 ];	/* choice items */
    char          * shortcut[ FL_CHOICE_MAXITEMS + 1 ];
    unsigned char   mode[ FL_CHOICE_MAXITEMS + 1 ];
    int             align;			/* the choice text is aligned */
    int             fontsize;		/* font size */
    int             fontstyle;		/* font style */
    int             pushed;
    int             below;
    unsigned char   modechange[ FL_CHOICE_MAXITEMS + 1 ];
	int             counter;
    int             no_title;
} FLI_CHOICE_SPEC;

#endif
