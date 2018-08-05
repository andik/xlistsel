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

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <sys/limits.h>

#include "extern.h"

#define MAX_ELEMS 10000

/* ---------------------------------------------------------------------- */

int parse_boolean(const char* s, int* value)
{
	     if(strcmp(s,"1") == 0)    *value = 1;
	else if(strcmp(s,"yes") == 0)  *value = 1;
	else if(strcmp(s,"Yes") == 0)  *value = 1;
	else if(strcmp(s,"YES") == 0)  *value = 1;
	else if(strcmp(s,"y") == 0)    *value = 1;
	else if(strcmp(s,"Y") == 0)    *value = 1;
	else if(strcmp(s,"True") == 0) *value = 1;
	else if(strcmp(s,"TRUE") == 0) *value = 1;
	else if(strcmp(s,"true") == 0) *value = 1;
	
	else if(strcmp(s,"0") == 0)      *value = 0;
	else if(strcmp(s,"no") == 0)     *value = 0;
	else if(strcmp(s,"No") == 0)     *value = 0;
	else if(strcmp(s,"NO") == 0)     *value = 0;
	else if(strcmp(s,"false") == 0)  *value = 0;
	else if(strcmp(s,"False") == 0)  *value = 0;
	else if(strcmp(s,"FALSE") == 0)  *value = 0;
	else if(strcmp(s,"n") == 0)      *value = 0;
	else if(strcmp(s,"N") == 0)      *value = 0;
	
	else return -1;
	
	return 0;
}

/* ---------------------------------------------------------------------- */

/* count the number of columns in a string s split by sep*/
int count_columns(const char* s, const char* sep)
{
	int ncols;
	const char* p;
	
	ncols = 0;
	for (p = s; p; (p = strpbrk(p, sep))) 
	{
		p++; /* move past the separator character */
		ncols++;
	}
	
	return ncols;
}

/* ---------------------------------------------------------------------- */

char***
table_add_row(char**** rows, int* nrows, char** row)
{
	/* table structure:
	
		char**[numrows] rows -> char*[numcols] row -> colum string 
	*/

	int newrows = *nrows + 1;

	/* Avoid invalid requests */
	if (newrows < 0)
	{
		errno = EOVERFLOW;
		return NULL;
	}

	/* Check for signed int overflow */
	if (newrows > INT_MAX / sizeof(char**))
	{
		errno = EOVERFLOW;
		return NULL;
	}

	if((*rows = realloc(*rows, newrows * sizeof(char**))) == NULL)
		return NULL;
	
	(*rows)[*nrows] = row;	
	(*nrows)++;
	return *rows;
}

/* ---------------------------------------------------------------------- */

char***
table_append(char**** rows, int* nrows, int* ncols, const char* s, const char* sep)
{
	char** row;
	void * mem;
	int column;
	
	/* split line into columns.
	Note: if no column number has been set, row_from_string() defines the column
	number here. Which means the first line which is parsed defines the number
	for columns for the whole table. */
	if((row = row_from_string(s, sep, ncols)) == NULL)
		return NULL;
	
	/* Allocate the new table size */
	if(table_add_row(rows, nrows, row) == NULL)
		return NULL;
	
	return *rows;
}

/* ---------------------------------------------------------------------- */

void free_row(char** row, int ncols)
{
	int i;
	
	for(i = 0; i < ncols; i++)
	{
		if(row[i] != NULL) 
		{
			free(row[i]);
			row[i] = 0;
		}
	}
	free(row);
}

/* ---------------------------------------------------------------------- */

/* allocates a char array of size 'ncols' and assigns the columns of the
   string 's' splitted by 'sep' to its elements. If there are less
   columns in the string than 'ncols' the rest is filled with NULL. */
char** row_from_string(const char* s, const char* sep, int* ncols)
{
	char* line, *p, *last;
	int column;
	char** row;
	
	/* first: get the number of columns: if zero the number is defined
	   by the first line that is added to the table */
	if (*ncols == 0)
		*ncols = count_columns(s, sep);
	
	row = calloc(*ncols, sizeof(char*));
	if(row==NULL) return NULL;

	/* split line into columns */

	column = 0;
	line = strdup(s); /* we need a buffer for strtok... :-( */
	if(line==NULL) return NULL;
	
	for ((p = strtok_r(line, sep, &last)); p; (p = strtok_r(NULL, sep, &last)))
	{
		row[column] = strdup(p);
		column++;
		if(column >= *ncols) break;
	}
	
	free(line);
	return row;
}
