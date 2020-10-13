#if 1
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "cJson.h"

#define cJSON_malloc malloc


static const char *ep;
const char *cJSON_GetErrorPtr() { return ep; }

/* Predeclare these prototypes. */
static const char *skip(const char *in);
cJSON *cJSON_New_Item();
static const char *parse_number(cJSON *item, const char *num);
static const char *parse_string(cJSON *item ,const char *str);


static const char *parse_value(cJSON *item, const char *value);
//static char *print_value(cJSON *item, int depth, int fmt, printbuffer *p);
static const char *parse_array(cJSON *item, const char *value);
//static char *print_array(cJSON *item, int depth, int fmt, printbuffer *p);
static const char *parse_object(cJSON *item, const char *value);
//static char *print_object(cJSON *item, int depth, int fmt, printbuffer *p);





cJSON *cJSON_Parse(const char *value)
{
	return cJSON_ParseWithOpts(value, 0, 0);
}

cJSON *cJSON_ParseWithOpts(const char *value, const char **s, int require_null_terminated)
{
	const char *end = 0;
	cJSON *c = cJSON_New_Item();
	ep = 0;
	if(!c) return 0;

	end = parse_value(c, skip(value));
}

/* Parse core  */
static const char *parse_value(cJSON *item, const char *value)
{
	if(!value) { return 0; }
	if(!strncmp(value, "null", 4)) { item->type = cJSON_NULL; return value + 4;}
	if(!strncmp(value, "false", 5)) { item->type = cJSON_False; return value + 5; }
	if(!strncmp(value, "true", 4)) { item->type = cJSON_True; return value + 4; }
	if(*value == '\"') {return parse_string(item, value);}
	if(*value == '-' || (*value >= '0' && *value <= '9')) {return parse_number(item, value);} 
	if(*value == '{') { return parse_object(item, value); }
	if(*value == '[') { return parse_array(item, value); }

	ep = value; return 0;
}

/* Utility to jump whitespace and CR/LF */
static const char *skip(const char *in)
{
	while(in && (unsigned char)*in <= 32)	
		in++;
	return in;
}


cJSON *cJSON_New_Item()
{
	cJSON *node = (cJSON *)cJSON_malloc(sizeof(cJSON));
	if(node)
		memset(node, 0, sizeof(cJSON));
	return node;
}
//static void * (*cJSON_malloc)(size_t sz) = malloc;

/* �Ƚ���key���ٽ���value������value������ִ�ж�Ӧ�Ľ������ַ��������֣� ���飬����ȣ���
������value������������ţ���ô������������Ϊ�������û�н����꣬������һ�
��ݹ���� */
static const char *parse_object(cJSON *item, const char *value)
{
	cJSON *child = 0;
	if(*value != '{') { ep = value; return 0; }
	item->type = cJSON_Object;
	value = skip(value + 1);	
	if(*value == '}') return value + 1;		//empty object

	item->child = child = cJSON_New_Item();
	if(child == 0) return 0;
	value = skip(parse_string(child, value));		// ʵ������parse key��value��key
	if(!value) return 0;
	child->string = child->valueString;				//  ��Ϊ��parse_object�������������޸�
	child->valueString = NULL;
	if(*value != ':') { ep = value; return 0; }
	value = skip(parse_value(child, skip(value + 1)));	//ʵ������parse key��value��value
	if(!value)
		return 0;	
	// �����������һ���Ƕ���','��˵���������û�н����꣬����һ��ö��ŷָ�����
	while(*value ==',') {
		cJSON *new_item;
		if(!(new_item = cJSON_New_Item()))	return 0;
		child->next = new_item;
		new_item->prev = child;
		child = new_item;
		value = skip(parse_string(child, skip(value+1)));	// ��һ�����ǽ�����key-value�е�key
		if(!value) return 0;
		child->string = child->valueString;
		child->valueString = NULL;
		if(*value != ':') {
			ep = value;
			return 0;
		}
		value = skip(parse_value(child, skip(value + 1)));
	}
	if(*value == '}') return value + 1; 
	ep = value; return 0;	/* malformed ���ε�*/
}


/* parse_string: ����key-value��key�����õ��� ��Ϊkey�������һ���ַ�����
ͬʱvalue�Ŀ�ͷ�����'\"'�ַ��Ļ� ����Ҳ���õ� */
static const char *parse_string(cJSON *item ,const char *str)
{
	const char *ptr = str + 1;
	char *ptr2;
	char *out;
	int len = 0;

	if(*str != '\"') {
		ep = str;
		return 0;
	}

	while(*ptr != '\"' && *ptr && ++len)
		if(*ptr++ == '\\')
			ptr++;
	
	out = (char *)cJSON_malloc(len + 1);
	if(!out)
		return 0;
	
	ptr = str + 1; 	ptr2 = out;
	while(*ptr != '\"' && *ptr) {
		if(*ptr != '\\') {
			*ptr2++ = *ptr++;
		} else {
			ptr++;
			switch(*ptr) {
				default:
				*ptr2++ = *ptr;
				break;
			}
		ptr++;
		}

	}

	*ptr2 = '\0';
	if(*ptr == '\"')
		ptr++;
	item->valueString = out;	
	item->type = cJSON_String;
	return ptr;
}

/* parse a string of double into double */
static const char *parse_number(cJSON *item, const char *num)
{
	double n = 0, sign = 1, factor = 10;	// default factor = 10 , ���磺1.5e+3	
	if (num == '-') {
		sign = -1;
		num++;
	}
	while (num >= '1' && num <= '9') {
		n = n * 10.0 + *num++ - '0';
	}
	
	if (*num == '.')
		num++;
	while (*num >= '0' && *num <= '9') { n += (*num++ * 0.1); }
	if (*num == 'e' || *num == 'E') {
		num++;
		if (*num == '+') num++;
		else if (*num == '-') {
			factor = 0.1;
			num++;
		}
		while (*num >= '0' && *num <= '9') {
			n += *num++ * factor;
		}
		n *= sign;
		item->valueDouble = n;
		item->valueInt = (int)n;
		item->type = cJSON_Number;
	}
	return num;
}


const char * parse_array(cJSON *item, const char *value)
{
	cJSON *child;
	if (*value != '[') {
		ep = value;
		return 0;
	}

	item->type = cJSON_Array;
	value = skip(value + 1);
	if (*value == ']')
		return value + 1;	// empty array

	item->child = child = cJSON_New_Item();		// ��Ҫ���ڵݹ�ʱ��̬�����½ڵ�
	if(!child)
		return 0;
	value = skip(parse_value(child, value));

	while (*value == ',') {
		cJSON *new_item;
		if (!(new_item = cJSON_New_Item()))	return 0;
		value = skip(parse_value(new_item, skip(value + 1)));
	}
	if (*value == ']')
		return value + 1;
	ep = value;
	return 0;
}
#endif
