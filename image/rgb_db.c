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
 *  $Id: rgb_db.c,v 1.4 2003/05/22 17:35:50 leeming Exp $
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
#include "flimage_int.h"

#include <ctype.h>

#define DEBUG 0

static char *rgbfile[] =
{
    "/usr/lib/X11/rgb.txt",	/* typical */
    "/usr/X11R6/lib/X11/rgb.txt", /* also typical */
    "/usr/local/lib/X11/rgb.txt",	/* try     */
#if defined(sun) || defined(__sun__)
    "/usr/openwin/lib/rgb.txt",	/* Sun     */
#endif
#ifdef __EMX__
    "/XFree86/lib/X11/rgb.txt",	/* OS2     */
#endif
#ifdef __VMS
    "SYS$MANAGER:DECW$RGB.DAT",	/* vax */
#endif
    0
};

typedef struct
{
    char name[32];
    unsigned short r, g, b;
}
RGBDB;

static RGBDB *rgb_db;
static int db_size, nentries;	/* total size and filled entries */
static int read_entry(FILE * fp, int *r, int *g, int *b, char name[]);
static char hexv[256];

int
fl_init_RGBdatabase(const char *f)
{
    FILE *fp = 0;
    char name[128], *lname;
    char *const *file = rgbfile;
    int r, g, b, lr, lg, lb, size = 700;
    RGBDB *db, *dbs;

    if (rgb_db)
	return 1;

    if (f)
	fp = fopen(f, "r");

    /* search default location if can't open input file */
    for (; !fp && *file; file++)
	fp = fopen(*file, "r");

    if (!fp)
    {
	M_err("InitColorLookup", "can't find the rgb color database");
	return -1;
    }

    /* now get the database */
    if (!(rgb_db = fl_malloc(sizeof(*rgb_db) * size)))
    {
	M_err("InitColorLookup", "Can't get memory");
	return -1;
    }


    for (r = 0; r < 10; r++)
	hexv[r + '0'] = r;

    for (r = 10; r <= 15; r++)
    {
	hexv[r - 10 + 'a'] = r;
	hexv[r - 10 + 'A'] = r;
    }


    /* now read it */
    db = rgb_db;
    dbs = rgb_db + size;
    lname = "";
    lr = lg = lb = -1;
    for (; read_entry(fp, &r, &g, &b, name) && db < dbs;)
    {
	db->r = r;
	db->g = g;
	db->b = b;

	/* unique the entry on the fly */
	if (r != lr || g != lg || b != lb || strcasecmp(name, lname))
	{
	    lname = strcpy(db->name, name);
	    nentries++;
#if DEBUG
	    fprintf(stderr,"(%3d %3d %3d) %s\n", db->r, db->g, db->b, db->name);
#endif
	    if (db == dbs - 1)
	    {
		size += size / 2;
		rgb_db = fl_realloc(rgb_db, sizeof(*rgb_db) * size);
		dbs = rgb_db + size;
	    }
	    db++;
	    lr = r;
	    lg = g;
	    lb = b;
	}
    }

    fclose(fp);
    db_size = size;

#if DEBUG
    fprintf(stderr, " TotalEntries: %d of %d filled\n", nentries, db_size);
#endif

    return (nentries > 100) ? 1 : -1;
}

static int
read_entry(FILE * fp, int *r, int *g, int *b, char name[])
{
    int n;
    char buf[256], *p;

    if (!fgets(buf, sizeof(buf), fp))
	return 0;

    if (buf[0] == '!')
	fgets(buf, sizeof(buf), fp);

    if (sscanf(buf, " %d %d %d %n", r, g, b, &n) < 3)
	return 0;

    p = buf + n;

    /* squeeze out all spaces */
    while (*p)
    {
	if (*p != ' ' && *p != '\n')
	    *name++ = *p;
	p++;
    }
    *name = 0;

    return (feof(fp) || ferror(fp)) ? 0 : 1;
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
