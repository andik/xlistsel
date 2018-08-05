/*
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

struct nk_listview_row;

int count_columns(const char* s, const char* sep);
char** row_from_string(const char* s, const char* sep, int *ncols);
char*** table_add_row(char**** rows, int* nrows, char** row);
char*** table_append(char**** rows, int* nrows, int* ncols, const char* s, const char* sep);

int parse_boolean(const char* s, int* value);

/* usage is a generated function, see Makefile and usage.c */
void usage();

/* Styling the Dialog */
enum theme {THEME_BLACK, THEME_WHITE, THEME_RED, THEME_BLUE, THEME_DARK};
void set_style(void *ptr, enum theme theme);
int parse_style(const char* s, int* value);
