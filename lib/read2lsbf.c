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
 * \file read2lsbf.c
 *
 *.  Copyright(c) 1993,1994 by T.C. Zhao
 *   All rights reserved.
 *.
 *    Read 2bytes LSB first
 ***********************************************************************/
#if !defined(lint) && defined(F_ID)
char *id_2lsb = "$Id: read2lsbf.c,v 1.4 2003/04/22 10:06:59 leeming Exp $";
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#include "forms.h"
#include "flinternal.h"
#include "ulib.h"

int
fl_fget2LSBF(FILE * fp)
{
    int ret = getc(fp);
    return (ret | getc(fp) << 8);
}

int
fl_fput2LSBF(int code, FILE * fp)
{
    return put2LSBF(code, fp);
}
