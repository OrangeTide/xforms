/*
 *
 * This file is part of XForms.
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
 * Showing the use of non-modal file selector
 *
 * T.C. Zhao and M. Overmars
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "include/forms.h"
#include "fd/pmbrowse_gui.h"
#include <stdlib.h>

FD_ttt * fd_ttt;

static int load_file(const char *, void *);

int
main(int argc, char *argv[])
{
    fl_initialize(&argc, argv, "FormDemo", 0, 0);
    fd_ttt = create_form_ttt();

    fl_show_form(fd_ttt->ttt, FL_PLACE_CENTER, FL_TRANSIENT, "PixmapBrowser");

    fl_set_fselector_placement(FL_PLACE_FREE);
    fl_set_fselector_callback(load_file, 0);
    fl_show_fselector("Load a Pixmap file", 0, "*.x?m",0);
    fl_do_forms();
    return 0;
}

static int
load_file(const char *fname, void *data)
{
     int ispix = 0;
     char *p;

     if (( p = strrchr(fname, '.')))
         ispix = strcmp(p+1, "xbm") != 0;

     if(ispix)
     {
        fl_hide_object(fd_ttt->bm);
        fl_show_object(fd_ttt->pm);
        fl_free_pixmap_pixmap(fd_ttt->pm);
        fl_set_pixmap_file(fd_ttt->pm, fname);
     }
     else
     {
        fl_hide_object(fd_ttt->pm);
        fl_show_object(fd_ttt->bm);
        fl_set_bitmap_file(fd_ttt->bm, fname);
     }
     return 1;
}


void done(FL_OBJECT *ob, long q)
{
    fl_finish();
    exit(0);
}


void reload(FL_OBJECT *ob, long q)
{
    fl_set_fselector_placement(FL_PLACE_MOUSE);
    fl_set_fselector_callback(load_file, 0);
    fl_show_fselector("Load a Pix/bitMap file", 0, 0,0);
}
