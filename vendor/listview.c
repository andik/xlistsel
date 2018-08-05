/*

authored from 2015-2018 by Micha Mettke and Andreas Koerner

LICENSE:
    This software is dual-licensed to the public domain and under the following
    license: you are granted a perpetual, irrevocable license to copy, modify,
    publish and distribute this file as you see fit.

*/

#include <nuklear.h>

#include "listview.h"

/* ----------------------------------------------------------------------- */

int nk_listview_init(struct nk_listview* self, const char* grpname)
{
	self->selected = -1;
	self->grpname = grpname;
	self->active = 0;
	self->item_height = 25;
	self->scroll.x = 0;
	self->scroll.y = 0;
	return 0;
}

/* ----------------------------------------------------------------------- */

int nk_listview_select(struct nk_listview* self, int entry)
{
	self->selected = entry;
	return 0;
}

/* ----------------------------------------------------------------------- */

int nk_listview_activate(struct nk_listview* self, int state)
{
	self->active = state;
	return 0;
}

/* ----------------------------------------------------------------------- */

int nk_listview_selected(struct nk_listview* self)
{
	return self->selected;
}

/* ----------------------------------------------------------------------- */

int
nk_listview_head(
	struct nk_context* ctx, struct nk_listview* self, int ncols, char*** header)
{
	int col;
	
	/* The header needs it's own group to stay aligned with the list elements 
		the scrollbar_size.y must be set to zero to avoid the vertical scrollbar.
	
		We set the paddings in a way that no space is lost when embedding the list.
	*/
	nk_style_push_vec2(ctx, &ctx->style.window.group_padding, nk_vec2(0,0));
	nk_style_push_vec2(ctx, &ctx->style.window.padding, nk_vec2(0,0));
	nk_style_push_float(ctx, &ctx->style.window.scrollbar_size.y, 0.0f);
	nk_layout_row_dynamic(ctx, 0, 1);
	if (nk_group_begin(ctx, "nk_listview_header", 0)) 
	{		
		nk_style_push_vec2(ctx, &ctx->style.window.spacing, nk_vec2(1,0));
		
		nk_layout_row_dynamic(ctx, 0, ncols);
		for(col = 0; col < ncols; col++)
		{
			const char* lbl = header[0][col];
			nk_select_label(ctx, lbl ? lbl : " ", NK_TEXT_CENTERED, 1);
		}
		
		nk_style_pop_vec2(ctx);
		
		nk_group_end(ctx);
	}
	nk_style_pop_float(ctx);
	nk_style_pop_vec2(ctx);
	nk_style_pop_vec2(ctx);
	
	return 1;
}

/* ----------------------------------------------------------------------- */

int 
nk_listview_draw(
	struct nk_context* ctx, struct nk_listview* self, 
	int ncols, int nrows, char*** rows)
{
	/*
		Todo:

		NK_KEY_SCROLL_DOWN
		NK_KEY_SCROLL_UP

	*/
	
	int changed = 0;
	int old_selection;
	struct nk_rect bounds;
	nk_uint item_height;
	nk_uint selected_position;
	
	old_selection = self->selected;
	bounds = nk_widget_bounds(ctx);
	item_height = self->item_height + (nk_uint) ctx->style.window.spacing.y;
	selected_position = item_height * self->selected;

	/* Event handling */
	if(self->active)
	{
		if(nk_input_is_key_pressed(&ctx->input, NK_KEY_DOWN) && self->selected < (nrows-1))
		{
			self->selected++;
			if(selected_position + 3*item_height> (self->scroll.y + bounds.h))
				self->scroll.y = selected_position - (nk_uint) bounds.h + 3*item_height;
			
		}
		else if(nk_input_is_key_pressed(&ctx->input, NK_KEY_UP) && self->selected > 0)
		{
			self->selected--;
			if(selected_position - item_height < self->scroll.y)
				self->scroll.y = selected_position - item_height;
		}
		else if(nk_input_is_key_pressed(&ctx->input, NK_KEY_TEXT_LINE_START))
		{
			self->selected = 0;
			self->scroll.y = 0;
		}
		else if(nk_input_is_key_pressed(&ctx->input, NK_KEY_TEXT_LINE_END))
		{
			self->selected = nrows-1;
			self->scroll.y = item_height * (nrows - 1);
		}
	}
	else
	{
		if(nk_input_is_mouse_click_in_rect(&ctx->input, NK_BUTTON_LEFT, bounds))
		{
			self->active = 1;
		}
	}

	/* Draw the list elements */
	
	
	/* We set the paddings so that no space is lost when embedding the list. */
	nk_style_push_vec2(ctx, &ctx->style.window.group_padding, nk_vec2(0,0));
	nk_style_push_vec2(ctx, &ctx->style.window.padding, nk_vec2(0,0));
	if (nk_group_scrolled_begin(ctx, &self->scroll, self->grpname, 0)) 
	{
		int selected;
		int row;
		int col;
		
		nk_style_push_vec2(ctx, &ctx->style.window.spacing, nk_vec2(1,1));
		
		nk_layout_row_dynamic(ctx, 0, ncols);	
		for(row = 0; row < nrows; row++)
		{
			selected = (row == self->selected);
			for(col = 0; col < ncols; col++)
			{
				const char* lbl = rows[row][col];
				if(nk_select_label(ctx, lbl ? lbl : " ", NK_TEXT_LEFT, selected))
				{
						self->selected = row;
				}
			}
		}
		nk_style_pop_vec2(ctx);
		nk_group_scrolled_end(ctx);
	}
	nk_style_pop_vec2(ctx);
	nk_style_pop_vec2(ctx);
	
	return old_selection != self->selected;
}
