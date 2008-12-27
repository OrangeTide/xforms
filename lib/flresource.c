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
 * You should have received a copy of the GNU Lesser General Public License
 * along with XForms.  If not, see <http://www.gnu.org/licenses/>.
 */


/**
 * \file flresource.c
 *
 *.
 *  This file is part of the XForms library package.
 *  Copyright (c) 1996-2002  T.C. Zhao
 *  All rights reserved.
 *.
 *
 *  Handle XResources. Only rudimetry support is implemented. However,
 *  using this minimum implementation, more powerful and application
 *  specific resources can be developed easily.
 *
 */

#if defined F_ID || defined DEBUG
char *fl_id_rsc = "$Id: flresource.c,v 1.34 2008/12/27 22:20:48 jtt Exp $";
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include/forms.h"
#include "flinternal.h"
#include "private/flsnprintf.h"
#include <X11/Xresource.h>
#include <ctype.h>
#include <sys/types.h>

#if 0
#include <locale.h>
#endif

#ifdef FL_WIN32
#include <X11/Xw32defs.h>
#endif


FLI_WM_STUFF fli_wmstuff;       /* also used in win.c */
static XrmDatabase fldatabase;	/* final merged database */
static XrmDatabase cmddb;	    /* command line database */
static char *fl_app_name,
            *fl_app_class,
            *fl_ori_app_name;

static void fli_init_resources( void );
static void fli_set_debug_level( int );


/* Command line options */

static XrmOptionDescRec copt[ ] =
{
    { "-fldebug",   "*fldebug",          XrmoptionSepArg, ( caddr_t ) "0" },
    { "-flversion", "*flversion",        XrmoptionNoArg,  ( caddr_t ) "1" },
    { "-flhelp",    "*flhelp",           XrmoptionNoArg,  ( caddr_t ) "1" },
    { "-name",      ".name",             XrmoptionSepArg, ( caddr_t ) 0   },
    { "-display",   ".display",          XrmoptionSepArg, ( caddr_t ) 0   },
    { "-sync",      "*sync",             XrmoptionNoArg,  ( caddr_t ) "1" },
    { "-depth",     "*depth",            XrmoptionSepArg, ( caddr_t ) 0   },
    { "-visual",    "*visual",           XrmoptionSepArg, ( caddr_t ) 0   },
    { "-private",   "*privateColormap",  XrmoptionNoArg,  ( caddr_t ) "1" },
    { "-shared",    "*sharedColormap",   XrmoptionNoArg,  ( caddr_t ) "1" },
    { "-standard",  "*standardColormap", XrmoptionNoArg,  ( caddr_t ) "1" },
    { "-stdcmap",   "*standardColormap", XrmoptionNoArg,  ( caddr_t ) "1" },
    { "-double",    "*doubleBuffer",     XrmoptionNoArg,  ( caddr_t ) "1" },
    { "-bw",        "*borderWidth",      XrmoptionSepArg, ( caddr_t ) 0   },
    { "-vid",       "*visualID",         XrmoptionSepArg, ( caddr_t ) 0   },

#ifdef DO_GAMMA_CORRECTION
    { "-rgamma",    "*rgamma",           XrmoptionSepArg, ( caddr_t ) 0   },
    { "-bgamma",    "*bgamma",           XrmoptionSepArg, ( caddr_t ) 0   },
    { "-ggamma",    "*ggamma",           XrmoptionSepArg, ( caddr_t ) 0   },
#endif

/*  { "-single",    "*doubleBuffer",     XrmoptionNoArg, ( caddr_t ) "0"   }, */
    { "-bs",        "*backingStore",     XrmoptionNoArg, ( caddr_t ) "0"   }
};

#define Ncopt  ( sizeof copt / sizeof *copt )


/* Other resources */

#define PV( a )  &( fli_cntl.a )
#define PS( a )  ( fli_cntl.a )
#define NS( a )  #a


/* xform and XForm will be generated on the fly */

typedef char Bop[ 8 ];		/* Boolean default */
typedef char Iop[ 8 ];		/* Integer default */
static Bop OpPrivateMap,
           OpSharedMap,
           OpStandardMap,
           OpDouble;
static Bop OpSync,
           OpULW = "1";
static Iop OpDebug,
           OpDepth,
           OpULT = "-1";
static char OpBS[ 12 ]       = "1";	/* whenmapped */
static char OpSafe[ 12 ];
static char OpSCBT[ 16 ]     = "thin";
static char OpBLsize[ 20 ]   = NS( FL_DEFAULT_SIZE );
static char OpMLsize[ 20 ]   = NS( FL_DEFAULT_SIZE );
static char OpBrFsize[ 20 ]  = NS( FL_DEFAULT_SIZE );
static char OpChFsize[ 20 ]  = NS( FL_DEFAULT_SIZE );
static char OpSLsize[ 20 ]   = NS( FL_DEFAULT_SIZE );
static char OpLLsize[ 20 ]   = NS( FL_DEFAULT_SIZE );
static char OpILsize[ 20 ]   = NS( FL_DEFAULT_SIZE );
static char OpIBW[ 20 ]      = NS( FL_BOUND_WIDTH );
static char OpPsize[ 20 ]    = NS( FL_DEFAULT_SIZE );
static char OpVisualID[ 20 ] = "0";

#ifdef DO_GAMMA_CORRECTION
static char OpRgamma[12] = "1";
static char OpGgamma[12] = "1";
static char OpBgamma[12] = "1";
#endif

static char OpCoordUnit[32];

static FL_VN_PAIR vn_coordunit[] =
{
	{ FL_COORD_PIXEL, "pixel" },
    { FL_COORD_MM, "mm" },
    { FL_COORD_POINT, "point" },
    { FL_COORD_centiPOINT, "centipoint" },
    { FL_COORD_centiPOINT, "cp" },
    { FL_COORD_centiMM, "centimm" },
    { FL_COORD_centiMM, "cmm" },
    { -1, "Invalid" }
};


#define SetR( name, rclass, type, deft, buflen ) \
       { #name, rclass, type, &fli_cntl.name, deft, buflen }


static FL_resource internal_resources[ ] =
{
    SetR( debug, "Debug", FL_INT,  OpDebug, 0 ),
    SetR( sync,  "Sync",  FL_BOOL, OpSync,  0 ),

#ifdef DO_GAMMA_CORRECTION
    SetR( rgamma, "Gamma", FL_FLOAT, OpRgamma, 0 ),
    SetR( ggamma, "Gamma", FL_FLOAT, OpGgamma, 0 ),
    SetR( bgamma, "Gamma", FL_FLOAT, OpBgamma, 0 ),
#endif

    SetR( depth, "Depth", FL_INT, OpDepth, 0 ),
    { "visual", "Visual", FL_STRING, PS( vname ), PS( vname ), 20 },
    { "scrollbarType", "ScrollbarType", FL_STRING, OpSCBT, OpSCBT, 16 },
    SetR( doubleBuffer, "DoubleBuffer", FL_BOOL, OpDouble, 0 ),
    SetR( ulThickness, "ULThickness", FL_INT, OpULT, 0 ),
    SetR( privateColormap, "PrivateColormap", FL_BOOL, OpPrivateMap, 0 ),
    SetR( sharedColormap, "SharedColormap", FL_BOOL, OpSharedMap, 0 ),
    SetR( standardColormap, "StandardColormap", FL_BOOL, OpStandardMap, 0 ),
    SetR( buttonFontSize, "FontSize", FL_INT, OpBLsize, 0 ),
    SetR( menuFontSize, "FontSize", FL_INT, OpMLsize, 0 ),
    SetR( choiceFontSize, "FontSize", FL_INT, OpChFsize, 0 ),
    SetR( browserFontSize, "FontSize", FL_INT, OpBrFsize, 0 ),
    SetR( labelFontSize, "FontSize", FL_INT, OpLLsize, 0 ),
    SetR( sliderLabelSize, "FontSize", FL_INT, OpSLsize, 0 ),
    SetR( inputLabelSize, "FontSize", FL_INT, OpILsize, 0 ),
    SetR( pupFontSize, "PupFontSize", FL_INT, OpPsize, 0 ),
    SetR( borderWidth, "BorderWidth", FL_INT, OpIBW, 0 ),
    SetR( ulPropWidth, "ULWidth", FL_BOOL, OpULW, 0 ),
    SetR( backingStore, "BackingStore", FL_INT, OpBS, 0 ),
    SetR( safe, "Safe", FL_INT, OpSafe, 0 ),
    {"coordUnit", "CoordUnit", FL_STRING, OpCoordUnit, OpCoordUnit, 32 },
    {"visualID", "VisualID", FL_LONG, &fli_requested_vid, OpVisualID, 0 }
};

#define Niopt ( sizeof internal_resources / sizeof *internal_resources )


/* Program can set its own default, overriding XForms default */

#define SetMember( a )   fli_cntl.a = cntl->a
#define GetMember( a )   cntl->a


/***************************************
 ***************************************/

void
fl_set_defaults( unsigned long mask,
				 FL_IOPT *     cntl )
{
    if ( mask & FL_PDPrivateMap )
    {
		SetMember( privateColormap );
		sprintf( OpPrivateMap, "%d", cntl->privateColormap != 0 );
    }

    if ( mask & FL_PDSharedMap )
    {
		SetMember( sharedColormap );
		sprintf( OpSharedMap, "%d", cntl->sharedColormap != 0 );
    }

    if ( mask & FL_PDStandardMap )
    {
		SetMember( standardColormap );
		sprintf( OpStandardMap, "%d", cntl->standardColormap != 0 );
    }

    if ( mask & FL_PDDouble )
    {
		SetMember( doubleBuffer );
		sprintf( OpDouble, "%d", cntl->doubleBuffer != 0 );
    }

    if ( mask & FL_PDDepth )
    {
		SetMember( depth );
		sprintf( OpDepth, "%d", cntl->depth );
    }

    if ( mask & FL_PDVisual )
    {
		SetMember( vclass );
		strcpy( fli_cntl.vname, fl_vclass_name( cntl->vclass ) );
    }

    if ( mask & FL_PDButtonFontSize )
    {
		SetMember( buttonFontSize );
		sprintf( OpBLsize, "%d", cntl->buttonFontSize );
    }

    if ( mask & FL_PDBrowserFontSize )
    {
		SetMember( browserFontSize );
		sprintf( OpBrFsize, "%d", cntl->browserFontSize );
    }

    if ( mask & FL_PDMenuFontSize )
    {
		SetMember( menuFontSize );
		sprintf( OpMLsize, "%d", cntl->menuFontSize );
    }

    if ( mask & FL_PDChoiceFontSize )
    {
		SetMember( choiceFontSize );
		sprintf( OpChFsize, "%d", cntl->choiceFontSize );
    }

    if ( mask & FL_PDSliderFontSize )
    {
		SetMember( sliderFontSize );
		sprintf( OpSLsize, "%d", cntl->sliderFontSize );
    }

    if ( mask & FL_PDInputFontSize )
    {
		SetMember( inputFontSize );
		sprintf( OpILsize, "%d", cntl->inputFontSize );
    }

    if ( mask & FL_PDLabelFontSize )
    {
		SetMember( labelFontSize );
		sprintf( OpLLsize, "%d", cntl->labelFontSize );
    }

    if ( mask & FL_PDBorderWidth )
		fl_set_border_width( cntl->borderWidth );

    if ( mask & FL_PDScrollbarType )
		fl_set_scrollbar_type( cntl->scrollbarType );

    if ( mask & FL_PDPupFontSize )
    {
		SetMember( pupFontSize );
		sprintf( OpPsize, "%d", cntl->pupFontSize );
    }

    if ( mask & FL_PDSafe )
    {
		SetMember( safe );
		sprintf( OpSafe, "%d", cntl->safe );
    }

    if ( mask & FL_PDBS )
    {
		SetMember( backingStore );
		sprintf( OpBS, "%d", cntl->backingStore );
    }

    if ( mask & FL_PDCoordUnit )
		fl_set_coordunit( cntl->coordUnit );

    if ( mask & FL_PDDebug )
		fli_set_debug_level( cntl->debug );
}


/***************************************
 ***************************************/

void
fl_get_defaults( FL_IOPT * cntl )
{
    if ( ! fl_display )
    {
		M_err( "GetDefault", "You forgot to call fl_initialize" );
		exit( 1 );
    }

    *cntl = fli_cntl;
}


/***************************************
 * convenience functions
 ***************************************/

void
fl_set_coordunit( int u )
{
    fli_cntl.coordUnit = u;
    strcpy( OpCoordUnit, fl_get_vn_name( vn_coordunit, fli_cntl.coordUnit ) );
}


/***************************************
 ***************************************/

void
fl_set_border_width( int bw )
{
    fli_cntl.borderWidth = bw;
    sprintf( OpIBW, "%d", bw );
}


/***************************************
 ***************************************/

void
fl_set_scrollbar_type( int t )
{
    fli_cntl.scrollbarType = t;
    if ( t == FL_NORMAL_SCROLLBAR )
		strcpy( OpSCBT, "normal" );
    else if ( t == FL_NICE_SCROLLBAR )
		strcpy( OpSCBT, "nice" );
    else if ( t == FL_PLAIN_SCROLLBAR )
		strcpy( OpSCBT, "plain" );
    else
		strcpy( OpSCBT, "thin" );
}


/***************************************
 ***************************************/

void
fl_set_visualID( long id )
{
    fli_requested_vid = id;
    sprintf( OpVisualID, "0x%lx", id );
}


/***************************************
 ***************************************/

int
fl_get_border_width( void )
{
    return fli_cntl.borderWidth;
}


/***************************************
 ***************************************/

int
fl_get_coordunit( void )
{
    return fli_cntl.coordUnit;
}


/***************************************
 ***************************************/

static void
fli_set_debug_level( int l )
{
    fli_cntl.debug = l;
    sprintf( OpDebug, "%d", fli_cntl.debug );
    fli_set_msg_threshold( fli_cntl.debug );
}


/***************************************
 * Handle XAPPLRESDIR (colon seperated directories) specification.
 * Ignore the %T %N stuff
 ***************************************/

static void
handle_applresdir( const char * rstr,
				   const char * appclass )
{
    char *tok;
    char rbuf[ 512 ],
		 buf[ 512 ];
    XrmDatabase fdb = 0;

    strcpy( rbuf, rstr );
    for ( tok = strtok( rbuf, ":" ); tok; tok = strtok( 0, ":" ) )
    {
        fl_snprintf( buf, sizeof buf,"%s/%s", tok,appclass );
		M_info( 0, "Trying XAPPLRESDIR: %s", buf );
		if ( ( fdb = XrmGetFileDatabase( buf ) ) )
		{
			XrmMergeDatabases( fdb, &fldatabase );
			M_warn( 0, "  XAPPLRESDIR %s loaded", buf );
		}
    }
}


/***************************************
 * Routine to merge all resource databases, excluding the commandline,
 * which is  done in fl_initialize.
 ***************************************/

static void
init_resource_database( const char *appclass )
{
    char buf[ FL_PATH_MAX + 127 ],
		 *rstr;
    XrmDatabase fdb = 0;

    if ( ! fl_display )
    {
		M_err( "InitResource", "fl_initialize is not called" );
		return;
    }

    if ( fldatabase )
		return;

    XrmInitialize( );

#ifdef __VMS
	/* For the VMS version try to load the resources from, in this order,

	   DECW$SYSTEM_DEFAULTS:appclass.DAT
	   DECW$USER_DEFAULTS:appclass.DAT
	   DECW$USER_DEFAULTS:DECW$XDEFAULTS.DAT
	   The window resource manager for this display
	*/

    fl_snprintf( buf, sizeof buf, "DECW$SYSTEM_DEFAULTS:%s.DAT", appclass );
    M_info( 0, "Trying Sys_default: %s", buf );
    if ( ( fdb = XrmGetFileDatabase( buf ) ) )
    {
		XrmMergeDatabases( fdb, &fldatabase );
		M_warn( 0, "   system default %s loaded", buf );
    }

    fl_snprintf( buf, sizeof buf, "DECW$USER_DEFAULTS:%s.DAT", appclass );
    M_info( 0, "Trying User_default: %s", buf );
    if ( ( fdb = XrmGetFileDatabase( buf ) ) )
    {
		XrmMergeDatabases( fdb, &fldatabase );
		M_warn( 0, "   system default %s loaded", buf );
    }

    fl_snprintf( buf, sizeof buf, "DECW$USER_DEFAULTS:DECW$XDEFAULTS.DAT" );
    M_info( 0, "Trying Sys_default: %s", buf );
    if ( ( fdb = XrmGetFileDatabase( buf ) ) )
    {
		XrmMergeDatabases( fdb, &fldatabase );
		M_warn( 0, "   system default %s loaded", buf );
    }

    M_info( 0, "Trying RESOURCE_MANAGER" );
    if ( ( rstr = XResourceManagerString( fl_display ) ) )
    {
		if ( ( fdb = XrmGetStringDatabase( rstr ) ) )
		{
			XrmMergeDatabases( fdb, &fldatabase );
			M_warn( 0, "   RESOURCE_MANAGER loaded" );
		}
    }
#else /* !VMS */

    fl_snprintf( buf, sizeof buf, "/usr/lib/X11/app-defaults/%s", appclass );
    M_info( 0, "Trying Sys_default: %s", buf );
    if ( ( fdb = XrmGetFileDatabase( buf ) ) )
    {
		XrmMergeDatabases( fdb, &fldatabase );
		M_warn( 0, "   system default %s loaded", buf );
    }

    /* try XAPPLRESDIR */

    M_info( 0, "Trying XAPPLRESDIR" );
    if ( ( rstr = getenv( "XAPPLRESDIR" ) ) )
		handle_applresdir( rstr, appclass );

    /* try server defined resources */

    M_info( 0, "Trying RESOURCE_MANAGER" );
    if ( ( rstr = XResourceManagerString( fl_display ) ) )
    {
		if ( ( fdb = XrmGetStringDatabase( rstr ) ) )
		{
			XrmMergeDatabases( fdb, &fldatabase );
			M_warn( 0, "   RESOURCE_MANAGER loaded" );
		}
    }
    else
    {
		/* try ~/.Xdefaults   */

		if ( ( rstr = getenv( "HOME" ) ) )
		{
            fl_snprintf( buf, sizeof buf,"%s/.Xdefaults", rstr );
			M_info( 0, "Trying %s", buf );
			if ( ( fdb = XrmGetFileDatabase( buf ) ) )
			{
				XrmMergeDatabases( fdb, &fldatabase );
				M_warn( 0, "   %s loaded", buf );
			}
		}
    }

    /* load file XENVIRONMENT */

    M_info( 0, "Trying $XEVIRONMENT" );
    if ( ( rstr = getenv( "XENVIRONMENT" ) ) )
    {
		if ( ( fdb = XrmGetFileDatabase( rstr ) ) )
		{
			XrmMergeDatabases( fdb, &fldatabase );
			M_warn( 0, "   %s loaded", rstr );
		}
    }
    else
    {
		/* ~/.Xdefaults-<hostname> */

		M_info( 0, "Trying ~/.Xdefaults-<hostname>" );
		if ( ( rstr = getenv( "HOME" ) ) )
		{
			int l;

            fl_snprintf( buf, sizeof buf,"%s/.Xdefaults", rstr );
			l = strlen( strcat( buf, "-" ) );
			gethostname( buf + l, sizeof buf - l );
			M_info( 0, "Trying %s", buf );
			if ( ( fdb = XrmGetFileDatabase( buf ) ) )
			{
				XrmMergeDatabases( fdb, &fldatabase );
				M_warn( 0, "   %s loaded", buf );
			}
		}
    }
#endif /* VMS */

    errno = 0;

    if ( ! fldatabase )
    {
		M_warn( "InitResource", "Can't find any resource databases!" );
		return;
    }
}


/***************************************
 ***************************************/

static int
is_true( const char *s )
{
    return    strncmp( s, "True", 4 ) == 0
		   || strncmp( s, "true", 4 ) == 0
		   || strncmp( s, "Yes", 3  ) == 0
		   || strncmp( s, "yes", 3  ) == 0
		   || strncmp( s, "On", 2   ) == 0
		   || strncmp( s, "on", 2   ) == 0
		   || *s == '1';
}


/***************************************
 * The resource routine of the lowest level. Only adds the application
 * name.
 *
 * Return the string representation of the resource value
 ***************************************/

const char *
fl_get_resource( const char * rname,	/* resource name */
				 const char * cname,	/* class name    */
				 FL_RTYPE     dtype,	/* data type     */
				 const char * defval,	/* default       */
				 void *       val,	    /* variable      */
				 int          size )    /* buffer size if string */
{
    XrmValue entry = { 0, NULL };
    char *type = NULL;
    char res_name[ 256 ]  = "",
		 res_class[ 256 ] = "";


	if ( ( ! rname || ! * rname ) && ( ! cname || ! *cname ) )
		return NULL;

	if ( rname && *rname )
		fl_snprintf( res_name, sizeof res_name,"%s.%s", fl_app_name, rname );
    else if ( cname && *cname )
        fl_snprintf( res_class, sizeof res_class,"%s.%s", fl_app_class, cname );

	/* Just checking the return value of XrmGetResource() doesn't seem to
	   work (as it should, unless I completely mis-understand the man
	   page), and 'entry' seems to return the same data as on a previous
	   call (despite the initialization!), but 'type' seems to get set to
	   NULL in cases of failure to find the requested resource in the
	   database. So in that case we try to use the default value.       JTT */

	if (    ! XrmGetResource( fldatabase, res_name, res_class, &type, &entry )
		 || ! type
		 || strcmp( type, "String" )
		 || ! entry.addr )
	{
		M_warn( "fl_get_resource", "%s (%s): not found", res_name, res_class );
		entry.addr = ( XPointer ) defval;
	}
	else
		M_info( "fl_get_resource", "%s (%s): %s", res_name, res_class,
				entry.addr );

    if ( dtype == FL_NONE || ! entry.addr )
		return entry.addr;

	switch ( dtype )
	{
		case FL_SHORT:
			*( short * ) val = atoi( entry.addr );
			break;

		case FL_INT:
			*( int * ) val = atoi( entry.addr );
			break;

		case FL_BOOL:
			*( int * ) val = is_true( entry.addr );
			break;

		case FL_LONG:
			*( long * ) val = strtol( entry.addr, NULL, 0 );
			break;

		case FL_FLOAT:
			*( float * ) val = ( float ) atof( entry.addr );
			break;

		case FL_STRING:
			if ( val && val != entry.addr && size > 0 )
			{
				strncpy( val, entry.addr, size );
				( ( char * ) val )[ size - 1 ] = '\0';
			}
			break;

		default:
			M_err( "GetResource", "Unknown type %d", dtype );
			return NULL;
	}

    return entry.addr;
}


/***************************************
 ***************************************/

void
fl_set_resource( const char * str,
				 const char * val )
{
    char res_name[ 256 ];

    fl_snprintf( res_name, sizeof res_name, "%s.%s", fl_app_name, str );
    XrmPutStringResource( &fldatabase, res_name, ( char * ) val );
}


/***************************************
 * internal resource initialization
 ***************************************/

static void
fli_init_resources( void )
{
    char res[ 256 ],
		 cls[ 256 ],
		 ores[ 256 ];
    char *appname = fl_app_name,
		 *appclass = fl_app_class;
    char *ori_appname = fl_ori_app_name;

    /* internal resources need to be prefixed xform and XForm. need to
       generate for all names */

    fl_snprintf( cls, sizeof cls, "%s.XForm", fl_app_class );
    fl_app_class = cls;
    fl_snprintf( res, sizeof res, "%s.xform", fl_app_name );
    fl_app_name = res;
    fl_snprintf( ores, sizeof ores, "%s.xform", fl_ori_app_name );
    fl_ori_app_name = ores;
    fl_get_app_resources( internal_resources, Niopt );

    fl_app_name = appname;
    fl_app_class = appclass;
    fl_ori_app_name = ori_appname;

    if ( fli_cntl.sync )
    {
		XSynchronize( fl_display, 1 );
		M_err( 0, "**** Synchronous Mode ********" );
		fli_set_debug_level( 4 );
    }
}


static int fl_argc;
static char **fl_argv;


/***************************************
 ***************************************/

static void
dup_argv( char ** argv,
		  int     n )
{
    int i;

    if ( ! argv )
		return;

    if ( ! fl_argv )
		fl_argv = fl_malloc( ( n + 1 ) * sizeof *fl_argv );

    for ( i = 0; i < n; i++ )
		fl_argv[ i ] = fl_strdup( argv[ i ] );
	
    fl_argv[ i ] = NULL;
}


/***************************************
 ***************************************/

char **
fl_get_cmdline_args( int *n )
{
    *n = fl_argc;
    return fl_argv;
}


/***************************************
 ***************************************/

void
fli_free_cmdline_args( void )
{
	size_t i;

	if ( ! fl_argv )
		return;

	for ( i = 0; fl_argv[ i ]; i++ )
		fl_safe_free( fl_argv[ i ] );

	fl_safe_free( fl_argv );
}


/***************************************
 * find the command name from arg[0] and return a copy of it
 ***************************************/

static char *
get_command_name( const char * arg0 )
{
    char *p;
    char *s = fl_strdup( arg0 );
    char *cmd_name = s;

#ifdef __VMS
    /* vms command disk:[path]command.exe  */

    if ( ( p = strrchr( s, ']' ) ) )
		cmd_name = p + 1;
#else
#ifdef FL_WIN32
    _splitpath( arg0, NULL, NULL, cmd_name, NULL );
#else
    if ( ( p = strrchr( s, '/' ) ) )
		cmd_name = p + 1;
#endif
#endif

    /* Remove the extension and the period */

    if ( ( p = strrchr( cmd_name, '.' ) ) )
		*p = '\0';

    /* Prevent a valgrind warning about a possible memory leak. */

    if ( s != cmd_name )
	{
		cmd_name = fl_strdup( cmd_name );
		fl_free( s );
    }

    return cmd_name;
}


/*
 * Global Routines to do the initialization and resource management
 *
 */

#define DumpD( a )    fprintf( stderr,"\t%s:%d\n", #a, fli_cntl.a )
#define DumpS( a )    fprintf( stderr,"\t%s:%s\n", #a, fli_cntl.a )
#define DumpF( a )    fprintf( stderr,"\t%s:%.3f\n", #a, fli_cntl.a )

static Window fli_GetVRoot( Display *,
							int );


/***************************************
 * initialize context
 ***************************************/

void
fli_init_context( void )
{
    if ( fli_context )
		return;

	fli_context = fl_calloc( 1, sizeof *fli_context );
	fli_context->io_rec        = NULL;
	fli_context->idle_rec      = NULL;
	fli_context->atclose       = NULL;
	fli_context->signal_rec    = NULL;
	fli_context->idle_delta    = FLI_TIMER_RES;
	fli_context->hscb          = FL_HOR_THIN_SCROLLBAR;
	fli_context->vscb          = FL_VERT_THIN_SCROLLBAR;
	fli_context->navigate_mask = ShiftMask;   /* to navigate input field */
}


/***************************************
 ***************************************/

void
fli_set_input_navigate( unsigned int mask )
{
     fli_init_context( );

     if ( mask == ShiftMask || mask == Mod1Mask || mask == ControlMask )
		 fli_context->navigate_mask = mask;
}


/***************************************
 ***************************************/

Display *
fl_initialize( int        * na,
			   char       * arg[ ],
			   const char * appclass,
			   FL_CMD_OPT * appopt,
			   int          nappopt )
{
    char disp_name[ 256 ],
		 disp_cls[ 256 ],
		 buf[ 256 ];
    XrmValue xval;
    char *type;
    float xdpi,
		  ydpi;

    if ( fl_display )
    {
		M_warn( "fl_initialize", "XForms: already initialized" );
		return fl_display;
    }

    /* be paranoid */

    if ( ! na || ! *na )
    {
		fprintf( stderr, "XForms: argc == 0 or argv == NULL detected\n" );
		exit( 1 );
    }

	/* Setting the locale was introduced in 0.89.5 (as far as I could figure
	   out) but never was officially documented. Since it can break programs
	   when run in a different locale then the one the original author used
	   (e.g. because reading of files with scanf() may not work anymore when,
	   due to the locale setting a ',' instead of a '.' as the decimal point is
	   expected) I have thrown it out. Jean-Marc and Angus already planned to
	   do that back in 2004 but obviously never got around to doing it.  JTT */

#if 0
    setlocale( LC_ALL, "" );
#endif

    fli_internal_init( );

    XrmInitialize( );

    /* Save a copy of the command line for later WM hints */

    fl_argc = *na;
    dup_argv( arg, *na );

    /* Get appname and class, but without any leading paths  */

    fl_ori_app_name = fl_app_name = get_command_name( arg[ 0 ] );
    fl_app_class = fl_strdup( ( appclass && *appclass ) ?
							  appclass : fl_app_name );

    /* Make class name upper case if non supplied */

    if ( ! appclass || ! *appclass )
    {
		fl_app_class[ 0 ] = toupper( fl_app_class[ 0 ] );
		if ( fl_app_class[ 0 ] == 'X' )
			fl_app_class[ 1 ] = toupper( fl_app_class[ 1 ] );
    }

    /* Do form internal resouces first */

    XrmParseCommand( &cmddb, copt, Ncopt, fl_app_name, na, arg );

    /* if there are still more left and  appopt is not zero */

    if ( appopt && na && *na )
		XrmParseCommand( &cmddb, appopt, nappopt,
						 ( char * ) fl_ori_app_name, na, arg );

    /* Check version request */

    fl_snprintf( disp_name, sizeof disp_name, "%s.fl_version", fl_app_name );
    fl_snprintf( disp_cls, sizeof disp_cls, "%s.flversion", fl_app_name );

    if ( XrmGetResource( cmddb, disp_name, disp_cls, &type, &xval ) )
		fli_print_version( 0 );

    /* Get the display name first before doing anything */

    fl_snprintf( disp_name, sizeof disp_name, "%s.display", fl_ori_app_name );
    fl_snprintf( disp_cls , sizeof disp_cls , "%s.Display", fl_app_class );

    buf[ 0 ] = '\0';

    if ( XrmGetResource( cmddb, disp_name, disp_cls, &type, &xval ) )
    {
		strncpy( buf, xval.addr, sizeof buf );
		buf[ sizeof buf - 1 ] = '\0';
    }

    /* Open display and quit if failure */

    if ( ! ( fl_display = XOpenDisplay( buf ) ) )
    {
        /* if no display is set, there is no guarantee that buf
           is long enough to contain the DISPLAY setting */

		M_err( 0, "%s: Can't open display %s", fl_argv[ 0 ],
			   XDisplayName( buf[ 0 ] ? buf : 0 ) );
		return 0;
    }

    flx->display = fl_display;
    flx->screen  = fl_screen;

    /* Get debug level settings since all error reporting will be controled
       by it */

    fl_snprintf( disp_name, sizeof disp_name, "%s.fldebug", fl_app_name );
    fl_snprintf( disp_cls , sizeof disp_cls , "%s.flDebug", fl_app_class );
    if ( XrmGetResource( cmddb, disp_name, disp_cls, &type, &xval ) )
		fli_set_debug_level( atoi( xval.addr ) );

    /* print help */

    fl_snprintf( disp_name, sizeof disp_name, "%s.flhelp", fl_app_name );
    fl_snprintf( disp_cls , sizeof disp_cls , "%s.flhelp", fl_app_class );

    if ( XrmGetResource( cmddb, disp_name, disp_cls, &type, &xval ) )
    {
		size_t i = 0;

		while ( i < Ncopt )
		{
			if ( i == 0 )
				fprintf( stderr, "%s: ", fl_argv[ 0 ] );
			else
				fprintf( stderr, "%*s  ", ( int ) strlen( fl_argv[ 0 ] ), "" );

			fprintf( stderr, " %s", copt[ i ].option );

			if ( copt[ i ].argKind == XrmoptionSepArg )
				fprintf( stderr, " \t[arg]" );
			fprintf( stderr, "\n" );
			i++;
		}

		exit(1);
    }

    /* Check if -name is present */

    fl_snprintf( disp_name, sizeof disp_name, "%s.name", fl_app_name );
    fl_snprintf( disp_cls,  sizeof disp_cls , "%s.Name", fl_app_class );
    M_warn( 0, "trying %s", disp_name );
    if ( XrmGetResource( cmddb, disp_name, disp_cls, &type, &xval ) )
    {
		fl_app_name = fl_strdup( xval.addr );
		M_warn( 0, "Change AppName from %s to %s", fl_ori_app_name,
				fl_app_name );
    }

    /* Merge  other resource database */

    init_resource_database( fl_app_class );

    /* Override any options in the database with the command line opt */

    XrmMergeDatabases( cmddb, &fldatabase );

    /* load FL resources */

    fli_init_resources( );

    fli_cntl.vclass = fl_vclass_val( fli_cntl.vname );
    fli_cntl.coordUnit = fl_get_vn_value( vn_coordunit, OpCoordUnit );


#if FL_DEBUG >= ML_WARN	/* { */
    if ( fli_cntl.debug )
    {
		fprintf( stderr, "Options Set\n" );
		DumpD( debug );
		fprintf( stderr, "\tVisual:%s (%d)\n",
				 fli_cntl.vclass >= 0 ?
				 fl_vclass_name( fli_cntl.vclass ) : "To be set",
				 fli_cntl.vclass );
		DumpD( depth );
		DumpD( privateColormap );
		DumpD( sharedColormap );
		DumpD( standardColormap );
		DumpD( doubleBuffer );
		DumpD( ulPropWidth );
		DumpD( ulThickness );
		DumpD( scrollbarType );
		DumpD( backingStore );
		fprintf( stderr, "\t%s:%s\n", "coordUnit",
				 fl_get_vn_name( vn_coordunit, fli_cntl.coordUnit ) );
		fprintf( stderr, "\t%s:0x%lx\n", "VisualId", fli_requested_vid );

#ifdef DO_GAMMA_CORRECTION
		DumpF( rgamma );
		DumpF( ggamma );
		DumpF( bgamma );
#endif
    }
#endif /* DEBUG *} */

#ifdef DO_GAMMA_CORRECTION
    fl_set_gamma( fli_cntl.rgamma, fli_cntl.ggamma, fli_cntl.bgamma );
#endif

    fl_set_ul_property( fli_cntl.ulPropWidth, fli_cntl.ulThickness );

    fli_cntl.vclass = fl_vclass_val( fli_cntl.vname );

    /* get the current keyboard state */

    {
		XKeyboardState xks;

		XGetKeyboardControl( fl_display, &xks );
		fli_keybdcontrol.auto_repeat_mode = xks.global_auto_repeat;
		fli_keybdmask = KBAutoRepeatMode;
    }

    /* other initializations */

    fl_screen = flx->screen = DefaultScreen( fl_display );
    fl_root                 = RootWindow( fl_display, fl_screen );
    fl_vroot                = fli_GetVRoot( fl_display, fl_screen );
    fli_wmstuff.pos_request = USPosition;

    if ( fl_root != fl_vroot )
    {
		M_warn( "FlInit", "fl_root=%lu fl_vroot=%lu", fl_root, fl_vroot );

		/* tvtwm requires this to position a window relative to the current
		   desktop */

		fli_wmstuff.pos_request = PPosition;
    }

    fl_scrh = DisplayHeight( fl_display, fl_screen );
    fl_scrw = DisplayWidth( fl_display, fl_screen );

    xdpi = fl_scrw * 25.4 / DisplayWidthMM( fl_display, fl_screen );
    ydpi = fl_scrh * 25.4 / DisplayHeightMM( fl_display, fl_screen );

    if ( xdpi / ydpi > 1.05 || ydpi / xdpi < 0.95 )
		M_warn( "FlInit", "NonSquarePixel %.1f %.1f", xdpi, ydpi );

    fli_dpi = ( xdpi + ydpi ) / 2;
    fli_dpi = ( ( int ) ( fli_dpi * 10.0 + 0.3 ) ) * 0.1;

    M_info( 0, "screen DPI=%f", fli_dpi );

    fl_vmode = fli_initialize_program_visual( );
    fli_init_colormap( fl_vmode );
    fli_init_font( );
    fli_init_context( );

#ifdef XlibSpecificationRelease
    if ( XSupportsLocale( ) )
    {
         XSetLocaleModifiers( "" );

        /* use the same input method throughout xforms */

        fli_context->xim = XOpenIM( fl_display, 0, 0, 0 );

        /* also use the same input context */

        if ( fli_context->xim )
        {
            int style =  XIMPreeditNothing | XIMStatusNothing;

            fli_context->xic = XCreateIC( fli_context->xim,
										  XNInputStyle, style,
										  ( char * ) NULL );

	    /* Clean-up on failure */

			if ( ! fli_context->xic )
			{
				M_err( "fl_initialize", "Could not create an input context" );
				XCloseIM( fli_context->xim );
			}
        }
		else
			M_err( "fl_initialize", "Could not create an input method" );
    }
#endif

    fli_default_xswa( );
    fli_init_stipples( );
    fli_set_err_msg_func( fl_show_alert );

    /* if using non-default visual or private colormap, need a window with
       correct visual/colormap and depth */

    fl_state[ fl_vmode ].trailblazer = fl_root;

    if (    fli_visual( fl_vmode ) != DefaultVisual( fl_display, fl_screen )
		 || fl_state[ fl_vmode ].pcm )
    {
		fl_state[ fl_vmode ].trailblazer =
		   fli_create_window( fl_root, fli_colormap( fl_vmode ),
							  "trailblazer" );
    }

    if ( strcmp( OpSCBT, "plain" ) == 0 )
    {
		fli_context->hscb = FL_HOR_PLAIN_SCROLLBAR;
		fli_context->vscb = FL_VERT_PLAIN_SCROLLBAR;
    }
    else if ( strcmp( OpSCBT, "normal" ) == 0 )
    {
		fli_context->hscb = FL_HOR_SCROLLBAR;
		fli_context->vscb = FL_VERT_SCROLLBAR;
    }
    else if ( strcmp( OpSCBT, "thin" ) == 0 )
    {
		fli_context->hscb = FL_HOR_THIN_SCROLLBAR;
		fli_context->vscb = FL_VERT_THIN_SCROLLBAR;
    }
    else if ( strcmp( OpSCBT, "nice" ) == 0 )
    {
		fli_context->hscb = FL_HOR_NICE_SCROLLBAR;
		fli_context->vscb = FL_VERT_NICE_SCROLLBAR;
    }

    fli_context->max_request_size = XMaxRequestSize( fl_display );

    if ( fli_context->max_request_size < 4096 )
    {
		M_err( "init", "Something is wrong with max_request_size:%ld",
			   fli_context->max_request_size );
		fli_context->max_request_size = 4096;
    }


#if XlibSpecificationRelease >= 6
    fli_context->ext_request_size = XExtendedMaxRequestSize( fl_display );
#else
    fli_context->ext_request_size = 0;
#endif

    if ( fli_context->ext_request_size == 0 )
		fli_context->ext_request_size = fli_context->max_request_size;

    fli_context->max_request_size -= 8;
    fli_context->ext_request_size -= 8;
    fli_context->tooltip_time = 600;

    fl_add_io_callback( ConnectionNumber( fl_display ), FL_READ, 0, 0 );

    /* has to be here, otherwise fl_add_symbol won't be able to replace the
       built-in */

    fli_init_symbols( );

    /* hang the database on the display so application can get it */

    XrmSetDatabase( fl_display, fldatabase );

	/* Initialize popup system */

	fli_popup_init( );

    return fl_display;
}


/***************************************
 * Find out about  virtual root. Taken from XFaq
 ***************************************/

#include <X11/Xatom.h>

static Window
fli_GetVRoot( Display * dpy,
			  int       scr )
{
    Window       rootReturn;
	Window	     parentReturn;
	Window *     children;
    unsigned int numChildren;
    Window       root = RootWindow( dpy, scr );
    Atom         __SWM_VROOT = None;
    unsigned int i;

    __SWM_VROOT = XInternAtom( dpy, "__SWM_VROOT", False );
    XQueryTree( dpy, root, &rootReturn, &parentReturn, &children,
				&numChildren );

    for ( i = 0; i < numChildren; i++ )
    {
		Atom          actual_type;
		int           actual_format;
		unsigned long nitems, bytesafter;
		Window *      newRoot = NULL;

		/* Kludge here: the ( void * ) bit in
		   ( unsigned char ** ) ( void * ) &newRoot
		   is there to avoid a spurious  GCC warning         JTT */

		if (    XGetWindowProperty( dpy, children[ i ], __SWM_VROOT, 0, 1,
									False, XA_WINDOW, &actual_type,
									&actual_format, &nitems, &bytesafter,
									( unsigned char ** ) ( void * ) &newRoot )
																	== Success
			 && newRoot )
		{
			root = *newRoot;
			break;
		}
    }

    XFree( ( char * ) children );
    return root;
}


/***************************************
 * For Application programs
 ***************************************/

static void
fli_get_app_resource( FL_resource * appresource,
					  int           n )
{
    FL_resource *flr = appresource,
		        *flrs = flr + n;

    for ( ; flr < flrs; flr++ )
		if ( flr->type == FL_STRING && flr->nbytes == 0 )
			M_err( "fl_get_app_resources", "%s: buflen==0", flr->res_name );
	else
	    fl_get_resource( flr->res_name, flr->res_class,
						 flr->type, flr->defval, flr->var, flr->nbytes );
}


/***************************************
 ***************************************/

void
fl_get_app_resources( FL_resource * appresource,
					  int           n )
{
    char *tmp = fl_app_name;

    fl_app_name = fl_ori_app_name;
    fli_get_app_resource( appresource, n );

    if ( fl_app_name != tmp )
    {
		fl_app_name = tmp;
		fli_get_app_resource( appresource, n );
    }
}


/***************************************
 ***************************************/

void
fl_flip_yorigin( void )
{
    if ( ! fl_display )
		fli_inverted_y = 1;
    else
		M_err( "fl_flip_yorigin", "Only supported before fl_initialize" );
}


/***************************************
 ***************************************/

void
fli_set_app_name( const char * n,
				  const char * c )
{
    if ( n )
		fl_app_name = fl_strdup( n );
    if ( c )
		fl_app_class = fl_strdup( c );
}
