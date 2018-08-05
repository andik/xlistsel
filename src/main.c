/*
 * xlistsel
 *
 * a simple tool to display a list and let the user select
 * one or more entries out of it.
 * Actions can be invoked on this entries by customisable app.buttons.
 *
 * This tool includes the nuklear user interface libraries which
 * means it does only depend in Xlib and is pretty lightweight.
 * It's written in pure ANSI C.
 *
 * -----------------------------------------------------------------------
 *
 * Copyright (c) 2018 Andreas Koerner <andi@jaak.de>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

 


#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#include <nuklear.h>
#include "nkapp.h"
#include "listview.h"

#include <options.h>
#include "extern.h"

/* ----------------------------------------------------------------------- */

/* Application State */
struct app
{
	/* Elements in the List */
	char*** tabrows; /* == char*[numtabrows][numcols] */
	int numtabrows;
	int numcols;

	/* Elements in the List */
	char*** header; /* == char*[numheader] */
	int numheader;

	/* Buttons in the Dialog - we also use table rows for simpler data keeping */
	char*** buttons; /* == char*[numbuttons] */
	int numbuttons;
	int numbuttoncols; /* this is just a dummy */
	
	/* number of lines in the description */
	#define MAX_DESC_LINES 32
	int num_desc_lines;
	char* desc_lines[MAX_DESC_LINES];
	
	/* this is required because we need the current state of the opts
	   variable in the option parser callbacks */
	struct opts* opts;
};


/* ----------------------------------------------------------------------- */

/* 
 * Globals
 */

/* line buffer for list elements */
static const int buflen = 512;
char buf[512];

struct opts opts;
struct app app;

/* The Listview Component */
struct nk_listview listview;

/* ----------------------------------------------------------------------- */

void usage()
{
	puts("xlistsel [options] [elem1 [elem2 [...]]]");
}

/* ----------------------------------------------------------------------- */

void output_result_and_exit(int btn)
{
	if ( listview.selected >= 0 && listview.selected < app.numtabrows )
	{
		if (opts.output_col >= 0)
		{
			printf("%s\n", app.tabrows[listview.selected][opts.output_col]);
		}
		else if(opts.output_col == -1)
		{
			int i;
			for(i = 0; i < app.numcols; i++)
			{
				if (i > 0) printf("%s", opts.sep);
				printf("%s", app.tabrows[listview.selected][i]);
			}
			printf("\n");		
		}
		else
		{
			errx(-1, "invalid opts.output_col value: %i", opts.output_col);
		}
	}
	exit(btn+1); /* never return 0 when button pressed: 0 is: window closed */
}

/* ----------------------------------------------------------------------- */

/* calculate the height of the listbox which must be provided
   as absolute value to nuklear */
float calc_list_height(struct nk_context* ctx, int h)
{
	int button_rows = 
		(app.numbuttons / opts.override_button_cols)
		+ ((app.numbuttons % opts.override_button_cols) > 0 ? 1 : 0);
	
	float tbheight = 
		  ctx->style.window.spacing.y * 2
		+ ctx->style.window.padding.y * 2
		+ ctx->current->layout->row.min_height * button_rows;
	
	return
		h 
		/* offset of the list */
		- ctx->current->layout->at_y 
		- ctx->current->layout->row.height
		/* height of the toolbar */
		- tbheight
		/* Window padding */
		/*- ctx->style.window.scrollbar_size.y (we use NK_WINDOW_NO_SCROLLBAR) */
	;
}

/* ----------------------------------------------------------------------- */

int app_draw(struct nk_context* ctx, int w, int h, void* userdata)
{
	/* nuklear is an immediate mode GUI library, which means
	   the code of this function is run every time when the window
	   must be drawn or an event happens. Theses functions create
	   draw commands in some internal nuklear structure which are
	   converted into X11 Draw commands after thus functions has finished. 
	
		 One Result of this limitation is, that the code here should try to
		 not waste too much time.
	*/

	if (nk_begin(ctx, opts.title, nk_rect(0, 0, w, h), NK_WINDOW_NO_SCROLLBAR))
	{
		int i;
		
		set_style(ctx, opts.style);
		
		/*
		 * Description Label
		 */
		
		if(opts.desc)
		{
			nk_layout_row_dynamic(ctx, ctx->style.font->height, 1);
			for(i=0; i < app.num_desc_lines; i++)
			{
				nk_label(ctx, app.desc_lines[i], NK_TEXT_LEFT);
			}
		}
		
		/* 
		 * the Listbox
		 */

		/* List app.header if set */
		if(app.header && app.numheader > 0)
			nk_listview_head(ctx, &listview, app.numcols, app.header);
		
		
		/* the actual elements */
		nk_layout_row_dynamic(ctx, calc_list_height(ctx, h), 1);
		nk_listview_draw(ctx, &listview, app.numcols, app.numtabrows, app.tabrows);
		
		
		/* 
		 * buttons below list 
		 */
		 
		nk_layout_row_dynamic(ctx, 0, opts.override_button_cols);
		for(i = 0; i < app.numbuttons; i++)
		{
			if(i == opts.okay_btn) /* highlight the active button (okay_btn)*/
				nk_style_push_style_item(ctx, 
					&ctx->style.button.normal, ctx->style.button.hover);
			if (nk_button_label(ctx, app.buttons[i][0]))
					output_result_and_exit(i);
			if(i == opts.okay_btn) /* highlight the active button */
				nk_style_pop_style_item(ctx);
		}
	}
	
	/* 
	 * Keyboard user input 
	 */
	
	/* Return: Confirm Dialog */
	if (nk_input_is_key_pressed(&ctx->input, NK_KEY_ENTER)) {
		output_result_and_exit(opts.okay_btn);

	/* Tab or Reight: next button */
	} else if (
		(nk_input_is_key_pressed(&ctx->input, NK_KEY_TAB)
		&& !nk_input_is_key_down(&ctx->input, NK_KEY_SHIFT))
		|| nk_input_is_key_pressed(&ctx->input, NK_KEY_RIGHT)
	) {
		opts.okay_btn++;
		if(opts.okay_btn >= app.numbuttons)
			opts.okay_btn = 0;
	
	/* Shift-Tab or Left: previous Button */
	} else if (
		(nk_input_is_key_pressed(&ctx->input, NK_KEY_TAB)
		&& nk_input_is_key_down(&ctx->input, NK_KEY_SHIFT))
		|| nk_input_is_key_pressed(&ctx->input, NK_KEY_LEFT)
	) {
		opts.okay_btn--;
		if(opts.okay_btn < 0)
			opts.okay_btn = app.numbuttons - 1;
	
	/* Escape: exit (if set as option) */	
	} else if (opts.escape_to_exit && 
		  nk_input_is_key_pressed(&ctx->input, NK_KEY_TEXT_RESET_MODE)) {
		exit(0);
	}
	
	nk_end(ctx);
	
	if (nk_window_is_hidden(ctx, opts.title)) return 0;
		
	return 1;
}

/* ----------------------------------------------------------------------- */

/* just a small wrapper to make main more readable */
static void add_row(const char* s)
{
	if(
		NULL == 
		table_append(&app.tabrows, &app.numtabrows, &app.numcols, s, opts.sep)
	)
		errx(-1, "error appending element to Table");
}

/* ----------------------------------------------------------------------- */

/* just a small wrapper to make main more readable */
static void add_button(const char* s)
{
	if(
		NULL ==
		table_append(&app.buttons, &app.numbuttons, &app.numbuttoncols, s, ":")
	)
		errx(-1, "error adding button");
}

/* ----------------------------------------------------------------------- */

/* just a small wrapper to make main more readable */
static void set_header(const char* s)
{
	if(
		NULL ==
		table_append(&app.header, &app.numheader, &app.numcols, s, opts.sep)
	)
		errx(-1, "error adding button");
}

/* ----------------------------------------------------------------------- */

/* called by opts_parse() */
int app_add_button(void* user, const char* arg, const char** errstr)
{
	add_button(arg);
	return 0;
}

/* ----------------------------------------------------------------------- */

/* called by opts_parse() */
int app_set_header(void* user, const char* arg, const char** errstr)
{
	set_header(arg);
	return 0;
}

/* ----------------------------------------------------------------------- */

/* called by opts_parse() */
int app_help(void* user, const char* arg, const char** errstr)
{
	usage();
	puts("Options:");
	puts("");
	opts_help();
	exit(-1);
}

/* ----------------------------------------------------------------------- */

int main(int argc, char** argv)
{
	int ch, i;
	const char* errstr;
	char* endp;
	char* p;
	int optend;
	
	/* our application state */
	errstr = NULL;
	
	app.buttons = 0;
	app.numbuttons = 0;
	app.numbuttoncols = 0;

	app.tabrows = 0;
	app.numtabrows = 0;
	app.numcols = 0;
	
	app.num_desc_lines = 0;
	
	app.header = 0;
	app.numheader = 0;

	/* Init and Parse the options */
	app.opts = &opts;
	opts_init(&opts);
	if((optend = opts_parse(&opts, &app, argc, argv)) < 0)
		errx(-1, "error in parameter '%s': %s", opts.errarg, opts.errstr);
	argc -= optend;
	argv += optend;

	/* init the listbox */
	nk_listview_init(&listview, "Listview");
	nk_listview_select(&listview, 0);   /* select first entry */
	nk_listview_activate(&listview, 1); /* set focus to listview */

	/* read list elements from command line (higher priority then stdin) */
	if (opts.read_stdin == 0)
	{
		for(i = 0; i < argc; i++)
			add_row(argv[i]);
	}
	else
	{
		/* read app.header line if requested */
		if(opts.first_line_is_header && fgets(buf, buflen, stdin))
		{
			char* p; strtok_r(buf, "\r\n", &p); /* remove lineend */
			set_header(buf);
		}

		/* read list elements from stdin */
		while(fgets(buf, buflen, stdin))
		{
			char* p; strtok_r(buf, "\r\n", &p); /* remove lineend */	
			add_row(buf);
		}
	}
	
	/* count linebreaks in the description */
	if(opts.desc)
	{
		char* p = 0;
		char* last = 0;
		for (
			(p = strtok_r(opts.desc, "\n", &last)); 
			 p;
			(p = strtok_r(NULL, "\n", &last)))
		{
			app.desc_lines[app.num_desc_lines] = p;
			app.num_desc_lines++;
			if(app.num_desc_lines >= MAX_DESC_LINES)
				errx(-1, "too much lines in description");
		}
	}
	
	/* special case: no button defined */
	if(app.numbuttons == 0)
		add_button("Ok");
	
	/* final checks */
	if(opts.output_col < -1 || opts.output_col >= app.numcols)
		errx(-1, "output column outside of range (0, %i): %i", 
			app.numcols, opts.output_col);
	if(opts.okay_btn < 0 || opts.okay_btn >= app.numbuttons)
		errx(-1, "okay button outside of range (1, %i): %i", 
			app.numbuttons, opts.okay_btn+1);
	if(opts.override_button_cols <= 0)
			opts.override_button_cols = app.numbuttons;
	
	/* Run the User Interface. see app_draw() above for Details */
	nkapp_run(app_draw, 0, opts.winwidth, opts.winheight, opts.title);
}

