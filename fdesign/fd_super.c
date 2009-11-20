/*
 * This file is part of XForms.
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
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with XForms.  If not, see <http://www.gnu.org/licenses/>.
 */


/**
 * \file fd_super.c
 *
 *  This file is part of XForms package
 *  Copyright (c) 1996-2002  T.C. Zhao and Mark Overmars
 *  All rights reserved.
 *
 * translation between superspec and spec
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include/forms.h"
#include "fd_main.h"
#include "fd_spec.h"

#include "private/pslider.h"
#include "private/pbrowser.h"
#include "private/ppositioner.h"
#include "private/pcounter.h"
#include "private/pspinner.h"
#include "private/pscrollbar.h"
#include "private/pdial.h"
#include "private/pxyplot.h"
#include "private/pchoice.h"
#include "private/pmenu.h"


/***************************************
 ***************************************/

SuperSPEC *
get_superspec( FL_OBJECT * ob )
{
    SuperSPEC *sp = ob->u_vdata;

    return sp ? sp : spec_to_superspec( ob );
}


/***************************************
 ***************************************/

SuperSPEC *
spec_to_superspec( FL_OBJECT * ob )
{
    SuperSPEC *spp;
    int i;
    int n;

    if ( ! ob->u_vdata )
    {
        ob->u_vdata = spp = fl_calloc( 1, sizeof *spp );

        spp->content    = NULL;
        spp->shortcut   = NULL;
        spp->callback   = NULL;
        spp->mode       = NULL;
        spp->mval       = NULL;
        spp->cspecv     = NULL;

        spp->new_menuapi = 0;
    }
    else
        spp = ob->u_vdata;

    spp->how_return = ob->how_return;

    if ( ob->objclass == FL_BROWSER )
    {
        FLI_BROWSER_SPEC *sp = ob->spec;

        spp->h_pref = sp->h_pref;
        spp->v_pref = sp->v_pref;

        for ( i = 1; i <= spp->nlines; i++ )
        {
            fl_safe_free( spp->content[ i ] );
            fl_safe_free( spp->shortcut[ i ] );
            fl_safe_free( spp->callback[ i ] );
        }

        n = fl_get_browser_maxline( ob );

        spp->content  = fl_realloc( spp->content,
                                    ( n + 1 ) * sizeof *spp->content );
        spp->shortcut = fl_realloc( spp->shortcut,
                                    ( n + 1 ) * sizeof *spp->shortcut );
        spp->callback = fl_realloc( spp->callback,
                                    ( n + 1 ) * sizeof *spp->callback );
        spp->mode     = fl_realloc( spp->mode,
                                    ( n + 1 ) * sizeof *spp->mode );
        spp->mval     = fl_realloc( spp->mval,
                                    ( n + 1 ) * sizeof *spp->mval );

        spp->nlines = n;
        for ( i = 1; i <= n; i++ )
        {
            spp->content[ i ] = fl_strdup( fl_get_browser_line( ob, i ) );
            spp->shortcut[ i ] = spp->callback[ i ] = NULL;
        }
    }
    else if ( ob->objclass == FL_CHOICE )
    {
        FLI_CHOICE_SPEC *sp = ob->spec;

        for ( i = 1; i <= spp->nlines; i++ )
        {
            fl_safe_free( spp->content[ i ] );
            fl_safe_free( spp->shortcut[ i ] );
        }

        n = spp->nlines = sp->numitems;

        spp->content  = fl_realloc( spp->content,
                                    ( n + 1 ) * sizeof *spp->content );
        spp->shortcut = fl_realloc( spp->shortcut,
                                    ( n + 1 ) * sizeof *spp->shortcut );
        spp->callback = fl_realloc( spp->callback,
                                    ( n + 1 ) * sizeof *spp->callback );
        spp->mode     = fl_realloc( spp->mode,
                                    ( n + 1 ) * sizeof *spp->mode );
        spp->mval     = fl_realloc( spp->mval,
                                    ( n + 1 ) * sizeof *spp->mval );

        spp->align   = sp->align;
        spp->int_val = sp->val;

        for ( i = 1; i <= n; i++ )
        {
            spp->mode[ i ] = sp->mode[ i ];
            spp->content[ i ] = fl_strdup( fl_get_choice_item_text( ob, i ) );

            if ( sp->shortcut[ i ] )
                spp->shortcut[ i ] = fl_strdup( sp->shortcut[ i ] );
            else
                spp->shortcut[ i ] = NULL;
        }
    }
    else if ( ob->objclass == FL_MENU )
    {
        FLI_MENU_SPEC *sp = ob->spec;

        for ( i = 1; i <= spp->nlines; i++ )
        {
            fl_safe_free( spp->content[ i ] );
            fl_safe_free( spp->shortcut[ i ] );
            fl_safe_free( spp->callback[ i ] );
        }

        n = spp->nlines = sp->numitems;

        spp->content  = fl_realloc( spp->content,
                                    ( n + 1 ) * sizeof *spp->content );
        spp->shortcut = fl_realloc( spp->shortcut,
                                    ( n + 1 ) * sizeof *spp->shortcut );
        spp->callback = fl_realloc( spp->callback,
                                    ( n + 1 ) * sizeof *spp->callback );
        spp->mode     = fl_realloc( spp->mode,
                                    ( n + 1 ) * sizeof *spp->mode );
        spp->mval     = fl_realloc( spp->mval,
                                    ( n + 1 ) * sizeof *spp->mval );

        for ( i = 1; i <= n; i++ )
        {
            spp->mode[ i ] = sp->mode[ i ];
            spp->mval[ i ] = sp->mval[ i ];
            spp->content[ i ] =
                      fl_strdup( fl_get_menu_item_text( ob, sp->mval[ i ] ) );

            if ( sp->shortcut[ i ] )
                spp->shortcut[ i ] = fl_strdup( sp->shortcut[ i ] );
            else
                spp->shortcut[ i ] = NULL;

            if ( sp->cb[ i ] )
                spp->callback[ i ] = fl_strdup( ( char * ) sp->cb[ i ] );
            else
                spp->callback[ i ] = NULL;
        }
    }
    else if (    ob->objclass == FL_SLIDER
              || ob->objclass == FL_VALSLIDER
              || ob->objclass == FL_THUMBWHEEL )
    {
        FLI_SLIDER_SPEC *sp = ob->spec;

        spp->val        = sp->val;
        spp->min        = sp->min;
        spp->max        = sp->max;
        spp->step       = sp->step;
        spp->prec       = sp->prec;
        spp->ldelta     = sp->ldelta;
        spp->rdelta     = sp->rdelta;
        spp->slsize     = sp->slsize;
    }
    else if (    ISBUTTON( ob->objclass )
              || ob->objclass == FL_PIXMAP
              || ob->objclass == FL_BITMAP )
    {
        FL_BUTTON_SPEC *sp = ob->spec;
        IconInfo *info;

        spp->mbuttons = 0;
        for ( i = 0; i < 5; i++ )
            if ( sp->react_to[ i ] )
                spp->mbuttons |= 1 << i; 

        spp->int_val = sp->val;

        if ( ! spp->cspecv )
        {
            info = spp->cspecv = fl_calloc( 1, sizeof *info );

            info->show_focus = 1;
            info->dx         = info->dy = 3;
            info->align      = FL_ALIGN_CENTER;
            info->fullpath   = 1;
        }

        info = spp->cspecv;

        if (    ob->objclass == FL_PIXMAPBUTTON
             || ob->objclass == FL_BITMAPBUTTON
             || ob->objclass == FL_PIXMAP
             || ob->objclass == FL_BITMAP )
        {
            spp->align = info->align;
            spp->dx = info->dx;
            spp->dy = info->dy;
            spp->show_focus = info->show_focus;
            spp->use_data = info->use_data;
            spp->fullpath = info->fullpath;
            strcpy( spp->filename, info->filename );
            strcpy( spp->data, info->data );
            strcpy( spp->focus_filename, info->focus_filename );
            strcpy( spp->helper, info->helper );
            strcpy( spp->focus_data, info->focus_data );
            strcpy( spp->width, info->width );
            strcpy( spp->height, info->height );
        }
    }
    else if ( ob->objclass == FL_POSITIONER )
    {
        FLI_POSITIONER_SPEC *sp = ob->spec;

        spp->xstep      = sp->xstep;
        spp->ystep      = sp->ystep;
        spp->xmin       = sp->xmin;
        spp->xmax       = sp->xmax;
        spp->xval       = sp->xval;
        spp->ymin       = sp->ymin;
        spp->ymax       = sp->ymax;
        spp->yval       = sp->yval;
    }
    else if ( ob->objclass == FL_COUNTER )
    {
        FLI_COUNTER_SPEC *sp = ob->spec;

        spp->val        = sp->val;
        spp->lstep      = sp->lstep;
        spp->sstep      = sp->sstep;
        spp->min        = sp->min;
        spp->max        = sp->max;
        spp->prec       = sp->prec;
    }
    else if ( ob->objclass == FL_SPINNER )
    {
        spp->dval = fl_get_spinner_value( ob );
        fl_get_spinner_bounds( ob, &spp->dmin, &spp->dmax );
        spp->dstep = fl_get_spinner_step( ob );
        spp->prec = fl_get_spinner_precision( ob );
    }
    else if ( ob->objclass == FL_DIAL )
    {
        FLI_DIAL_SPEC *sp = ob->spec;

        spp->min        = sp->min;
        spp->max        = sp->max;
        spp->val        = sp->val;
        spp->step       = sp->step;
        spp->thetai     = sp->thetai;
        spp->thetaf     = sp->thetaf;
        spp->direction  = sp->direction;
    }
    else if ( ob->objclass == FL_XYPLOT )
    {
        FLI_XYPLOT_SPEC *sp = ob->spec;

        spp->xmajor         = sp->xmajor;
        spp->xminor         = sp->xminor;
        spp->ymajor         = sp->ymajor;
        spp->yminor         = sp->yminor;
        spp->xscale         = sp->xscale;
        spp->yscale         = sp->yscale;
        spp->xgrid          = sp->xgrid;
        spp->ygrid          = sp->ygrid;
        spp->grid_linestyle = sp->grid_linestyle;
        spp->xbase          = sp->xbase;
        spp->ybase          = sp->ybase;
        spp->mark_active    = sp->mark_active;
    }
    else if ( ob->objclass == FL_SCROLLBAR )
    {
        FLI_SCROLLBAR_SPEC *scbsp = ob->spec;
        FLI_SLIDER_SPEC *sp = scbsp->slider->spec;

        spp->val        = sp->val;
        spp->min        = sp->min;
        spp->max        = sp->max;
        spp->prec       = sp->prec;
        spp->step       = sp->step;
        spp->slsize     = sp->slsize;
        spp->ldelta     = sp->ldelta;
        spp->rdelta     = sp->rdelta;
    }

    return spp;
}


/***************************************
 ***************************************/

void *
superspec_to_spec( FL_OBJECT * ob )
{
    void *v = ob->spec;
    SuperSPEC *spp = ob->u_vdata;
    int i = 0;

    if ( ! spp )
        return v;

    if ( ob->objclass == FL_BROWSER )
    {
        FLI_BROWSER_SPEC *sp = ob->spec;

        fl_clear_browser( ob );

        sp->h_pref = spp->h_pref;
        sp->v_pref = spp->v_pref;

        for ( i = 1; i <= spp->nlines; i++ )
            fl_addto_browser( ob, spp->content[ i ] );
    }
    else if ( ob->objclass == FL_CHOICE )
    {
        fl_clear_choice( ob );

        ( ( FLI_CHOICE_SPEC * ) ob->spec)->align = spp->align;

        for ( i = 1; i <= spp->nlines; i++ )
        {
            fl_addto_choice( ob, spp->content[ i ] );
            fl_set_choice_item_mode( ob, i, spp->mode[ i ] );
            if ( spp->shortcut[ i ] )
                fl_set_choice_item_shortcut( ob, i, spp->shortcut[ i ] );
        }

        if ( spp->nlines >= spp->int_val )
            fl_set_choice( ob, spp->int_val );
    }
    else if ( ob->objclass == FL_MENU )
    {
        fl_clear_menu( ob );

        for ( i = 1; i <= spp->nlines; i++ )
        {
            fl_addto_menu( ob, spp->content[ i ] );
            fl_set_menu_item_mode( ob, i, spp->mode[ i ] );
            if ( spp->shortcut[ i ] )
                fl_set_menu_item_shortcut( ob, i, spp->shortcut[ i ] );
            if ( spp->callback[ i ] )
                fl_set_menu_item_callback( ob, i,
                               ( FL_PUP_CB ) fl_strdup( spp->callback[ i ] ) );
            if ( spp->mval[ i ] != i )
                fl_set_menu_item_id( ob, i, spp->mval[ i ] );
        }
    }
    else if (    ob->objclass == FL_SLIDER
              || ob->objclass == FL_VALSLIDER
              || ob->objclass == FL_THUMBWHEEL)
    {
        FLI_SLIDER_SPEC *sp = ob->spec;

        sp->val    = spp->val;
        sp->min    = spp->min;
        sp->max    = spp->max;
        sp->step   = spp->step;
        sp->prec   = spp->prec;
        sp->ldelta = spp->ldelta;
        sp->rdelta = spp->rdelta;
        sp->slsize = spp->slsize;
    }
    else if (    ISBUTTON(ob->objclass)
              || ob->objclass == FL_PIXMAP
              || ob->objclass == FL_BITMAP )
    {
        FL_BUTTON_SPEC *sp = ob->spec;
        IconInfo *info = spp->cspecv;

        for ( i = 0; i < 5; i++ )
            sp->react_to[ i ] = ( spp->mbuttons & ( 1 << i ) ) != 0;
        if ( ISBUTTON( ob->objclass ) )
            fl_set_button_mouse_buttons( ob, spp->mbuttons );

        sp->val = spp->int_val;
        if ( ISBUTTON( ob->objclass ) )
            fl_set_button( ob, sp->val );

        if (    ob->objclass == FL_PIXMAPBUTTON
             || ob->objclass == FL_BITMAPBUTTON
             || ob->objclass == FL_PIXMAP
             || ob->objclass == FL_BITMAP )
        {
            info->align      = spp->align;
            info->dx         = spp->dx;
            info->dy         = spp->dy;
            info->show_focus = spp->show_focus;
            info->use_data   = spp->use_data;
            info->fullpath   = spp->fullpath;

            strcpy( info->filename, spp->filename );
            strcpy( info->data, spp->data );
            strcpy( info->focus_filename, spp->focus_filename );
            strcpy( info->helper, spp->helper );
            strcpy( info->focus_data, spp->focus_data );
            strcpy( info->width, spp->width );
            strcpy( info->height, spp->height );

            if ( ob->objclass == FL_PIXMAPBUTTON || ob->objclass == FL_PIXMAP )
            {
                fl_set_pixmap_align( ob, info->align | FL_ALIGN_INSIDE,
                                     info->dx, info->dy );
                fl_set_pixmapbutton_focus_outline( ob, info->show_focus );
            }

            if ( *info->filename )
            {
                if (    ob->objclass == FL_PIXMAPBUTTON
                     || ob->objclass == FL_PIXMAP )
                {
                    fl_set_pixmap_file( ob, info->filename );
                    if ( *info->focus_filename )
                        fl_set_pixmapbutton_focus_file( ob,
                                                        info->focus_filename );
                }
                else
                    fl_set_bitmap_file( ob, info->filename );
            }
        }

        if ( info->helper[ 0 ] )
            fl_set_object_helper( ob, get_helper( info->helper ) );
    }
    else if ( ob->objclass == FL_POSITIONER )
    {
        FLI_POSITIONER_SPEC *sp = ob->spec;

        sp->xstep      = spp->xstep;
        sp->ystep      = spp->ystep;
        sp->xmin       = spp->xmin;
        sp->xmax       = spp->xmax;
        sp->xval       = spp->xval;
        sp->ymin       = spp->ymin;
        sp->ymax       = spp->ymax;
        sp->yval       = spp->yval;
    }
    else if ( ob->objclass == FL_COUNTER )
    {
        FLI_COUNTER_SPEC *sp = ob->spec;

        sp->val        = spp->val;
        sp->sstep      = spp->sstep;
        sp->lstep      = spp->lstep;
        sp->min        = spp->min;
        sp->max        = spp->max;
        sp->prec       = spp->prec;
    }
    else if ( ob->objclass == FL_SPINNER )
    {
        fl_set_spinner_value( ob, spp->dval );
        fl_set_spinner_bounds( ob, spp->dmin, spp->dmax );
        fl_set_spinner_step( ob, spp->dstep );
        fl_set_spinner_precision( ob, spp->prec );
    }
    else if ( ob->objclass == FL_DIAL )
    {
        FLI_DIAL_SPEC *sp = ob->spec;

        sp->min        = spp->min;
        sp->max        = spp->max;
        sp->val        = spp->val;
        sp->step       = spp->step;
        sp->thetai     = spp->thetai;
        sp->thetaf     = spp->thetaf;
        sp->direction  = spp->direction;
    }
    else if ( ob->objclass == FL_XYPLOT )
    {
        FLI_XYPLOT_SPEC *sp = ob->spec;

        sp->xmajor         = spp->xmajor;
        sp->xminor         = spp->xminor;
        sp->ymajor         = spp->ymajor;
        sp->yminor         = spp->yminor;
        sp->xscale         = spp->xscale;
        sp->yscale         = spp->yscale;
        sp->xgrid          = spp->xgrid;
        sp->ygrid          = spp->ygrid;
        sp->xbase          = spp->xbase;
        sp->ybase          = spp->ybase;
        sp->grid_linestyle = spp->grid_linestyle;
        sp->mark_active    = spp->mark_active;
    }
    else if ( ob->objclass == FL_SCROLLBAR )
    {
        FLI_SCROLLBAR_SPEC *scbsp = ob->spec;
        FLI_SLIDER_SPEC *sp = scbsp->slider->spec;

        sp->val       = spp->val;
        sp->min       = spp->min;
        sp->max       = spp->max;
        sp->prec       = spp->prec;
        sp->step       = spp->step;
        sp->slsize     = spp->slsize;
        sp->ldelta     = spp->ldelta;
        sp->rdelta     = spp->rdelta;
    }

    return v;
}


static int keep_content = 1;

/***************************************
 ***************************************/

void
copy_superspec( FL_OBJECT * target,
                FL_OBJECT * src )
{
    SuperSPEC *t,
              *s;
    void *tmp;
    int i;

    t = get_superspec( target );
    s = get_superspec( src );

    if ( ! t || ! s )
    {
        M_err( "CopySuperSPEC", "null spec" );
        return;
    }

    tmp = t->cspecv;
    *t = *s;
    t->cspecv = tmp;

    if ( ! keep_content )
    {
        for ( i = 1; i <= t->nlines; i++ )
        {
            t->content[ i ]  = NULL;
            t->mode[ i ]     = 0;
            t->shortcut[ i ] = NULL;
        }

        t->nlines = 0;
    }
    else
    {
        if ( s->nlines )
        {
            if ( s->mode )
                t->mode = fl_calloc( s->nlines + 1, sizeof *t->mode );
            if ( s->content )
                t->content = fl_calloc( s->nlines + 1, sizeof * t->content);
            if ( s->shortcut )
                t->shortcut = fl_calloc( s->nlines + 1, sizeof *t->shortcut );
        }

        for ( i = 1; i <= s->nlines; i++ )
        {
            t->mode[ i ]    = s->mode[ i ];
            t->content[ i ] = fl_strdup( s->content[ i ] );

            if ( s->shortcut[ i ] )
                t->shortcut[ i ] = fl_strdup( s->shortcut[ i ] );
        }

        if ( t->cspecv && s->cspecv && t->cspecv_size )
            memcpy( t->cspecv, s->cspecv, t->cspecv_size );
    }
    
    superspec_to_spec( target );
}


/*
 * Local variables:
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
