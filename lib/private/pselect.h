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
 * You should have received a copy of the GNU General Public License
 * along with XForms.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef PSELECT_H_
#define PSELECT_H_


typedef struct {
	FL_POPUP        * popup;
	FL_POPUP_RETURN * sel;
	int               align;
	char              * title;
	int               lstyle;
	int               lsize;
	FL_COLOR          lcolor;
	int               timeout_id;
	long              repeat_ms;
	int               p_bw;
	int               p_title_font_style;
	int               p_title_font_size;
	int               p_entry_font_style;
	int               p_entry_font_size;
	FL_COLOR          p_bg_color;
	FL_COLOR          p_on_color;
	FL_COLOR          p_title_color;
	FL_COLOR          p_text_color;
	FL_COLOR          p_text_on_color;
	FL_COLOR          p_text_off_color;
	int               p_policy;
	int               p_min_width;
} FLI_SELECT_SPEC;


#endif
