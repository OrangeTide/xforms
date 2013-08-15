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
 * \file fd_attribs.c
 *
 *  This file is part of XForms package
 *  Copyright (c) 1996-2002  T.C. Zhao and Mark Overmars
 *  All rights reserved.
 *
 * It contains some routines to deal with attributes of the objects.
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "include/forms.h"
#include "fd_main.h"
#include "private/flsnprintf.h"


/****************** FORMS AND CALL-BACKS ************************/

int auto_apply = 1;

static void readback_attributes( FL_OBJECT * );


/***************************************
 * Callback routine to get a color from the user
 ***************************************/

void
setcolor_cb( FL_OBJECT * obj,
             long        arg  FL_UNUSED_ARG )
{
    int col1 = fl_show_colormap( obj->col1 );

    fl_set_object_color( obj, col1, col1 );
    auto_apply_cb( obj, 0 );
}


/***************************************
 * If we're to change attributes automatically
 ***************************************/

void
auto_apply_cb( FL_OBJECT * ob,
               long        arg )
{
    if ( auto_apply )
        apply_cb( ob, arg );
}


/***************************************
 * The auto-apply setting itself
 ***************************************/

void
autoapply_setting_cb( FL_OBJECT * ob,
                      long        arg  FL_UNUSED_ARG )
{
    if ( ! ( auto_apply = fl_get_button( ob ) ) )
        fl_show_object( fd_attrib->applyobj );
    else
        fl_hide_object( fd_attrib->applyobj );
}


/******* For cancel and restore operation *******{*/

static FL_OBJECT *oldcopy = NULL,      /* object being changed */
                 *curobj;
static char oldname[ MAX_VAR_LEN ];
static char oldcbname[ MAX_VAR_LEN ],
            oldargname[ MAX_VAR_LEN ];


/***************************************
 ***************************************/

static void
save_object( FL_OBJECT * obj )
{
    char *ol;
    long *os;
    FL_OBJECT *tmp;

    get_object_name( obj, oldname, oldcbname, oldargname );

    if ( ! oldcopy )
        oldcopy = fl_make_object( 0, 0, 0, 0, 0, 0, NULL, NULL );

    ol                = oldcopy->label;
    os                = oldcopy->shortcut;
    *oldcopy          = *obj;
    oldcopy->label    = ol;
    oldcopy->shortcut = os;

    fl_set_object_label( oldcopy, obj->label );

    for ( tmp = obj->child; tmp; tmp = tmp->nc )
        tmp->parent = obj;
}


/***************************************
 * Duplicate everything in oldobj to obj
 ***************************************/

static void
restore_object( FL_OBJECT * obj )
{
    char *ol = obj->label;
    long *os = obj->shortcut;
    FL_OBJECT *tmp;
    void *spec;
    FL_OBJECT *child,
              *prev,
              *next;

    if ( obj->type != oldcopy->type )
        change_type( obj, oldcopy->type );

    child = obj->child;
    prev  = obj->prev;
    next  = obj->next;
    spec = obj->spec;

    set_object_name( obj, oldname, oldcbname, oldargname );
    *obj          = *oldcopy;
    obj->child    = child;
    for ( tmp = child; tmp; tmp = tmp->nc )
        tmp->parent = obj;
    obj->spec     = spec;
    obj->prev     = prev;
    obj->next     = next;
    obj->label    = ol;
    obj->shortcut = os;
    fl_set_object_label( obj, oldcopy->label );

    fli_handle_object( obj, FL_ATTRIB, 0, 0, 0, NULL, 0 );
}


/********** End of cancel/restore ********}***/


/***************************************
 * Really change object attributes
 ***************************************/

void
apply_cb( FL_OBJECT * obj  FL_UNUSED_ARG,
          long        arg  FL_UNUSED_ARG )
{
    readback_attributes( curobj );
    change_selected_objects( curobj );
    redraw_the_form( 0 );
}


/***************************************
 * Restore from the original copy
 ***************************************/

void
restore_cb( FL_OBJECT * ob    FL_UNUSED_ARG,
            long        data  FL_UNUSED_ARG )
{
    restore_object( curobj );
    show_attributes( curobj );
    change_selected_objects( curobj );
    redraw_the_form( 0 );
}


/****************** GLOBAL INITIALIZATION  ************************/

static FL_OBJECT *fnts;


/***************************************
 ***************************************/

static void
add_font_choice( const char * p )
{
    fl_addto_choice( fnts, p );
}


/* Font sizes. Need to do this because of symbolic names */

typedef struct {
    int    size;
    char * name,
         * sc;
} Fsizes;

static Fsizes fsizes[ ] =
{
    { FL_TINY_SIZE,   "Tiny",     "Tt#t" },
    { FL_SMALL_SIZE,  "Small",    "Ss#s" },
    { FL_NORMAL_SIZE, "Normal",   "Nn#n" },
    { FL_MEDIUM_SIZE, "Medium",   "Mm#m" },
    { FL_LARGE_SIZE,  "Large",    "Ll#l" },
    { FL_HUGE_SIZE,   "Huge",     "Hh#h" },
    { 11,             "Variable", ""     }
};

#define NFSIZE ( sizeof fsizes / sizeof *fsizes )


/***************************************
 ***************************************/

static void
attrib_init( FD_generic_attrib * ui )
{
    static int attrib_initialized;
    int i;
    char buf[ 64 ];
    VN_pair *vp;

    if ( attrib_initialized )
        return;

    attrib_initialized = 1;

    fl_clear_choice( ui->boxobj );
    for ( i = 1, vp = vn_btype; vp->val >= 0; vp++, i++ )
    {
        fl_addto_choice( ui->boxobj, vp->shown );
        fl_set_choice_item_shortcut( ui->boxobj, i, vp->hotkey );
    }

    fl_set_object_return( ui->nameobj, FL_RETURN_END );
    fl_set_object_return( ui->cbnameobj, FL_RETURN_END );

    /* resize */

    fl_set_choice_fontsize( ui->resize, fd_align_fontsize );
    for ( vp = vn_resize; vp->val >= 0; vp++ )
        fl_addto_choice( ui->resize, vp->name + 3 );

    /* gravity. Due to compatibilities issues, there are more than need in
       vn_gravity */

    for ( i = 0, vp = vn_gravity; vp->val >= 0 && i < 9; vp++, i++ )
    {
        fl_addto_choice( ui->nwgravity, vp->name + 3 );
        fl_addto_choice( ui->segravity, vp->name + 3 );
    }

    /* align (only show the first 9 elements of 'vn_align' the rest is
       in there only for backward compatibility reasons when reading in
       a file) */

    fl_set_choice_fontsize( ui->align, fd_align_fontsize );
    for ( vp = vn_align, i = 0; vp->val >= 0 && i < 9; vp++, i++ )
        fl_addto_choice( ui->align, vp->name + 9 );
    fl_addto_choice( ui->inside, "Inside|Outside" );

    /* font stuff */

    fnts = ui->fontobj;
    fl_enumerate_fonts( add_font_choice, 1 );
    fl_addto_choice( ui->styleobj, "Normal|Shadow|Engraved|Embossed" );

    /* size */

    for ( i = 0; i < ( int ) NFSIZE; i++ )
    {
        if ( fsizes[ i ].size == FL_NORMAL_SIZE )
        {
            fsizes[ i ].name = "Normal";
            fsizes[ i ].sc = "Nn#n";
        }

        sprintf( buf, "%2d  %s%%r1", fsizes[ i ].size, fsizes[ i ].name );
        fl_addto_choice( ui->sizeobj, buf );
        fl_set_choice_item_shortcut( ui->sizeobj, i + 1, fsizes[ i ].sc );
    }

}

/* Check for obvious errors */

#define OK_letter( c )    (    *c == '_'                       \
                            || *c == '['                       \
                            || *c == ']'                       \
                            || * c== '.'                       \
                            || ( *c == ':' && *++c == ':' )    \
                            || ( *c == '-' && *++c == '>' ) )


/***************************************
 ***************************************/

static int
valid_c_identifier( const char * s )
{
    if ( fdopt.lax )
        return 1;

    /* Empty is considered to be valid */

    if ( ! s || ! *s || ( *s == ' ' && *( s + 1 ) == '\0' ) )
        return 1;

    if ( ! isalpha( ( unsigned char ) *s ) && *s != '_' )
        return 0;

    for ( s++; *s; s++ )
        if ( ! isalnum( ( unsigned char ) *s ) && ! OK_letter( s ) )
            return 0;

    return 1;
}


/***************************************
 ***************************************/

static int
validate_cvar_name( FL_OBJECT * obj,
                    const char * w )
{
    const char *s = fl_get_input( obj );

    if ( ! valid_c_identifier( s ) )
    {
        char *m;

        if ( ! w || ! *w )
            m = fl_strdup( "Invalid C identifier:" );
        else
            m = fli_print_to_string( "Invalid C identifier specified for %s:",
                                     w );

        fl_show_alert( "Error", m, s, 0 );
        fl_free( m );
        fl_set_focus_object( obj->form, obj );
        return 0;
    }

    return 1;
}


/***************************************
 ***************************************/

static int
validate_attributes( void )
{
    return    validate_cvar_name( fd_generic_attrib->nameobj, "object name" )
           && validate_cvar_name( fd_generic_attrib->cbnameobj, "callback" );
}


/***************************************
 ***************************************/

void
validate_cvar_name_cb( FL_OBJECT * obj,
                       long        data )
{
    validate_cvar_name( obj, data == 0 ? "object name" : "callback" );
}


/********************* THE ACTUAL ROUTINES ****************************/

/***************************************
 ***************************************/

static void
readback_attributes( FL_OBJECT * obj )
{
    int spstyle;
    char name[ 128],
         cbname[ 128 ];
    char tmpbuf[ 128 ];

    obj->boxtype = fl_get_choice( fd_generic_attrib->boxobj ) - 1;

    /* Label style consists of two parts */

    obj->lstyle = fl_get_choice( fd_generic_attrib->fontobj ) - 1;
    spstyle = fl_get_choice( fd_generic_attrib->styleobj ) - 1;
    obj->lstyle +=
              spstyle == 3 ? FL_EMBOSSED_STYLE : ( spstyle * FL_SHADOW_STYLE );
    obj->col1 = fd_generic_attrib->col1obj->col1;
    obj->col2 = fd_generic_attrib->col2obj->col1;
    obj->lcol = fd_generic_attrib->lcolobj->col1;

    fl_snprintf( tmpbuf, sizeof tmpbuf, "FL_ALIGN_%s",
                 fl_get_choice_text( fd_generic_attrib->align ) );
    obj->align = align_val( tmpbuf );

    if (    fl_get_choice( fd_generic_attrib->inside ) == 1
         && ! fl_is_center_lalign( obj->align ) )
        obj->align = fl_to_inside_lalign( obj->align );
    else
        obj->align = fl_to_outside_lalign( obj->align );

    fl_snprintf( tmpbuf, sizeof tmpbuf, "FL_%s",
                 fl_get_choice_text( fd_generic_attrib->resize ) );
    obj->resize = resize_val( tmpbuf );

    fl_snprintf( tmpbuf, sizeof tmpbuf, "FL_%s",
                 fl_get_choice_text( fd_generic_attrib->segravity ) );
    obj->segravity = gravity_val( tmpbuf );

    fl_snprintf( tmpbuf, sizeof tmpbuf, "FL_%s",
                 fl_get_choice_text( fd_generic_attrib->nwgravity ) );
    obj->nwgravity = gravity_val( tmpbuf );

    obj->lsize = fsizes[ fl_get_choice( fd_generic_attrib->sizeobj ) - 1 ].size;

    set_label( obj, fl_get_input( fd_generic_attrib->labelobj ) );
    set_shortcut( obj, fl_get_input( fd_generic_attrib->scobj ) );

    fli_sstrcpy( name, fl_get_input( fd_generic_attrib->nameobj ),
                 sizeof name );
    fli_sstrcpy( cbname, fl_get_input( fd_generic_attrib->cbnameobj ),
                 sizeof cbname );

    if ( ! valid_c_identifier( name ) )
        *name = '\0';

    if ( ! valid_c_identifier( cbname ) )
        *cbname = '\0';

    set_object_name( obj, name, cbname,
                     fl_get_input( fd_generic_attrib->argobj ) );

    /* Change type need to be the last call as it may create objects based on
       the current object, which need to have the latest attributes */

    if ( obj->objclass == FL_BOX )
        change_type( obj, obj->boxtype );
    else
        change_type( obj, fl_get_choice( fd_generic_attrib->typeobj ) - 1 );
}


/***************************************
 ***************************************/

void
show_attributes( const FL_OBJECT * obj )
{
    char objname[ MAX_VAR_LEN ],
         cbname[ MAX_VAR_LEN ],
         argname[ MAX_VAR_LEN ];
    char buf[ MAX_VAR_LEN ];
    char *label;
    int i,
        lstyle,
        spstyle,
        oksize,
        align = fl_to_outside_lalign( obj->align );
    static char othersize[ 32 ];

    fl_freeze_form( fd_generic_attrib->generic_attrib );

    /* Fill in list of types */

    fl_clear_choice( fd_generic_attrib->typeobj );
    fl_set_choice_fontsize( fd_generic_attrib->typeobj, fd_type_fontsize );

    if ( obj->objclass != FL_BOX )
    {
        for ( i = 0; i < find_class_maxtype( obj->objclass ); i++ )
        {
            strcat( strcpy( buf, find_type_name( obj->objclass, i ) ), "%r1" );
            fl_addto_choice( fd_generic_attrib->typeobj, buf );
        }

        fl_set_choice( fd_generic_attrib->typeobj, obj->type + 1 );
    }

    /* Fill in settings */

    fl_set_choice( fd_generic_attrib->boxobj, obj->boxtype + 1 );
    fl_set_choice_text( fd_generic_attrib->align, align_name( align, 0 ) + 9 );
    fl_set_choice( fd_generic_attrib->inside, ( obj->align == align ) + 1 );

    lstyle = obj->lstyle % FL_SHADOW_STYLE;
    spstyle = obj->lstyle / FL_SHADOW_STYLE;

    if ( spstyle >= 3 )
        spstyle = 3;

    fl_set_choice( fd_generic_attrib->fontobj, lstyle + 1 );
    fl_set_choice( fd_generic_attrib->styleobj, spstyle + 1 );

    for ( oksize = i = 0; !oksize && i < ( int ) NFSIZE; i++ )
        if ( ( oksize = obj->lsize == fsizes[ i ].size ))
            fl_set_choice( fd_generic_attrib->sizeobj, i + 1 );

    if ( ! oksize )
    {
        sprintf( othersize, "%d (Variable)", obj->lsize );
        fsizes[ NFSIZE - 1 ].size = obj->lsize;
        fsizes[ NFSIZE - 1 ].name = othersize;
        fl_replace_choice( fd_generic_attrib->sizeobj, NFSIZE, othersize );
        fl_set_choice( fd_generic_attrib->sizeobj, NFSIZE );
    }

    /* gravity stuff */

    fl_set_choice_text( fd_generic_attrib->resize,
                        resize_name( obj->resize ) + 3 );
    fl_set_choice_text( fd_generic_attrib->nwgravity,
                        gravity_name( obj->nwgravity ) + 3 );
    fl_set_choice_text( fd_generic_attrib->segravity,
                        gravity_name( obj->segravity ) + 3 );

    get_object_name( obj, objname, cbname, argname );
    label = get_label( obj, 0 );
    fl_set_input( fd_generic_attrib->labelobj, label );
    fl_free( label );
    fl_set_input( fd_generic_attrib->nameobj, objname );
    fl_set_input( fd_generic_attrib->cbnameobj, cbname );
    fl_set_input( fd_generic_attrib->argobj, argname );

    fl_set_input( fd_generic_attrib->scobj, get_shortcut_string( obj ) );

    fl_set_object_color( fd_generic_attrib->col1obj, obj->col1, obj->col1 );
    fl_set_object_color( fd_generic_attrib->col2obj, obj->col2, obj->col2 );
    fl_set_object_color( fd_generic_attrib->lcolobj, obj->lcol, obj->lcol );
    fl_unfreeze_form( fd_generic_attrib->generic_attrib );
}


/***************************************
 * Displays the form to change the attributes. 'all' indicates
 * whether all label, name, etc. should also be changed
 ***************************************/

int
change_object( FL_OBJECT * obj,
               int         all )
{
    FL_OBJECT *retobj;
    FD_generic_attrib *ui = fd_generic_attrib;
    FL_FORM * spec_form = NULL;

    attrib_init( fd_generic_attrib );

    /* Save current attributes for later restore */

    curobj = obj;
    save_object( obj );

    /* Show only required parts */

    if ( all )
    {
        fl_show_object( ui->labelobj  );
        fl_show_object( ui->scobj     );
        fl_show_object( ui->nameobj   );
        fl_show_object( ui->cbnameobj );
        fl_show_object( ui->argobj    );
    }
    else
    {
        fl_hide_object( ui->labelobj  );
        fl_hide_object( ui->scobj     );
        fl_hide_object( ui->nameobj   );
        fl_hide_object( ui->cbnameobj );
        fl_hide_object( ui->argobj    );
        spec_form = fl_get_tabfolder_folder_bynumber( fd_attrib->attrib_folder,
                                                      2 );
        fl_delete_folder( fd_attrib->attrib_folder, spec_form );
    }

    /* Show attributes of the current object */

    show_attributes( obj );

    /* Do interaction */

    fl_deactivate_all_forms( );

    /* Disable selection */

    no_selection = 1;

    /* Always come up with Generic */

    fl_set_folder_bynumber( fd_attrib->attrib_folder, 1 );

    if ( fd_attrib->attrib->y < 55 )
        fd_attrib->attrib->y = 25;

    fl_show_form( fd_attrib->attrib, FL_PLACE_GEOMETRY, FL_FULLBORDER,
                  "Attributes" );
    fl_set_app_mainform( fd_attrib->attrib );

    /* Both cancel and readyobj should have their own callbacks, so we don't
       need to call fl_do_forms(), but since attribute editing can't be
       invoked for more than one item at a time we need to block the
       proces_xevent. TODO */

    do
    {
        XEvent xev;

        retobj = fl_do_forms( );
        if ( retobj == FL_EVENT )
            fl_XNextEvent( &xev );
    } while ( ! (    ( retobj == fd_attrib->readyobj && validate_attributes( ) )
                  || retobj == fd_attrib->cancelobj ) );

    if ( spec_form )
        fl_addto_tabfolder( fd_attrib->attrib_folder, "Spec", spec_form );
    fl_set_app_mainform( fd_control->control );
    fl_hide_form( fd_attrib->attrib );
    fl_activate_all_forms( );

    no_selection = 0;

    if ( retobj == fd_attrib->cancelobj )
    {
        restore_object( obj );
        redraw_the_form( 0 );
        return FL_FALSE;
    }
    else
    {
        readback_attributes( obj );
        return FL_TRUE;
    }
}


/***************************************
 * Sets the attributes of an object
 ***************************************/

void
set_attribs( FL_OBJECT  * obj,
             int          boxtype,
             int          col1,
             int          col2,
             int          lcol,
             int          align,
             int          lsize,
             int          lstyle,
             const char * label )
{
    char *s;

    obj->boxtype = boxtype;
    obj->col1    = col1;
    obj->col2    = col2;
    obj->lcol    = lcol;
    obj->align   = align;
    obj->lsize   = lsize;
    obj->lstyle  = lstyle;

    if ( ( s = strchr( label, '\010' ) ) )
        memmove( s, s + 1, strlen( s ) + 1 );
        
    fl_set_object_label( obj, label );

    fli_handle_object( obj, FL_ATTRIB, 0, 0, 0, NULL, 0 );

    /* Some extra adjustments for spinner objects (this is a hack but
       avoiding it would require a complete change of how fdesign works) */

    if ( obj->objclass == FL_SPINNER )
    {
        FL_OBJECT *subobj = fl_get_spinner_input( obj );

        subobj->col1 = col1;
        subobj->col2 = col2;

        subobj->lstyle = lstyle;
        subobj->lsize  = lsize; 

        fli_handle_object( subobj, FL_ATTRIB, 0, 0, 0, NULL, 0 );
   }
}


/***************************************
 * More attributes
 ***************************************/

void
set_miscattribs( FL_OBJECT    * obj,
                 unsigned int   nw,
                 unsigned int   se,
                 unsigned int   re )
{
    obj->nwgravity = nw;
    obj->segravity = se;
    obj->resize    = re;
}


#define NL  10


/***************************************
 * Sets the label, turning \n into NL
 ***************************************/

void
set_label( FL_OBJECT  * obj,
           const char * str )
{
    int i = 0,
        j = 0;
    char *tmpstr = fl_malloc( strlen( str ) + 1 );

    *tmpstr = '\0';

    do
    {
        if ( str[ i ] == '\\' && str[ i + 1 ] == 'n' )
        {
            tmpstr[ j++ ] = NL;
            i++;
        }
        else if ( str[ i ] == '\\' && strncmp( str + i + 1, "010", 3 ) == 0 )
        {
            if ( ! obj->shortcut || ! *obj->shortcut )
                tmpstr[ j++ ] = *fl_ul_magic_char;
            i += 3;
        }
        else
            tmpstr[ j++ ] = str[ i ];
    } while ( str[ i++ ] );

    fl_set_object_label( obj, tmpstr );
    fl_free( tmpstr );
}


/***************************************
 ***************************************/

void
set_shortcut( FL_OBJECT  * obj,
              const char * sc )
{
    if (    obj->type != FL_RETURN_BUTTON
         && obj->type != FL_HIDDEN_RET_BUTTON )
        fl_set_object_shortcut( obj, sc, 1 );
}


/* if \ preceeds c, \ does not need quote */

#define Ok( c ) \
    ( c== '"' || c== '\\' || c == 't' || c == 'n' \
      || isdigit( ( unsigned char )  c ) )


/***************************************
 * Decide if label need quotes ('\')
 ***************************************/

static int
need_quote( const char * s,
            int          i )
{
    int c = s[ i ],
        p,
        n;

    p =  i ? s[ i - 1 ] : 0;    /* prev char */
    n = *s ? s[ i + 1 ] : 0;    /* next char */

    if ( c == '"' && p != '\\' )
        return 1;
    else if ( c == '\\' && p != '\\' )
        return ! isdigit( ( unsigned char ) n ) && ! Ok( n );
    else
        return 0;
}


/***************************************
 * Read the label, turning NL into \n
 ***************************************/

char *
get_label( const FL_OBJECT * obj,
           int               c_source )
{
    int i = 0,
        j = 0;
    const char *label = obj->label;
    int len = strlen( label );
    int tlen = len + 1;
    char *tmpstr = fl_malloc( tlen );

    for ( i = 0; i < len; i++ )
    {
        if ( label[ i ] == NL )
        {
            tmpstr = fl_realloc( tmpstr, tlen += 1 );
            tmpstr[ j++ ] = '\\';
            tmpstr[ j++ ] = 'n';
        }
        else if ( label[ i ] == *fl_ul_magic_char )
        {
            if ( ! obj->shortcut || ! *obj->shortcut )
            {
                tmpstr = fl_realloc( tmpstr, tlen += 3 );
                tmpstr[ j++ ] = '\\';
                tmpstr[ j++ ] = '0';
                tmpstr[ j++ ] = '1';
                tmpstr[ j++ ] = '0';
            }
        }
        else if ( c_source && need_quote( label, i ) )
        {
            tmpstr = fl_realloc( tmpstr, tlen += 1 );
            tmpstr[ j++ ] = '\\';
            tmpstr[ j++ ] = label[ i ];
        }
        else
            tmpstr[ j++ ] = label[ i ];
    }

    tmpstr[ j ] = '\0';

    return tmpstr;
}


/***************************************
 * Convert shortcut into string representation.
 * ESC -> ^[, F1 -> &1 etc.
 ***************************************/

static int
special_key( int    key,
             char * outbuf )
{
    char *start = outbuf;

    if ( key >= XK_F1 && key <= XK_F30 )
    {
        int p = ( key - XK_F1 + 1 ) / 10,
            q = ( key - XK_F1 + 1 ) % 10;

        *outbuf++ = '&';
        if ( p )
            *outbuf++ = '0' + p;
        *outbuf++ = '0' + q;
    }
    else if ( IsUp( key ) )
    {
        *outbuf++ = '&';
        *outbuf++ = 'A';
    }
    else if ( IsDown( key ) )
    {
        *outbuf++ = '&';
        *outbuf++ = 'B';
    }
    else if ( IsRight( key ) )
    {
        *outbuf++ = '&';
        *outbuf++ = 'C';
    }
    else if ( IsLeft( key ) )
    {
        *outbuf++ = '&';
        *outbuf++ = 'D';
    }
    else
        *outbuf++ = key;

    *outbuf = '\0';

    return outbuf - start;
}


/***************************************
 ***************************************/

char *
get_shortcut_string( const FL_OBJECT * obj )
{
    static char tmps[ 127 ];
    char *p = tmps;
    long *sc = obj->shortcut;
    int n;

    for ( *p = '\0'; sc && *sc; sc++ )
    {
        if ( *sc >= FL_ALT_MASK )
        {
            *p++ = '#';
            n = special_key( *sc - FL_ALT_MASK, p );
            p += n;
        }
        else if ( *sc == '#' || *sc == '&' || *sc == '^' )  /* prefixed w/ ^ */
        {
            *p++ = '^';
            *p++ = *sc;
        }
        else if ( *sc < 30 )
        {
            *p++ = '^';
            if ( *sc <= 'Z' )
                *p++ = 'A' + *sc - 1;
            else if ( *sc == 27 )   /* Escape */
                *p++ = '[';
        }
        else if ( *sc > 255 )
        {
            n = special_key( *sc, p );
            p += n;
        }
        else
            *p++ = *sc;
    }

    *p = '\0';

    return tmps;
}


/***************************************
 * Makes a copy of the object. Only if 'exact' is set
 * the objects name is copied.
 ***************************************/

FL_OBJECT *
copy_object( FL_OBJECT * obj,
             int         exact )
{
    char name[ MAX_VAR_LEN ],
         cbname[ MAX_VAR_LEN ],
         argname[ MAX_VAR_LEN ];
    FL_OBJECT *obj2;

    obj2 = add_an_object( obj->objclass, obj->type, obj->x, obj->y,
                          obj->w, obj->h );
    get_object_name( obj, name, cbname, argname );
    set_object_name( obj2, exact ? name : "", cbname, argname );

    set_attribs( obj2, obj->boxtype, obj->col1, obj->col2,
                 obj->lcol, obj->align, obj->lsize, obj->lstyle, obj->label );

    set_miscattribs( obj2, obj->nwgravity, obj->segravity, obj->resize );

    /* Also copy the object specific info */

    copy_superspec( obj2, obj );

    fl_delete_object( obj2 );

    return obj2;
}


/***************************************
 * Changes the type of an object by reconstructing it. A quite nasty
 * procedure that delves into the form structure in a bad way.
 * And it looks a lot like a memory leak...
 ***************************************/

void
change_type( FL_OBJECT * obj,
             int         type )
{
    FL_OBJECT *ttt,
              *defobj,
              *prev;
    FL_FORM *form;
    int boxtype,
        is_focus;
    SuperSPEC *sp = obj->u_vdata;
    long int *shct = NULL;

    if ( obj->type == type )
        return;

    if ( obj->shortcut )
    {
        size_t i = 0;

        while ( obj->shortcut[ i++ ] )
            /* empty */ ;

        if ( i )
        {
            shct = malloc( i * sizeof *shct );
            memcpy( shct, obj->shortcut, i * sizeof *shct );
        }
    }

    /* Create a new object. */

    ttt = add_an_object( obj->objclass, type, obj->x, obj->y, obj->w, obj->h );

    /* Remove it from the form */

    fl_delete_object( ttt );

    /* Create a default object from which we can test if the user has changed
       boxtype and other attributes. This is done primarily to get around the
       type change problem with types having different default boxtype. Don't
       need to free the defobj as it is managed by find_class_default */

    defobj = find_class_default( obj->objclass, obj->type );

    if ( defobj->boxtype != obj->boxtype )
        boxtype = obj->boxtype;
    else
        boxtype = ttt->boxtype;

    /* Set the attributes */

    set_attribs( ttt, boxtype, obj->col1, obj->col2,
                 obj->lcol, obj->align, obj->lsize, obj->lstyle, obj->label );

    set_miscattribs( ttt, obj->nwgravity, obj->segravity, obj->resize );

    is_focus = obj->focus;

    prev = obj->prev;
    form = obj->form;

    clear_selection( );

    fl_delete_object( obj );
    fli_handle_object( obj, FL_FREEMEM, 0, 0, 0, NULL, 0 );
    if ( obj->child )
        fli_free_composite( obj );

    *obj = *ttt;
    obj->u_vdata = sp;
    obj->form = NULL;
    if ( prev->next )
        fli_insert_object( obj, prev->next );
    else
        fl_add_object( form, obj );

    addto_selection( obj );

    superspec_to_spec( obj );

    fli_handle_object( obj, FL_ATTRIB, 0, 0, 0, NULL, 0 );

    /* Correct the object focus if required. */

    if ( is_focus )
        fl_set_object_focus( obj->form, obj );

    obj->shortcut = shct;

    redraw_the_form( 0 );
    show_attributes( obj );
}


/***************************************
 ***************************************/

static void
accept_spec( FL_OBJECT * ob    FL_UNUSED_ARG,
             long        data  FL_UNUSED_ARG )
{
    long tmp;

    spec_to_superspec( curobj );
    fl_set_folder_bynumber( fd_attrib->attrib_folder, 1 );
    tmp = fd_attrib->attrib_folder->argument;
    fd_attrib->attrib_folder->argument = -1;
    fl_call_object_callback( fd_attrib->attrib_folder );
    fd_attrib->attrib_folder->argument = tmp;
}


/***************************************
 ***************************************/

static void
spec_apply_cb( FL_OBJECT * ob    FL_UNUSED_ARG,
               long        data  FL_UNUSED_ARG )
{
    redraw_the_form( 0 );
}


/***************************************
 * Switch between "Generic" and "Spec" folder
 ***************************************/

void
folder_switch_cb( FL_OBJECT * ob,
                  long        data )
{
    FD_attrib *ui = ob->form->fdui;

    int active = fl_get_active_folder_number( ui->attrib_folder );

    if ( active == 1 )
    {
        if ( data != -1 )       /* -1 indicates manual call */
            fl_call_object_callback( fd_attrib->readyobj );

        fd_attrib->readyobj->type = FL_RETURN_BUTTON;     /* yuk! */
        fl_set_object_shortcut( fd_attrib->readyobj, "^M", 0 );
        fl_redraw_object( fd_attrib->readyobj );
        fl_set_object_callback( fd_attrib->readyobj, NULL, 0 );
        fl_set_object_callback( fd_attrib->applyobj, apply_cb, 0 );
        fl_set_object_callback( fd_attrib->restoreobj, restore_cb, 0 );
        cleanup_spec( curobj );
    }
    else
    {
        fd_attrib->readyobj->type = FL_NORMAL_BUTTON;   /* yuk! */
        fl_set_object_shortcut( fd_attrib->readyobj, "", 0 );
        fl_redraw_object( fd_attrib->readyobj );
        fl_redraw_object( fd_attrib->readyobj );
        fl_set_object_callback( fd_attrib->readyobj, accept_spec, 0 );
        fl_set_object_callback( fd_attrib->applyobj, spec_apply_cb, 0 );
        set_objclass_spec_attributes( curobj, 0 );
    }
}


/*
 * Local variables:
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
