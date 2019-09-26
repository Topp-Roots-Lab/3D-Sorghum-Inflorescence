#pragma once
#include <map>
#include <iostream>
#include <vector>
#include <string>
#include <ply.h>




namespace ply
{

	char *type_names[] = {  /* names of scalar types */
		(char*)"invalid",
		(char*)"int8", (char*)"int16", (char*)"int32", (char*)"uint8", (char*)"uint16", (char*)"uint32", (char*)"float32", (char*)"float64",
	};

	char *old_type_names[] = {  /* old names of types for backward compatability */
		(char*)"invalid",
		(char*)"char", (char*)"short",(char*) "int", (char*)"uchar", (char*)"ushort", (char*)"uint", (char*)"float", (char*)"double",
	};

#define NO_OTHER_PROPS  -1

#define DONT_STORE_PROP  0
#define STORE_PROP       1

#define OTHER_PROP       0
#define NAMED_PROP       1

	int ply_type_size[] = {
		0, 1, 2, 4, 1, 2, 4, 4, 8
	};

	static char *my_alloc(int size, int lnum, const char *fname)
	{
		char *ptr;

		ptr = (char *)malloc(size);

		if (ptr == 0) {
			fprintf(stderr, "Memory allocation bombed on line %d in %s\n", lnum, fname);
		}

		return (ptr);
	}


	/**** NEW STUFF ****/
	/**** NEW STUFF ****/
	/**** NEW STUFF ****/
	/**** NEW STUFF ****/

	int equal_strings(const char *s1, char *s2)
	{
		int i;

		while (*s1 && *s2)
			if (*s1++ != *s2++)
				return (0);

		if (*s1 != *s2)
			return (0);
		else
			return (1);
	}



	/******************************************************************************
	Given a file pointer, get ready to read PLY data from the file.

	Entry:
	fp - the given file pointer

	Exit:
	nelems     - number of elements in object
	elem_names - list of element names
	returns a pointer to a PlyFile, used to refer to this file, or NULL if error
	******************************************************************************/



	char **get_words(FILE *fp, int *nwords, char **orig_line)
	{
#define BIG_STRING 4096
		int i, j;
		static char str[BIG_STRING];
		static char str_copy[BIG_STRING];
		char **words;
		int max_words = 10;
		int num_words = 0;
		char *ptr, *ptr2;
		char *result;

		words = (char **)myalloc(sizeof(char *) * max_words);

		/* read in a line */
		result = fgets(str, BIG_STRING, fp);
		if (result == NULL) {
			*nwords = 0;
			*orig_line = NULL;
			return (NULL);
		}

		/* convert line-feed and tabs into spaces */
		/* (this guarentees that there will be a space before the */
		/*  null character at the end of the string) */

		str[BIG_STRING - 2] = ' ';
		str[BIG_STRING - 1] = '\0';

		for (ptr = str, ptr2 = str_copy; *ptr != '\0'; ptr++, ptr2++) {
			*ptr2 = *ptr;
			if (*ptr == '\t') {
				*ptr = ' ';
				*ptr2 = ' ';
			}
			else if (*ptr == '\n') {
				*ptr = ' ';
				*ptr2 = '\0';
				break;
			}
		}

		/* find the words in the line */

		ptr = str;
		while (*ptr != '\0') {

			/* jump over leading spaces */
			while (*ptr == ' ')
				ptr++;

			/* break if we reach the end */
			if (*ptr == '\0')
				break;

			/* allocate more room for words if necessary */
			if (num_words >= max_words) {
				max_words += 10;
				words = (char **)realloc(words, sizeof(char *) * max_words);
			}

			if (*ptr == '\"') {  /* a quote indidicates that we have a string */

								 /* skip over leading quote */
				ptr++;

				/* save pointer to beginning of word */
				words[num_words++] = ptr;

				/* find trailing quote or end of line */
				while (*ptr != '\"' && *ptr != '\0')
					ptr++;

				/* replace quote with a null character to mark the end of the word */
				/* if we are not already at the end of the line */
				if (*ptr != '\0')
					*ptr++ = '\0';
			}
			else {               /* non-string */

								 /* save pointer to beginning of word */
				words[num_words++] = ptr;

				/* jump over non-spaces */
				while (*ptr != ' ')
					ptr++;

				/* place a null character here to mark the end of the word */
				*ptr++ = '\0';
			}
		}

		/* return the list of words */
		*nwords = num_words;
		*orig_line = str_copy;
		return (words);
	}


	/******************************************************************************
	Return the value of an item, given a pointer to it and its type.

	Entry:
	item - pointer to item
	type - data type that "item" points to

	Exit:
	returns a double-precision float that contains the value of the item
	******************************************************************************/

	double get_item_value(char *item, int type)
	{
		unsigned char *puchar;
		char *pchar;
		short int *pshort;
		unsigned short int *pushort;
		int *pint;
		unsigned int *puint;
		float *pfloat;
		double *pdouble;
		int int_value;
		unsigned int uint_value;
		double double_value;

		switch (type) {
		case Int8:
			pchar = (char *)item;
			int_value = *pchar;
			return ((double)int_value);
		case Uint8:
			puchar = (unsigned char *)item;
			int_value = *puchar;
			return ((double)int_value);
		case Int16:
			pshort = (short int *)item;
			int_value = *pshort;
			return ((double)int_value);
		case Uint16:
			pushort = (unsigned short int *) item;
			int_value = *pushort;
			return ((double)int_value);
		case Int32:
			pint = (int *)item;
			int_value = *pint;
			return ((double)int_value);
		case Uint32:
			puint = (unsigned int *)item;
			uint_value = *puint;
			return ((double)uint_value);
		case Float32:
			pfloat = (float *)item;
			double_value = *pfloat;
			return (double_value);
		case Float64:
			pdouble = (double *)item;
			double_value = *pdouble;
			return (double_value);
		default:
			fprintf(stderr, "get_item_value: bad type = %d\n", type);
			exit(-1);
		}

		return (0.0);  /* never actually gets here */
	}

	void add_element(PlyFile *plyfile, char **words, int nwords)
	{
		PlyElement *elem;

		/* create the new element */
		elem = (PlyElement *)myalloc(sizeof(PlyElement));
		elem->name = _strdup(words[1]);
		elem->num = atoi(words[2]);
		elem->nprops = 0;

		/* make room for new element in the object's list of elements */
		if (plyfile->num_elem_types == 0)
			plyfile->elems = (PlyElement **)myalloc(sizeof(PlyElement *));
		else
			plyfile->elems = (PlyElement **)realloc(plyfile->elems,
				sizeof(PlyElement *) * (plyfile->num_elem_types + 1));

		/* add the new element to the object's list */
		plyfile->elems[plyfile->num_elem_types] = elem;
		plyfile->num_elem_types++;
	}


	int get_prop_type(char *type_name)
	{
		int i;

		/* try to match the type name */
		for (i = StartType + 1; i < EndType; i++)
			if (equal_strings(type_name, (char*)type_names[i]))
				return (i);

		/* see if we can match an old type name */
		for (i = StartType + 1; i < EndType; i++)
			if (equal_strings(type_name, (char*)old_type_names[i]))
				return (i);

		/* if we get here, we didn't find the type */
		return (0);
	}






	/******************************************************************************
	Copy the comments from one PLY file to another.

	Entry:
	out_ply - destination file to copy comments to
	in_ply  - the source of the comments
	******************************************************************************/
	/**
	void copy_comments_ply(PlyFile *out_ply, PlyFile *in_ply)
	{
	int i;

	for (i = 0; i < in_ply->num_comments; i++)
	append_comment_ply(out_ply, in_ply->comments[i]);
	}**/


	/******************************************************************************
	Append object information (arbitrary text) to a PLY file.

	Entry:
	ply      - file to append object info to
	obj_info - the object info to append
	******************************************************************************/

	/******************************************************************************
	Append object information (arbitrary text) to a PLY file.

	Entry:
	ply      - file to append object info to
	obj_info - the object info to append
	******************************************************************************/

	void append_obj_info_ply(PlyFile *ply, char *obj_info)
	{
		/* (re)allocate space for new info */
		if (ply->num_obj_info == 0)
			ply->obj_info = (char **)myalloc(sizeof(char *));
		else
			ply->obj_info = (char **)realloc(ply->obj_info,
				sizeof(char *) * (ply->num_obj_info + 1));

		/* add info to list */
		ply->obj_info[ply->num_obj_info] = _strdup(obj_info);
		ply->num_obj_info++;
	}


	/******************************************************************************
	Copy the object information from one PLY file to another.

	Entry:
	out_ply - destination file to copy object information to
	in_ply  - the source of the object information
	******************************************************************************/

	void copy_obj_info_ply(PlyFile *out_ply, PlyFile *in_ply)
	{
		int i;

		for (i = 0; i < in_ply->num_obj_info; i++)
			append_obj_info_ply(out_ply, in_ply->obj_info[i]);
	}

	void append_comment_ply(PlyFile *ply, char *comment)
	{
		/* (re)allocate space for new comment */
		if (ply->num_comments == 0)
			ply->comments = (char **)myalloc(sizeof(char *));
		else
			ply->comments = (char **)realloc(ply->comments,
				sizeof(char *) * (ply->num_comments + 1));

		/* add comment to list */
		ply->comments[ply->num_comments] = _strdup(comment);
		ply->num_comments++;
	}

	void store_item(
		char *item,
		int type,
		int int_val,
		unsigned int uint_val,
		double double_val
	)
	{
		unsigned char *puchar;
		short int *pshort;
		unsigned short int *pushort;
		int *pint;
		unsigned int *puint;
		float *pfloat;
		double *pdouble;

		switch (type) {
		case Int8:
			*item = int_val;
			break;
		case Uint8:
			puchar = (unsigned char *)item;
			*puchar = uint_val;
			break;
		case Int16:
			pshort = (short *)item;
			*pshort = int_val;
			break;
		case Uint16:
			pushort = (unsigned short *)item;
			*pushort = uint_val;
			break;
		case Int32:
			pint = (int *)item;
			*pint = int_val;
			break;
		case Uint32:
			puint = (unsigned int *)item;
			*puint = uint_val;
			break;
		case Float32:
			pfloat = (float *)item;
			*pfloat = double_val;
			break;
		case Float64:
			pdouble = (double *)item;
			*pdouble = double_val;
			break;
		default:
			fprintf(stderr, "store_item: bad type = %d\n", type);
			exit(-1);
		}
	}

	/******************************************************************************
	Extract the value of an item from an ascii word, and place the result
	into an integer, an unsigned integer and a double.

	Entry:
	word - word to extract value from
	type - data type supposedly in the word

	Exit:
	int_val    - integer value
	uint_val   - unsigned integer value
	double_val - double-precision floating point value
	******************************************************************************/

	void get_ascii_item(
		char *word,
		int type,
		int *int_val,
		unsigned int *uint_val,
		double *double_val
	)
	{
		switch (type) {
		case Int8:
		case Uint8:
		case Int16:
		case Uint16:
		case Int32:
			*int_val = atoi(word);
			*uint_val = *int_val;
			*double_val = *int_val;
			break;

		case Uint32:
			*uint_val = strtoul(word, (char **)NULL, 10);
			*int_val = *uint_val;
			*double_val = *uint_val;
			break;

		case Float32:
		case Float64:
			*double_val = atof(word);
			*int_val = (int)*double_val;
			*uint_val = (unsigned int)*double_val;
			break;

		default:
			fprintf(stderr, "get_ascii_item: bad type = %d\n", type);
			exit(-1);
		}
	}

	void ascii_get_element(PlyFile *plyfile, char *elem_ptr)
	{
		int i, j, k;
		PlyElement *elem;
		PlyProperty *prop;
		char **words;
		int nwords;
		int which_word;
		FILE *fp = plyfile->fp;
		char *elem_data = nullptr, *item = nullptr;
		char *item_ptr = nullptr;
		int item_size;
		int int_val;
		unsigned int uint_val;
		double double_val;
		int list_count;
		int store_it;
		char **store_array;
		char *orig_line = nullptr;
		char *other_data = nullptr;
		int other_flag;

		/* the kind of element we're reading currently */
		elem = plyfile->which_elem;

		/* do we need to setup for other_props? */

		if (elem->other_offset != NO_OTHER_PROPS) {
			char **ptr;
			other_flag = 1;
			/* make room for other_props */
			other_data = (char *)myalloc(elem->other_size);
			/* store pointer in user's structure to the other_props */
			ptr = (char **)(elem_ptr + elem->other_offset);
			*ptr = other_data;
		}
		else
			other_flag = 0;

		/* read in the element */

		words = get_words(plyfile->fp, &nwords, &orig_line);
		if (words == NULL) {
			fprintf(stderr, "ply_get_element: unexpected end of file\n");
			exit(-1);
		}

		which_word = 0;

		for (j = 0; j < elem->nprops; j++) {

			prop = elem->props[j];
			store_it = (elem->store_prop[j] | other_flag);

			/* store either in the user's structure or in other_props */
			if (elem->store_prop[j])
				elem_data = elem_ptr;
			else
				elem_data = other_data;

			if (prop->is_list == PLY_LIST) {       /* a list */

												   /* get and store the number of items in the list */
				get_ascii_item(words[which_word++], prop->count_external,
					&int_val, &uint_val, &double_val);
				if (store_it) {
					item = elem_data + prop->count_offset;
					store_item(item, prop->count_internal, int_val, uint_val, double_val);
				}

				/* allocate space for an array of items and store a ptr to the array */
				list_count = int_val;
				item_size = ply_type_size[prop->internal_type];
				store_array = (char **)(elem_data + prop->offset);

				if (list_count == 0) {
					if (store_it)
						*store_array = NULL;
				}
				else {
					if (store_it) {
						//item_ptr = (char *)myalloc( sizeof( char ) * item_size * list_count );
						//item = item_ptr;
						//*store_array = item_ptr;
						item = elem_data + prop->offset;
					}

					/* read items and store them into the array */
					for (k = 0; k < list_count; k++) {
						get_ascii_item(words[which_word++], prop->external_type,
							&int_val, &uint_val, &double_val);
						if (store_it) {
							store_item(item, prop->internal_type,
								int_val, uint_val, double_val);
							item += item_size;
						}
					}
				}

			}
			else if (prop->is_list == PLY_STRING) {   /* a string */
				if (store_it) {
					char *str;
					char **str_ptr;
					str = _strdup(words[which_word++]);
					item = elem_data + prop->offset;
					str_ptr = (char **)item;
					*str_ptr = str;
				}
				else {
					which_word++;
				}
			}
			else {                     /* a scalar */
				get_ascii_item(words[which_word++], prop->external_type,
					&int_val, &uint_val, &double_val);
				if (store_it) {
					item = elem_data + prop->offset;
					store_item(item, prop->internal_type, int_val, uint_val, double_val);
				}
			}

		}

		free(words);
	}

	void get_binary_item(
		FILE *fp,
		int type,
		int *int_val,
		unsigned int *uint_val,
		double *double_val
	)
	{
		char c[8];
		void *ptr;

		ptr = (void *)c;

		switch (type) {
		case Int8:
			fread(ptr, 1, 1, fp);
			*int_val = *((char *)ptr);
			*uint_val = *int_val;
			*double_val = *int_val;
			break;
		case Uint8:
			fread(ptr, 1, 1, fp);
			*uint_val = *((unsigned char *)ptr);
			*int_val = *uint_val;
			*double_val = *uint_val;
			break;
		case Int16:
			fread(ptr, 2, 1, fp);
			*int_val = *((short int *)ptr);
			*uint_val = *int_val;
			*double_val = *int_val;
			break;
		case Uint16:
			fread(ptr, 2, 1, fp);
			*uint_val = *((unsigned short int *) ptr);
			*int_val = *uint_val;
			*double_val = *uint_val;
			break;
		case Int32:
			fread(ptr, 4, 1, fp);
			*int_val = *((int *)ptr);
			*uint_val = *int_val;
			*double_val = *int_val;
			break;
		case Uint32:
			fread(ptr, 4, 1, fp);
			*uint_val = *((unsigned int *)ptr);
			*int_val = *uint_val;
			*double_val = *uint_val;
			break;
		case Float32:
			fread(ptr, 4, 1, fp);
			*double_val = *((float *)ptr);
			*int_val = *double_val;
			*uint_val = *double_val;
			break;
		case Float64:
			fread(ptr, 8, 1, fp);
			*double_val = *((double *)ptr);
			*int_val = *double_val;
			*uint_val = *double_val;
			break;
		default:
			fprintf(stderr, "get_binary_item: bad type = %d\n", type);
			exit(-1);
		}
	}





	/******************************************************************************
	Read an element from a binary file.

	Entry:
	plyfile  - file identifier
	elem_ptr - pointer to an element
	******************************************************************************/

	void binary_get_element(PlyFile *plyfile, char *elem_ptr)
	{
		int i, j, k;
		PlyElement *elem = nullptr;
		PlyProperty *prop = nullptr;
		FILE *fp = plyfile->fp;
		char *elem_data = nullptr;
		char *item = nullptr;
		char *item_ptr = nullptr;
		int item_size;
		int int_val;
		unsigned int uint_val;
		double double_val;
		int list_count;
		int store_it;
		char **store_array = nullptr;
		char *other_data = nullptr;
		int other_flag;

		/* the kind of element we're reading currently */
		elem = plyfile->which_elem;

		/* do we need to setup for other_props? */

		if (elem->other_offset != NO_OTHER_PROPS) {
			char **ptr;
			other_flag = 1;
			/* make room for other_props */
			other_data = (char *)myalloc(elem->other_size);
			/* store pointer in user's structure to the other_props */
			ptr = (char **)(elem_ptr + elem->other_offset);
			*ptr = other_data;
		}
		else
			other_flag = 0;

		/* read in a number of elements */

		for (j = 0; j < elem->nprops; j++) {

			prop = elem->props[j];
			store_it = (elem->store_prop[j] | other_flag);

			/* store either in the user's structure or in other_props */
			if (elem->store_prop[j])
				elem_data = elem_ptr;
			else
				elem_data = other_data;

			if (prop->is_list == PLY_LIST) {          /* list */

													  /* get and store the number of items in the list */
				get_binary_item(fp, prop->count_external,
					&int_val, &uint_val, &double_val);
				if (store_it) {
					item = elem_data + prop->count_offset;
					store_item(item, prop->count_internal, int_val, uint_val, double_val);
				}

				/* allocate space for an array of items and store a ptr to the array */
				list_count = int_val;
				item_size = ply_type_size[prop->internal_type];
				store_array = (char **)(elem_data + prop->offset);
				if (list_count == 0) {
					if (store_it)
						*store_array = NULL;
				}
				else {
					if (store_it) {
						item_ptr = (char *)myalloc(sizeof(char) * item_size * list_count);
						item = item_ptr;
						*store_array = item_ptr;
					}

					/* read items and store them into the array */
					for (k = 0; k < list_count; k++) {
						get_binary_item(fp, prop->external_type,
							&int_val, &uint_val, &double_val);
						if (store_it) {
							store_item(item, prop->internal_type,
								int_val, uint_val, double_val);
							item += item_size;
						}
					}
				}

			}
			else if (prop->is_list == PLY_STRING) {     /* string */
				int len;
				char *str;
				fread(&len, sizeof(int), 1, fp);
				str = (char *)myalloc(len);
				fread(str, len, 1, fp);
				if (store_it) {
					char **str_ptr;
					item = elem_data + prop->offset;
					str_ptr = (char **)item;
					*str_ptr = str;
				}
			}
			else {                                      /* scalar */
				get_binary_item(fp, prop->external_type,
					&int_val, &uint_val, &double_val);
				if (store_it) {
					item = elem_data + prop->offset;
					store_item(item, prop->internal_type, int_val, uint_val, double_val);
				}
			}

		}
	}

	void get_element_ply(PlyFile *plyfile, void *elem_ptr)
	{
		if (plyfile->file_type == PLY_ASCII)
			ascii_get_element(plyfile, (char *)elem_ptr);
		else
			binary_get_element(plyfile, (char *)elem_ptr);
	}

	PlyProperty *find_property(PlyElement *elem, const char *prop_name, int *index)
	{
		int i;

		for (i = 0; i < elem->nprops; i++)
			if (equal_strings(prop_name, elem->props[i]->name)) {
				*index = i;
				return (elem->props[i]);
			}

		*index = -1;
		return (NULL);
	}

	void setup_property_ply(
		PlyFile *plyfile,
		const PlyProperty *prop
	)
	{
		PlyElement *elem;
		PlyProperty *prop_ptr;
		int index;

		elem = plyfile->which_elem;

		/* deposit the property information into the element's description */

		prop_ptr = find_property(elem, prop->name, &index);
		if (prop_ptr == NULL) {
			fprintf(stderr, "Warning:  Can't find property '%s' in element '%s'\n",
				prop->name, elem->name);
			return;
		}
		prop_ptr->internal_type = prop->internal_type;
		prop_ptr->offset = prop->offset;
		prop_ptr->count_internal = prop->count_internal;
		prop_ptr->count_offset = prop->count_offset;

		/* specify that the user wants this property */
		elem->store_prop[index] = STORE_PROP;
	}


	/******************************************************************************
	Copy the comments from one PLY file to another.

	Entry:
	out_ply - destination file to copy comments to
	in_ply  - the source of the comments
	******************************************************************************/

	void copy_comments_ply(PlyFile *out_ply, PlyFile *in_ply)
	{
		int i;

		for (i = 0; i < in_ply->num_comments; i++)
			append_comment_ply(out_ply, in_ply->comments[i]);
	}

	void add_comment(PlyFile *plyfile, char *line)
	{
		int i;

		/* skip over "comment" and leading spaces and tabs */
		i = 7;
		while (line[i] == ' ' || line[i] == '\t')
			i++;

		append_comment_ply(plyfile, &line[i]);
	}

	void add_property(PlyFile *plyfile, char **words, int nwords)
	{
		/*int prop_type;
		int count_type;*/
		PlyProperty *prop;
		PlyElement *elem;

		/* create the new property */

		prop = (PlyProperty *)myalloc(sizeof(PlyProperty));

		if (equal_strings(words[1], (char *)"list")) {          /* list */
			prop->count_external = get_prop_type(words[2]);
			prop->external_type = get_prop_type(words[3]);
			prop->name = _strdup(words[4]);
			prop->is_list = PLY_LIST;
		}
		else if (equal_strings(words[1], (char *)"string")) {   /* string */
			prop->count_external = Int8;
			prop->external_type = Int8;
			prop->name = _strdup(words[2]);
			prop->is_list = PLY_STRING;
		}
		else {                                           /* scalar */
			prop->external_type = get_prop_type(words[1]);
			prop->name = _strdup(words[2]);
			prop->is_list = PLY_SCALAR;
		}

		/* add this property to the list of properties of the current element */

		elem = plyfile->elems[plyfile->num_elem_types - 1];

		if (elem->nprops == 0)
			elem->props = (PlyProperty **)myalloc(sizeof(PlyProperty *));
		else
			elem->props = (PlyProperty **)realloc(elem->props,
				sizeof(PlyProperty *) * (elem->nprops + 1));

		elem->props[elem->nprops] = prop;
		elem->nprops++;
	}

	void add_obj_info(PlyFile *plyfile, char *line)
	{
		int i;

		i = 8;
		while (line[i] == ' ' || line[i] == '\t')
			i++;

		append_obj_info_ply(plyfile, &line[i]);
	}

	PlyFile *ply_read(FILE *fp, int *nelems, char ***elem_names)
	{
		int i, j;
		PlyFile *plyfile;
		int nwords;
		char **words;
		int found_format = 0;
		char **elist;
		PlyElement *elem;
		char *orig_line;

		/* check for NULL file pointer */
		if (fp == NULL)
			return (NULL);

		/* create record for this object */

		plyfile = (PlyFile *)myalloc(sizeof(PlyFile));
		plyfile->num_elem_types = 0;
		plyfile->comments = NULL;
		plyfile->num_comments = 0;
		plyfile->obj_info = NULL;
		plyfile->num_obj_info = 0;
		plyfile->fp = fp;
		plyfile->other_elems = NULL;
		plyfile->rule_list = NULL;

		/* read and parse the file's header */

		words = get_words(plyfile->fp, &nwords, &orig_line);
		if (!words || !equal_strings(words[0], (char*)"ply"))
			return (NULL);

		while (words) {

			/* parse words */

			if (equal_strings(words[0], (char*)"format")) {
				if (nwords != 3)
					return (NULL);
				if (equal_strings(words[1], (char*)"ascii"))
					plyfile->file_type = PLY_ASCII;
				else if (equal_strings(words[1], (char*)"binary_big_endian"))
					plyfile->file_type = PLY_BINARY_BE;
				else if (equal_strings(words[1], (char*)"binary_little_endian"))
					plyfile->file_type = PLY_BINARY_LE;
				else
					return (NULL);
				plyfile->version = atof(words[2]);
				found_format = 1;
			}
			else if (equal_strings(words[0], (char*)"element"))
				add_element(plyfile, words, nwords);
			else if (equal_strings(words[0], (char*)"property"))
				add_property(plyfile, words, nwords);
			else if (equal_strings(words[0], (char*)"comment"))
				add_comment(plyfile, orig_line);
			else if (equal_strings(words[0], (char*)"obj_info"))
				add_obj_info(plyfile, orig_line);
			else if (equal_strings(words[0], (char*)"end_header"))
				break;

			/* free up words space */
			free(words);

			words = get_words(plyfile->fp, &nwords, &orig_line);
		}

		/* create tags for each property of each element, to be used */
		/* later to say whether or not to store each property for the user */

		for (i = 0; i < plyfile->num_elem_types; i++) {
			elem = plyfile->elems[i];
			elem->store_prop = (char *)myalloc(sizeof(char) * elem->nprops);
			for (j = 0; j < elem->nprops; j++)
				elem->store_prop[j] = DONT_STORE_PROP;
			elem->other_offset = NO_OTHER_PROPS; /* no "other" props by default */
		}

		/* set return values about the elements */

		elist = (char **)myalloc(sizeof(char *) * plyfile->num_elem_types);
		for (i = 0; i < plyfile->num_elem_types; i++)
			elist[i] = _strdup(plyfile->elems[i]->name);

		*elem_names = elist;
		*nelems = plyfile->num_elem_types;

		/* return a pointer to the file's information */

		return (plyfile);
	}

	PlyFile *read_ply(FILE *fp)
	{
		PlyFile *ply;
		int num_elems;
		char **elem_names;

		ply = ply_read(fp, &num_elems, &elem_names);

		return (ply);
	}




	using std::map;
	using std::vector;
	using std::string;
	enum status { FILE_NOT_OPEN, FAILURE, SUCCESS };
	struct Vertex
	{
		float x; float y; float z;
		unsigned char r, g, b;
		float s; // measure
	};
	struct Edge
	{
		int v1; int v2;
		unsigned char r, g, b;
		float s; // measure
	};
	struct Face
	{
		unsigned char nvts;
		int verts[3];
		unsigned char r, g, b;
		float s; // measure
	};
	class PLYreader
	{
	public:
		PLYreader() {};
		~PLYreader() {};

		//
		// open the file for reading
		bool open(const char* _fname)
		{
			FILE* fp = fopen(_fname, "r");
			if (!fp)
				return false;
			m_ply = read_ply(fp);
			return true;
		}

		PlyFile *read_ply(FILE *fp)
		{
			PlyFile *ply;
			int num_elems;
			char **elem_names;

			ply = ply_read(fp, &num_elems, &elem_names);

			return (ply);
		}

		char *setup_element_read_ply(PlyFile *ply, int index, int *elem_count)
		{
			PlyElement *elem;

			if (index < 0 || index > ply->num_elem_types) {
				fprintf(stderr, "Warning:  No element with index %d\n", index);
				return (0);
			}

			elem = ply->elems[index];

			/* set this to be the current element */
			ply->which_elem = elem;

			/* return the number of such elements in the file and the element's name */
			*elem_count = elem->num;
			return (elem->name);
		}




		//
		// try to read in a list of expected elements (e.g. "vertex", "edge", "face", etc.)
		// @param _v/e/f_props specifies the format of an element
		// @return list of each type of element, resp. empty if that element is missing.
		template<typename V, typename E, typename F>
		status read(const char* _fname,
			const map<string, PlyProperty>& _v_props,
			const map<string, PlyProperty>& _e_props,
			const map<string, PlyProperty>& _f_props,
			vector<V>& _vts,
			vector<E>& _edges,
			vector<F>& _faces)
		{
			if (!open(_fname))
				return FILE_NOT_OPEN;
			int elem_size = 0;
			// find the element to setup
			char* elem_name = nullptr;
			for (auto i = 0; i < m_ply->num_elem_types; ++i)
			{
				elem_name = setup_element_read_ply(m_ply, i, &elem_size);
				if (equal_strings("vertex", elem_name))
				{
					if (elem_size == 0)
						continue;
					std::cout << "about to read element: " << elem_name
						<< " (" << elem_size << ")" << std::endl;
					// prepare the format of the element to be read
					for (auto j = 0; j < m_ply->elems[i]->nprops; ++j)
					{
						auto prop = m_ply->elems[i]->props[j];
						auto find_it = _v_props.find(prop->name);
						if (find_it != _v_props.end())
						{
							std::cout << "found property: " << prop->name << std::endl;
							setup_property_ply(m_ply, &(find_it->second));
						}
					}

					// read in all elements
					V v;
					_vts.clear();
					_vts.reserve(elem_size);
					for (auto i = 0; i < elem_size; ++i)
					{
						get_element_ply(m_ply, (void *)&v);
						_vts.push_back(v);
					}
				}
				else if (equal_strings("edge", elem_name))
				{
					if (elem_size == 0)
						continue;
					std::cout << "about to read element: " << elem_name
						<< " (" << elem_size << ")" << std::endl;
					// prepare the format of the element to be read
					for (auto j = 0; j < m_ply->elems[i]->nprops; ++j)
					{
						auto prop = m_ply->elems[i]->props[j];
						auto find_it = _e_props.find(prop->name);
						if (find_it != _e_props.end())
						{
							std::cout << "found property: " << prop->name << std::endl;
							setup_property_ply(m_ply, &(find_it->second));
						}
					}
					// read in all elements
					E e;
					_edges.clear();
					_edges.reserve(elem_size);
					for (auto i = 0; i < elem_size; ++i)
					{
						get_element_ply(m_ply, (void *)&e);
						_edges.push_back(e);
					}
				}
				else if (equal_strings("face", elem_name))
				{
					if (elem_size == 0)
						continue;
					std::cout << "about to read element: " << elem_name
						<< " (" << elem_size << ")" << std::endl;
					// prepare the format of the element to be read
					for (auto j = 0; j < m_ply->elems[i]->nprops; ++j)
					{
						auto prop = m_ply->elems[i]->props[j];
						auto find_it = _f_props.find(prop->name);
						if (find_it != _f_props.end())
						{
							std::cout << "found property: " << prop->name << std::endl;
							setup_property_ply(m_ply, &(find_it->second));
						}
						else
						{
							// when we don't want the prop, greg turk's ply reader didn't give internal type
							// a valid value. We make it the same as external type to avoid crashing.
							prop->internal_type = prop->external_type;
						}
					}
					// read in all elements
					F f;
					_faces.clear();
					_faces.reserve(elem_size);
					for (auto i = 0; i < elem_size; ++i)
					{
						get_element_ply(m_ply, (void *)&f);
						_faces.push_back(f);
					}
				}
			}
			//close();
			return SUCCESS;
		}







		/******************************************************************************
		Copy the object information from one PLY file to another.

		Entry:
		out_ply - destination file to copy object information to
		in_ply  - the source of the object information
		******************************************************************************/

		void copy_obj_info_ply(PlyFile *out_ply, PlyFile *in_ply)
		{
			int i;

			for (i = 0; i < in_ply->num_obj_info; i++)
				append_obj_info_ply(out_ply, in_ply->obj_info[i]);
		}

		void close_ply(PlyFile *plyfile)
		{
			fclose(plyfile->fp);
		}

		void free_ply(PlyFile *plyfile)
		{
			/* free up memory associated with the PLY file */
			free(plyfile);
		}
		//
		// closes the reader.
		void close()
		{
			if (m_ply)
			{
				close_ply(m_ply);
				free_ply(m_ply);
			}
		}
	private:
		PlyFile * m_ply;
	};

	class PLYwriter
	{
	public:
		PLYwriter() {}
		~PLYwriter() {}

		PlyFile *ply_write(
			FILE *fp,
			int nelems,
			char **elem_names,
			int file_type
		)
		{
			int i;
			PlyFile *plyfile;
			PlyElement *elem;

			/* check for NULL file pointer */
			if (fp == NULL)
				return (NULL);

			/* create a record for this object */

			plyfile = (PlyFile *)myalloc(sizeof(PlyFile));
			plyfile->file_type = file_type;
			plyfile->num_comments = 0;
			plyfile->num_obj_info = 0;
			plyfile->num_elem_types = nelems;
			plyfile->version = 1.0;
			plyfile->fp = fp;
			plyfile->other_elems = NULL;

			/* tuck aside the names of the elements */

			plyfile->elems = (PlyElement **)myalloc(sizeof(PlyElement *) * nelems);
			for (i = 0; i < nelems; i++) {
				elem = (PlyElement *)myalloc(sizeof(PlyElement));
				plyfile->elems[i] = elem;
				elem->name = _strdup(elem_names[i]);
				elem->num = 0;
				elem->nprops = 0;
			}

			/* return pointer to the file descriptor */
			return (plyfile);
		}

		PlyFile *write_ply(
			FILE *fp,
			int nelems,
			char **elem_names,
			int file_type
		)
		{
			PlyFile *ply;

			ply = ply_write(fp, nelems, elem_names, file_type);

			return (ply);
		}

		//
		// open a file for writing
		bool open(const char * _fname,
			int _n_elems, char** _elem_names, int _mode)
		{
			FILE* fp = fopen(_fname, "w");
			if (!fp)
				return false;
			m_ply = write_ply(fp, _n_elems, _elem_names, _mode);
			return true;
		}

		PlyElement *find_element(PlyFile *plyfile, char *element)
		{
			int i;

			for (i = 0; i < plyfile->num_elem_types; i++)
				if (equal_strings(element, plyfile->elems[i]->name))
					return (plyfile->elems[i]);

			return (NULL);
		}

		PlyProperty *find_property(PlyElement *elem, const char *prop_name, int *index)
		{
			int i;

			for (i = 0; i < elem->nprops; i++)
				if (equal_strings(prop_name, elem->props[i]->name)) {
					*index = i;
					return (elem->props[i]);
				}

			*index = -1;
			return (NULL);
		}

		void copy_property(PlyProperty *dest, const PlyProperty *src)
		{
			dest->name = _strdup(src->name);
			dest->external_type = src->external_type;
			dest->internal_type = src->internal_type;
			dest->offset = src->offset;

			dest->is_list = src->is_list;
			dest->count_external = src->count_external;
			dest->count_internal = src->count_internal;
			dest->count_offset = src->count_offset;
		}

		void describe_property_ply(
			PlyFile *plyfile,
			const PlyProperty *prop
		)
		{
			PlyElement *elem;
			PlyProperty *elem_prop;

			elem = plyfile->which_elem;

			/* create room for new property */

			if (elem->nprops == 0) {
				elem->props = (PlyProperty **)myalloc(sizeof(PlyProperty *));
				elem->store_prop = (char *)myalloc(sizeof(char));
				elem->nprops = 1;
			}
			else {
				elem->nprops++;
				elem->props = (PlyProperty **)
					realloc(elem->props, sizeof(PlyProperty *) * elem->nprops);
				elem->store_prop = (char *)
					realloc(elem->store_prop, sizeof(char) * elem->nprops);
			}

			/* copy the new property */

			elem_prop = (PlyProperty *)myalloc(sizeof(PlyProperty));
			elem->props[elem->nprops - 1] = elem_prop;
			elem->store_prop[elem->nprops - 1] = NAMED_PROP;
			copy_property(elem_prop, prop);
		}

		void describe_element_ply(
			PlyFile *plyfile,
			char *elem_name,
			int nelems
		)
		{
			int i;
			PlyElement *elem;
			PlyProperty *prop;

			/* look for appropriate element */
			elem = find_element(plyfile, elem_name);
			if (elem == NULL) {
				fprintf(stderr, "describe_element_ply: can't find element '%s'\n", elem_name);
				exit(-1);
			}

			elem->num = nelems;

			/* now this element is the current element */
			plyfile->which_elem = elem;
		}

		void write_scalar_type(FILE *fp, int code)
		{
			/* make sure this is a valid code */

			if (code <= StartType || code >= EndType) {
				fprintf(stderr, "write_scalar_type: bad data code = %d\n", code);
				exit(-1);
			}

			/* write the code to a file */

			fprintf(fp, "%s", type_names[code]);
		}

		void header_complete_ply(PlyFile *plyfile)
		{
			int i, j;
			FILE *fp = plyfile->fp;
			PlyElement *elem;
			PlyProperty *prop;

			fprintf(fp, "ply\n");

			switch (plyfile->file_type) {
			case PLY_ASCII:
				fprintf(fp, "format ascii 1.0\n");
				break;
			case PLY_BINARY_BE:
				fprintf(fp, "format binary_big_endian 1.0\n");
				break;
			case PLY_BINARY_LE:
				fprintf(fp, "format binary_little_endian 1.0\n");
				break;
			default:
				fprintf(stderr, "ply_header_complete: bad file type = %d\n",
					plyfile->file_type);
				exit(-1);
			}

			/* write out the comments */

			for (i = 0; i < plyfile->num_comments; i++)
				fprintf(fp, "comment %s\n", plyfile->comments[i]);

			/* write out object information */

			for (i = 0; i < plyfile->num_obj_info; i++)
				fprintf(fp, "obj_info %s\n", plyfile->obj_info[i]);

			/* write out information about each element */

			for (i = 0; i < plyfile->num_elem_types; i++) {

				elem = plyfile->elems[i];
				fprintf(fp, "element %s %d\n", elem->name, elem->num);

				/* write out each property */
				for (j = 0; j < elem->nprops; j++) {
					prop = elem->props[j];
					if (prop->is_list == PLY_LIST) {
						fprintf(fp, "property list ");
						write_scalar_type(fp, prop->count_external);
						fprintf(fp, " ");
						write_scalar_type(fp, prop->external_type);
						fprintf(fp, " %s\n", prop->name);
					}
					else if (prop->is_list == PLY_STRING) {
						fprintf(fp, "property string");
						fprintf(fp, " %s\n", prop->name);
					}
					else {
						fprintf(fp, "property ");
						write_scalar_type(fp, prop->external_type);
						fprintf(fp, " %s\n", prop->name);
					}
				}
			}

			fprintf(fp, "end_header\n");
		}

		void write_ascii_item(
			FILE *fp,
			int int_val,
			unsigned int uint_val,
			double double_val,
			int type
		)
		{
			switch (type) {
			case Int8:
			case Int16:
			case Int32:
				fprintf(fp, "%d ", int_val);
				break;
			case Uint8:
			case Uint16:
			case Uint32:
				fprintf(fp, "%u ", uint_val);
				break;
			case Float32:
			case Float64:
				fprintf(fp, "%g ", double_val);
				break;
			default:
				fprintf(stderr, "write_ascii_item: bad type = %d\n", type);
				exit(-1);
			}
		}

		void get_stored_item(
			void *ptr,
			int type,
			int *int_val,
			unsigned int *uint_val,
			double *double_val
		)
		{
			switch (type) {
			case Int8:
				*int_val = *((char *)ptr);
				*uint_val = *int_val;
				*double_val = *int_val;
				break;
			case Uint8:
				*uint_val = *((unsigned char *)ptr);
				*int_val = *uint_val;
				*double_val = *uint_val;
				break;
			case Int16:
				*int_val = *((short int *)ptr);
				*uint_val = *int_val;
				*double_val = *int_val;
				break;
			case Uint16:
				*uint_val = *((unsigned short int *) ptr);
				*int_val = *uint_val;
				*double_val = *uint_val;
				break;
			case Int32:
				*int_val = *((int *)ptr);
				*uint_val = *int_val;
				*double_val = *int_val;
				break;
			case Uint32:
				*uint_val = *((unsigned int *)ptr);
				*int_val = *uint_val;
				*double_val = *uint_val;
				break;
			case Float32:
				*double_val = *((float *)ptr);
				*int_val = *double_val;
				*uint_val = *double_val;
				break;
			case Float64:
				*double_val = *((double *)ptr);
				*int_val = *double_val;
				*uint_val = *double_val;
				break;
			default:
				fprintf(stderr, "get_stored_item: bad type = %d\n", type);
				exit(-1);
			}
		}

		void write_binary_item(
			FILE *fp,
			int int_val,
			unsigned int uint_val,
			double double_val,
			int type
		)
		{
			unsigned char uchar_val;
			char char_val;
			unsigned short ushort_val;
			short short_val;
			float float_val;

			switch (type) {
			case Int8:
				char_val = int_val;
				fwrite(&char_val, 1, 1, fp);
				break;
			case Int16:
				short_val = int_val;
				fwrite(&short_val, 2, 1, fp);
				break;
			case Int32:
				fwrite(&int_val, 4, 1, fp);
				break;
			case Uint8:
				uchar_val = uint_val;
				fwrite(&uchar_val, 1, 1, fp);
				break;
			case Uint16:
				ushort_val = uint_val;
				fwrite(&ushort_val, 2, 1, fp);
				break;
			case Uint32:
				fwrite(&uint_val, 4, 1, fp);
				break;
			case Float32:
				float_val = double_val;
				fwrite(&float_val, 4, 1, fp);
				break;
			case Float64:
				fwrite(&double_val, 8, 1, fp);
				break;
			default:
				fprintf(stderr, "write_binary_item: bad type = %d\n", type);
				exit(-1);
			}
		}

		void put_element_ply(PlyFile *plyfile, void *elem_ptr)
		{
			int i, j, k;
			FILE *fp = plyfile->fp;
			PlyElement *elem;
			PlyProperty *prop;
			char *item;
			char *elem_data;
			char **item_ptr;
			int list_count;
			int item_size;
			int int_val;
			unsigned int uint_val;
			double double_val;
			char **other_ptr;

			elem = plyfile->which_elem;
			elem_data = (char *)elem_ptr;
			other_ptr = (char **)(((char *)elem_ptr) + elem->other_offset);

			/* write out either to an ascii or binary file */

			if (plyfile->file_type == PLY_ASCII) {

				/* write an ascii file */

				/* write out each property of the element */
				for (j = 0; j < elem->nprops; j++) {

					prop = elem->props[j];

					if (elem->store_prop[j] == OTHER_PROP)
						elem_data = *other_ptr;
					else
						elem_data = (char *)elem_ptr;

					if (prop->is_list == PLY_LIST) {  /* list */
						item = elem_data + prop->count_offset;
						get_stored_item((void *)item, prop->count_internal,
							&int_val, &uint_val, &double_val);
						write_ascii_item(fp, int_val, uint_val, double_val,
							prop->count_external);
						list_count = uint_val;
						/*item_ptr = (char **)( elem_data + prop->offset );
						item = item_ptr[ 0 ];*/
						item = elem_data + prop->offset;
						item_size = ply_type_size[prop->internal_type];
						for (k = 0; k < list_count; k++) {
							get_stored_item((void *)item, prop->internal_type,
								&int_val, &uint_val, &double_val);
							write_ascii_item(fp, int_val, uint_val, double_val,
								prop->external_type);
							item += item_size;
						}
					}
					else if (prop->is_list == PLY_STRING) {  /* string */
						char **str;
						item = elem_data + prop->offset;
						str = (char **)item;
						fprintf(fp, "\"%s\"", *str);
					}
					else {                                  /* scalar */
						item = elem_data + prop->offset;
						get_stored_item((void *)item, prop->internal_type,
							&int_val, &uint_val, &double_val);
						write_ascii_item(fp, int_val, uint_val, double_val,
							prop->external_type);
					}
				}

				fprintf(fp, "\n");
			}
			else {

				/* write a binary file */

				/* write out each property of the element */
				for (j = 0; j < elem->nprops; j++) {
					prop = elem->props[j];
					if (elem->store_prop[j] == OTHER_PROP)
						elem_data = *other_ptr;
					else
						elem_data = (char *)elem_ptr;
					if (prop->is_list == PLY_LIST) {   /* list */
						item = elem_data + prop->count_offset;
						item_size = ply_type_size[prop->count_internal];
						get_stored_item((void *)item, prop->count_internal,
							&int_val, &uint_val, &double_val);
						write_binary_item(fp, int_val, uint_val, double_val,
							prop->count_external);
						list_count = uint_val;
						/*item_ptr = (char **)( elem_data + prop->offset );
						item = item_ptr[ 0 ];*/
						item = elem_data + prop->offset;
						item_size = ply_type_size[prop->internal_type];
						for (k = 0; k < list_count; k++) {
							get_stored_item((void *)item, prop->internal_type,
								&int_val, &uint_val, &double_val);
							write_binary_item(fp, int_val, uint_val, double_val,
								prop->external_type);
							item += item_size;
						}
					}
					else if (prop->is_list == PLY_STRING) {   /* string */
						int len;
						char **str;
						item = elem_data + prop->offset;
						str = (char **)item;

						/* write the length */
						len = strlen(*str) + 1;
						fwrite(&len, sizeof(int), 1, fp);

						/* write the string, including the null character */
						fwrite(*str, len, 1, fp);
					}
					else {                   /* scalar */
						item = elem_data + prop->offset;
						item_size = ply_type_size[prop->internal_type];
						get_stored_item((void *)item, prop->internal_type,
							&int_val, &uint_val, &double_val);
						write_binary_item(fp, int_val, uint_val, double_val,
							prop->external_type);
					}
				}

			}
		}

		void put_element_setup_ply(PlyFile *plyfile, char *elem_name)
		{
			PlyElement *elem;

			elem = find_element(plyfile, elem_name);
			if (elem == NULL) {
				fprintf(stderr, "put_element_setup_ply: can't find element '%s'\n", elem_name);
				exit(-1);
			}

			plyfile->which_elem = elem;
		}

		//
		// write a list of elements to file.
		// @param _export_v/e/f if v/e/f will be written
		// @param _v/e/f_prop specifies format for v/e/f elements.
		//        provide dummy if corresponding elements will not be written.
		// @param _vts/edges/faces_to_write contains the elements to write. 
		//        provide dummy if corresponding elements will not be written.
		template<typename V, typename E, typename F>
		status write(const char* _fname,
			bool _export_v,
			bool _export_e,
			bool _export_f,
			const map<string, PlyProperty>& _v_prop,
			const map<string, PlyProperty>& _e_prop,
			const map<string, PlyProperty>& _f_prop,
			const vector<V>& _vts_to_write,
			const vector<E>& _edges_to_write,
			const vector<F>& _faces_to_write)
		{
			char* names[] = {
				_export_v ? "vertex" : nullptr,
				_export_e ? "edge" : nullptr,
				_export_f ? "face" : nullptr
			};
			int cnt = _export_v + _export_e + _export_f;

			if (!open(_fname, cnt, names, PLY_ASCII))
				return FILE_NOT_OPEN;

			const map<string, PlyProperty>* props[] = { &_v_prop, &_e_prop, &_f_prop };
			const int sizes[] = { _vts_to_write.size(), _edges_to_write.size(), _faces_to_write.size() };
			// get header ready
			for (auto i = 0; i < 3; ++i)
			{
				if (!names[i])
					continue;
				describe_element_ply(m_ply, names[i], sizes[i]);
				for (auto it = props[i]->begin(); it != props[i]->end(); ++it)
				{
					describe_property_ply(m_ply, &it->second);
				}
			}
			header_complete_ply(m_ply);
			// write data
			if (_export_v)
			{
				put_element_setup_ply(m_ply, names[0]);
				for (auto j = 0; j < _vts_to_write.size(); ++j)
					put_element_ply(m_ply, (void*)&_vts_to_write[j]);
			}
			if (_export_e)
			{
				put_element_setup_ply(m_ply, names[1]);
				for (auto j = 0; j < _edges_to_write.size(); ++j)
					put_element_ply(m_ply, (void*)&_edges_to_write[j]);
			}
			if (_export_f)
			{
				put_element_setup_ply(m_ply, names[2]);
				for (auto j = 0; j < _faces_to_write.size(); ++j)
					put_element_ply(m_ply, (void*)&_faces_to_write[j]);
			}

			close();
			return SUCCESS;
		}
		// 
		// simply write vts, edges, and tri angle faces to a file
		/*status write(
		const char* _fname,
		const vector<ply::Vertex>& output_vts,
		const vector<ply::Edge>& output_edges,
		const vector<ply::Face>& output_faces
		);*/
		/**
		status PLYwriter::write(
		const char* _fname,
		const vector<ply::Vertex>& output_vts,
		const vector<ply::Edge>& output_edges,
		const vector<ply::Face>& output_faces
		)
		{
		std::map<std::string, PlyProperty> vert_props;
		vert_props["x"] = { (char*)"x", Float32, Float32, offsetof(Vertex, x), PLY_SCALAR, 0, 0, 0 };
		vert_props["y"] = { (char*)"y", Float32, Float32, offsetof(Vertex, y), PLY_SCALAR, 0, 0, 0 };
		vert_props["z"] = { (char*)"z", Float32, Float32, offsetof(Vertex, z), PLY_SCALAR, 0, 0, 0 };
		std::map<std::string, PlyProperty> edge_props;
		edge_props["vertex1"] = { (char*)"vertex1", Int32, Int32, offsetof(Edge, v1), PLY_SCALAR, 0, 0, 0 };
		edge_props["vertex2"] = { (char*)"vertex2", Int32, Int32, offsetof(Edge, v2), PLY_SCALAR, 0, 0, 0 };
		std::map<std::string, PlyProperty> face_props;
		face_props["vertex_indices"] = {
		(char*)"vertex_indices", Int32, Int32, offsetof(Face, verts),
		PLY_LIST, Uint8, Uint8, offsetof(Face,nvts) };

		std::cout << "writing to ply file... " << std::endl;
		std::cout << "# vts/edges/faces: "
		<< output_vts.size() << "/"
		<< output_edges.size() << "/"
		<< output_faces.size() << std::endl;

		return write(_fname, true, true, true,
		vert_props, edge_props, face_props,
		output_vts, output_edges, output_faces);
		}**/

		void close_ply(PlyFile *plyfile)
		{
			fclose(plyfile->fp);
		}


		/******************************************************************************
		Free the memory used by a PLY file.

		Entry:
		plyfile - identifier of file
		******************************************************************************/

		void free_ply(PlyFile *plyfile)
		{
			/* free up memory associated with the PLY file */
			free(plyfile);
		}


		//
		// closes the writer.
		void close()
		{
			if (m_ply)
			{
				close_ply(m_ply);
				free_ply(m_ply);
			}
		}
	private:
		PlyFile * m_ply;
	};
}
//
//namespace ply
//{

/*ply::PLYreader::PLYreader()
{
}

ply::PLYreader::~PLYreader()
{
}*/

/*bool ply::PLYreader::open( const char * _fname )
{
FILE* fp = fopen( _fname, "r" );
if ( !fp )
return false;
m_ply = read_ply( fp );
return true;
}*/

/*void ply::PLYreader::close()
{
if ( m_ply )
{
close_ply( m_ply );
free_ply( m_ply );
}
}*/

//template<typename V, typename E, typename F>
//ply::status ply::PLYreader::read( const char* _fname,
//	const map<string, PlyProperty>& _v_props,
//	const map<string, PlyProperty>& _e_props,
//	const map<string, PlyProperty>& _f_props,
//	vector<V>& _vts,
//	vector<E>& _edges,
//	vector<F>& _faces )
//{
//	if ( !open( _fname ) )
//		return FILE_NOT_OPEN;
//	int elem_size = 0;
//	// find the element to setup
//	char* elem_name = nullptr;
//	for ( auto i = 0; i < m_ply->num_elem_types; ++i )
//	{
//		elem_name = setup_element_read_ply( m_ply, i, &elem_size );
//		if ( equal_strings( "vertex", elem_name ) )
//		{
//			if ( elem_size == 0 )
//				continue;
//			std::cout << "about to read element: " << elem_name
//				<< " (" << elem_size << ")" << std::endl;
//			// prepare the format of the element to be read
//			for ( auto j = 0; j < m_ply->elems[ i ]->nprops; ++j )
//			{
//				auto prop = m_ply->elems[ i ]->props[ j ];
//				auto find_it = _v_props.find( prop->name );
//				if ( find_it != _v_props.end() )
//				{
//					std::cout << "found property: " << prop->name << std::endl;
//					setup_property_ply( m_ply, &( find_it->second ) );
//				}
//			}
//			// read in all elements
//			V v;
//			_vts.clear();
//			_vts.reserve( elem_size );
//			for ( auto i = 0; i < elem_size; ++i )
//			{
//				get_element_ply( m_ply, (void *)&v );
//				_vts.push_back( v );
//			}
//		}
//		else if ( equal_strings( "edge", elem_name ) )
//		{
//			if ( elem_size == 0 )
//				continue;
//			std::cout << "about to read element: " << elem_name
//				<< " (" << elem_size << ")" << std::endl;
//			// prepare the format of the element to be read
//			for ( auto j = 0; j < m_ply->elems[ i ]->nprops; ++j )
//			{
//				auto prop = m_ply->elems[ i ]->props[ j ];
//				auto find_it = _e_props.find( prop->name );
//				if ( find_it != _e_props.end() )
//				{
//					std::cout << "found property: " << prop->name << std::endl;
//					setup_property_ply( m_ply, &( find_it->second ) );
//				}
//			}
//			// read in all elements
//			E e;
//			_edges.clear();
//			_edges.reserve( elem_size );
//			for ( auto i = 0; i < elem_size; ++i )
//			{
//				get_element_ply( m_ply, (void *)&e );
//				_edges.push_back( e );
//			}
//		}
//		else if ( equal_strings( "face", elem_name ) )
//		{
//			if ( elem_size == 0 )
//				continue;
//			std::cout << "about to read element: " << elem_name
//				<< " (" << elem_size << ")" << std::endl;
//			// prepare the format of the element to be read
//			for ( auto j = 0; j < m_ply->elems[ i ]->nprops; ++j )
//			{
//				auto prop = m_ply->elems[ i ]->props[ j ];
//				auto find_it = _f_props.find( prop->name );
//				if ( find_it != _f_props.end() )
//				{
//					std::cout << "found property: " << prop->name << std::endl;
//					setup_property_ply( m_ply, &( find_it->second ) );
//				}
//			}
//			// read in all elements
//			F f;
//			_faces.clear();
//			_faces.reserve( elem_size );
//			for ( auto i = 0; i < elem_size; ++i )
//			{
//				get_element_ply( m_ply, (void *)&f );
//				_faces.push_back( f );
//			}
//		}
//	}
//	close();
//	return SUCCESS;
//}

/*ply::PLYwriter::PLYwriter()
{
}

ply::PLYwriter::~PLYwriter()
{
}*/

/*bool ply::PLYwriter::open( const char * _fname,
int _n_elems, char** _elem_names, int _mode )
{
FILE* fp = fopen( _fname, "w" );
if ( !fp )
return false;
m_ply = write_ply( fp, _n_elems, _elem_names, _mode );
return true;
}*/

/*void ply::PLYwriter::close()
{
if ( m_ply )
{
close_ply( m_ply );
free_ply( m_ply );
}
}*/

/*template<typename V, typename E, typename F>
ply::status ply::PLYwriter::write( const char* _fname,
bool _export_v,
bool _export_e,
bool _export_f,
const map<string, PlyProperty>& _v_prop,
const map<string, PlyProperty>& _e_prop,
const map<string, PlyProperty>& _f_prop,
const vector<V>& _vts_to_write,
const vector<E>& _edges_to_write,
const vector<F>& _faces_to_write )
{
char* names[] = {
_export_v ? "vertex" : nullptr,
_export_e ? "edge" : nullptr,
_export_f ? "face" : nullptr
};
int cnt = _export_v + _export_e + _export_f;

if ( !open( _fname, cnt, names, PLY_ASCII ) )
return FILE_NOT_OPEN;

const map<string, PlyProperty>* props[] = { &_v_prop, &_e_prop, &_f_prop };
const int sizes[] = { _vts_to_write.size(), _edges_to_write.size(), _faces_to_write.size() };
// get header ready
for ( auto i = 0; i < 3; ++i )
{
if ( !names[ i ] )
continue;
describe_element_ply( m_ply, names[ i ], sizes[ i ] );
for ( auto it = props[ i ]->begin(); it != props[ i ]->end(); ++it )
{
describe_property_ply( m_ply, &it->second );
}
}
header_complete_ply( m_ply );
// write data
if ( _export_v )
{
put_element_setup_ply( m_ply, names[ 0 ] );
for ( auto j = 0; j < _vts_to_write.size(); ++j )
put_element_ply( m_ply, (void*)&_vts_to_write[ j ] );
}
if ( _export_e )
{
put_element_setup_ply( m_ply, names[ 1 ] );
for ( auto j = 0; j < _edges_to_write.size(); ++j )
put_element_ply( m_ply, (void*)&_edges_to_write[ j ] );
}
if ( _export_f )
{
put_element_setup_ply( m_ply, names[ 2 ] );
for ( auto j = 0; j < _faces_to_write.size(); ++j )
put_element_ply( m_ply, (void*)&_faces_to_write[ j ] );
}

close();
return SUCCESS;
}*/



//}