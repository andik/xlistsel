#ifndef NK_LISTVIEW_H
#define NK_LISTVIEW_H

/*

	simple Listview with selection to select an item out of a list of strings.

	You maintain the list yourself. 
	(Because this is the way of immediate mode UIs.)
	Listview cares about the drawing and the selection of the 
	element
	

	authored from 2015-2018 by Micha Mettke and Andreas Koerner

LICENSE:
    This software is dual-licensed to the public domain and under the following
    license: you are granted a perpetual, irrevocable license to copy, modify,
    publish and distribute this file as you see fit.

*/

/* do not allow that much colums */
#define NK_LISTVIEW_MAX_COLS 16


struct nk_listview
{
	/* selected index: -1: none selected */
	int selected;
	/* title of the nk group */
	const char* grpname;
	/* true, when reacting to user keys */
	int active;
	/* scrolling position of the group ... */
	struct nk_scroll scroll;
	/* height of a single line */
	int item_height;
};

struct nk_context;

int nk_listview_init(struct nk_listview* self, const char* grpname);
int nk_listview_head(struct nk_context* ctx, struct nk_listview* self, int ncols, char*** header);
int nk_listview_draw(struct nk_context* ctx, struct nk_listview* self, int ncols, int nrows, char*** rows);
int nk_listview_select(struct nk_listview* self, int entry);
int nk_listview_activate(struct nk_listview* self, int state);

#endif /* NK_LISTVIEW_H */
