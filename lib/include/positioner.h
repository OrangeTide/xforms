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
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.     See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with XForms.  If not, see <http://www.gnu.org/licenses/>.
 */

/********************** crop here for forms.h **********************/

/**
 * \file positioner.h
 */

#ifndef FL_POSITIONER_H
#define FL_POSITIONER_H


#define FL_NORMAL_POSITIONER      0
#define FL_OVERLAY_POSITIONER     1
#define FL_INVISIBLE_POSITIONER   1

/***** Defaults *****/

#define FL_POSITIONER_BOXTYPE   FL_DOWN_BOX
#define FL_POSITIONER_COL1      FL_COL1
#define FL_POSITIONER_COL2      FL_RED
#define FL_POSITIONER_LCOL      FL_LCOL
#define FL_POSITIONER_ALIGN     FL_ALIGN_BOTTOM

/***** Routines *****/

FL_EXPORT FL_OBJECT * fl_create_positioner( int          type,
                                            FL_Coord     x,
                                            FL_Coord     y,
                                            FL_Coord     w,
                                            FL_Coord     h,
                                            const char * label );

FL_EXPORT FL_OBJECT * fl_add_positioner( int          type,
                                         FL_Coord     x,
                                         FL_Coord     y,
                                         FL_Coord     w,
                                         FL_Coord     h,
                                         const char * label );

FL_EXPORT void fl_set_positioner_xvalue( FL_OBJECT * ob,
                                         double      val );

FL_EXPORT double fl_get_positioner_xvalue( FL_OBJECT * ob );

FL_EXPORT void fl_set_positioner_xbounds( FL_OBJECT * ob,
                                          double      min,
                                          double      max );

FL_EXPORT void fl_get_positioner_xbounds( FL_OBJECT * ob,
                                          double    * min,
                                          double    * max );

FL_EXPORT void fl_set_positioner_yvalue( FL_OBJECT * ob,
                                         double      val );

FL_EXPORT double fl_get_positioner_yvalue( FL_OBJECT * ob );

FL_EXPORT void fl_set_positioner_ybounds( FL_OBJECT * ob,
                                          double      min,
                                          double      max );

FL_EXPORT void fl_get_positioner_ybounds( FL_OBJECT * ob,
                                          double    * min,
                                          double    * max );

FL_EXPORT void fl_set_positioner_xstep( FL_OBJECT * ob,
                                        double      value );

FL_EXPORT void fl_set_positioner_ystep( FL_OBJECT * ob,
                                        double      value );

FL_EXPORT void fl_set_positioner_return( FL_OBJECT * ob,
                                         int         value );

#endif /* ! defined FL_POSITIONER_H */
