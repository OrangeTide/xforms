/*
 *	This file is part of the XForms library package.
 *
 *  XForms is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU Lesser General Public License as
 *  published by the Free Software Foundation; either version 2.1, or
 *  (at your option) any later version.
 *
 *  XForms is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with XForms.  If not, see <http://www.gnu.org/licenses/>.
 */

/********************** crop here for forms.h **********************/


#ifndef FL_SCROLLBAR_H
#define FL_SCROLLBAR_H

enum {
	FL_HOR_SCROLLBAR,
	FL_VERT_SCROLLBAR,
	FL_HOR_THIN_SCROLLBAR,
	FL_VERT_THIN_SCROLLBAR,
	FL_HOR_NICE_SCROLLBAR,
	FL_VERT_NICE_SCROLLBAR,
	FL_HOR_PLAIN_SCROLLBAR,
	FL_HOR_BASIC_SCROLLBAR = FL_HOR_PLAIN_SCROLLBAR,
	FL_VERT_PLAIN_SCROLLBAR,
	FL_VERT_BASIC_SCROLLBAR = FL_VERT_PLAIN_SCROLLBAR
};

enum {
   FL_THIN_SCROLLBAR,
   FL_PLAIN_SCROLLBAR,
   FL_NORMAL_SCROLLBAR,
   FL_NICE_SCROLLBAR
};

FL_EXPORT FL_OBJECT * fl_create_scrollbar( int			type,
										   FL_Coord	    x,
										   FL_Coord	    y,
										   FL_Coord	    w,
										   FL_Coord	    h,
										   const char * label );

FL_EXPORT FL_OBJECT * fl_add_scrollbar( int			 type,
										FL_Coord	 x,
										FL_Coord	 y,
										FL_Coord	 w,
										FL_Coord	 h,
										const char * label );

FL_EXPORT double fl_get_scrollbar_value( FL_OBJECT * ob );

FL_EXPORT void fl_set_scrollbar_value( FL_OBJECT * ob,
									   double	   val );

FL_EXPORT void fl_set_scrollbar_size( FL_OBJECT * ob,
									  double	  val );

FL_EXPORT void fl_set_scrollbar_increment( FL_OBJECT * ob,
										   double	   l,
										   double	   r );

FL_EXPORT void fl_get_scrollbar_increment( FL_OBJECT * ob,
										   double    * a,
										   double    * b );

FL_EXPORT void fl_set_scrollbar_bounds( FL_OBJECT * ob,
										double		b1,
										double		b2 );

FL_EXPORT void fl_get_scrollbar_bounds( FL_OBJECT * ob,
										double    * b1,
										double    * b2 );

FL_EXPORT void fl_set_scrollbar_return( FL_OBJECT * ob,
										int			ret );

FL_EXPORT void fl_set_scrollbar_step( FL_OBJECT * ob,
									  double	  step );

#endif /* ! defined FL_SCROLLBAR_H */
