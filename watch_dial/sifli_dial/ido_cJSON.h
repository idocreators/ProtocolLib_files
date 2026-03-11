/*
  Copyright (c) 2009 Dave Gamble
 
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
 
  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.
 
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#ifndef cJSON__h
#define cJSON__h

#ifdef __cplusplus
extern "C"
{
#endif

/* ido_cJSON Types: */
#define ido_cJSON_False 0
#define ido_cJSON_True 1
#define ido_cJSON_NULL 2
#define ido_cJSON_Number 3
#define ido_cJSON_String 4
#define ido_cJSON_Array 5
#define ido_cJSON_Object 6
	
#define ido_cJSON_IsReference 256
#define ido_cJSON_StringIsConst 512

/* The ido_cJSON structure: */
typedef struct ido_cJSON {
	struct ido_cJSON *next,*prev;	/* next/prev allow you to walk array/object chains. Alternatively, use GetArraySize/GetArrayItem/GetObjectItem */
	struct ido_cJSON *child;		/* An array or object item will have a child pointer pointing to a chain of the items in the array/object. */

	int type;					/* The type of the item, as above. */

	char *valuestring;			/* The item's string, if type==ido_cJSON_String */
	int valueint;				/* The item's number, if type==ido_cJSON_Number */
	double valuedouble;			/* The item's number, if type==ido_cJSON_Number */

	char *string;				/* The item's name string, if this item is the child of, or is in the list of subitems of an object. */
} ido_cJSON;

typedef struct ido_cJSON_Hooks {
      void *(*malloc_fn)(size_t sz);
      void (*free_fn)(void *ptr);
} ido_cJSON_Hooks;

/* Supply malloc, realloc and free functions to ido_cJSON */
extern void ido_cJSON_InitHooks(ido_cJSON_Hooks* hooks);


/* Supply a block of JSON, and this returns a ido_cJSON object you can interrogate. Call ido_cJSON_Delete when finished. */
extern ido_cJSON *ido_cJSON_Parse(const char *value);
/* Render a ido_cJSON entity to text for transfer/storage. Free the char* when finished. */
extern char  *ido_cJSON_Print(ido_cJSON *item);
/* Render a ido_cJSON entity to text for transfer/storage without any formatting. Free the char* when finished. */
extern char  *ido_cJSON_PrintUnformatted(ido_cJSON *item);
/* Render a ido_cJSON entity to text using a buffered strategy. prebuffer is a guess at the final size. guessing well reduces reallocation. fmt=0 gives unformatted, =1 gives formatted */
extern char *ido_cJSON_PrintBuffered(ido_cJSON *item,int prebuffer,int fmt);
/* Delete a ido_cJSON entity and all subentities. */
extern void   ido_cJSON_Delete(ido_cJSON *c);

/* Returns the number of items in an array (or object). */
extern int	  ido_cJSON_GetArraySize(ido_cJSON *array);
/* Retrieve item number "item" from array "array". Returns NULL if unsuccessful. */
extern ido_cJSON *ido_cJSON_GetArrayItem(ido_cJSON *array,int item);
/* Get item "string" from object. Case insensitive. */
extern ido_cJSON *ido_cJSON_GetObjectItem(ido_cJSON *object,const char *string);

/* For analysing failed parses. This returns a pointer to the parse error. You'll probably need to look a few chars back to make sense of it. Defined when ido_cJSON_Parse() returns 0. 0 when ido_cJSON_Parse() succeeds. */
extern const char *ido_cJSON_GetErrorPtr(void);
	
/* These calls create a ido_cJSON item of the appropriate type. */
extern ido_cJSON *ido_cJSON_CreateNull(void);
extern ido_cJSON *ido_cJSON_CreateTrue(void);
extern ido_cJSON *ido_cJSON_CreateFalse(void);
extern ido_cJSON *ido_cJSON_CreateBool(int b);
extern ido_cJSON *ido_cJSON_CreateNumber(double num);
extern ido_cJSON *ido_cJSON_CreateString(const char *string);
extern ido_cJSON *ido_cJSON_CreateArray(void);
extern ido_cJSON *ido_cJSON_CreateObject(void);

/* These utilities create an Array of count items. */
extern ido_cJSON *ido_cJSON_CreateIntArray(const int *numbers,int count);
extern ido_cJSON *ido_cJSON_CreateFloatArray(const float *numbers,int count);
extern ido_cJSON *ido_cJSON_CreateDoubleArray(const double *numbers,int count);
extern ido_cJSON *ido_cJSON_CreateStringArray(const char **strings,int count);

/* Append item to the specified array/object. */
extern void ido_cJSON_AddItemToArray(ido_cJSON *array, ido_cJSON *item);
extern void	ido_cJSON_AddItemToObject(ido_cJSON *object,const char *string,ido_cJSON *item);
extern void	ido_cJSON_AddItemToObjectCS(ido_cJSON *object,const char *string,ido_cJSON *item);	/* Use this when string is definitely const (i.e. a literal, or as good as), and will definitely survive the ido_cJSON object */
/* Append reference to item to the specified array/object. Use this when you want to add an existing ido_cJSON to a new ido_cJSON, but don't want to corrupt your existing ido_cJSON. */
extern void ido_cJSON_AddItemReferenceToArray(ido_cJSON *array, ido_cJSON *item);
extern void	ido_cJSON_AddItemReferenceToObject(ido_cJSON *object,const char *string,ido_cJSON *item);

/* Remove/Detatch items from Arrays/Objects. */
extern ido_cJSON *ido_cJSON_DetachItemFromArray(ido_cJSON *array,int which);
extern void   ido_cJSON_DeleteItemFromArray(ido_cJSON *array,int which);
extern ido_cJSON *ido_cJSON_DetachItemFromObject(ido_cJSON *object,const char *string);
extern void   ido_cJSON_DeleteItemFromObject(ido_cJSON *object,const char *string);
	
/* Update array items. */
extern void ido_cJSON_InsertItemInArray(ido_cJSON *array,int which,ido_cJSON *newitem);	/* Shifts pre-existing items to the right. */
extern void ido_cJSON_ReplaceItemInArray(ido_cJSON *array,int which,ido_cJSON *newitem);
extern void ido_cJSON_ReplaceItemInObject(ido_cJSON *object,const char *string,ido_cJSON *newitem);

/* Duplicate a ido_cJSON item */
extern ido_cJSON *ido_cJSON_Duplicate(ido_cJSON *item,int recurse);
/* Duplicate will create a new, identical ido_cJSON item to the one you pass, in new memory that will
need to be released. With recurse!=0, it will duplicate any children connected to the item.
The item->next and ->prev pointers are always zero on return from Duplicate. */

/* ParseWithOpts allows you to require (and check) that the JSON is null terminated, and to retrieve the pointer to the final byte parsed. */
extern ido_cJSON *ido_cJSON_ParseWithOpts(const char *value,const char **return_parse_end,int require_null_terminated);

extern void ido_cJSON_Minify(char *json);

/* Macros for creating things quickly. */
#define ido_cJSON_AddNullToObject(object,name)		ido_cJSON_AddItemToObject(object, name, ido_cJSON_CreateNull())
#define ido_cJSON_AddTrueToObject(object,name)		ido_cJSON_AddItemToObject(object, name, ido_cJSON_CreateTrue())
#define ido_cJSON_AddFalseToObject(object,name)		ido_cJSON_AddItemToObject(object, name, ido_cJSON_CreateFalse())
#define ido_cJSON_AddBoolToObject(object,name,b)	ido_cJSON_AddItemToObject(object, name, ido_cJSON_CreateBool(b))
#define ido_cJSON_AddNumberToObject(object,name,n)	ido_cJSON_AddItemToObject(object, name, ido_cJSON_CreateNumber(n))
#define ido_cJSON_AddStringToObject(object,name,s)	ido_cJSON_AddItemToObject(object, name, ido_cJSON_CreateString(s))

/* When assigning an integer value, it needs to be propagated to valuedouble too. */
#define ido_cJSON_SetIntValue(object,val)			((object)?(object)->valueint=(object)->valuedouble=(val):(val))
#define ido_cJSON_SetNumberValue(object,val)		((object)?(object)->valueint=(object)->valuedouble=(val):(val))

#ifdef __cplusplus
}
#endif

#endif
