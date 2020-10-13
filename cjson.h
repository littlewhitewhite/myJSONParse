#if 1
typedef struct cJSON {
	struct cJSON *next, *prev;
	struct cJSON *child;

	int type;

	char *valueString;
	int valueInt;
	double valueDouble;
	
	char *string;
}cJSON;

typedef enum
{
	cJSON_False, cJSON_True, cJSON_NULL, cJSON_Number, cJSON_String, cJSON_Array, cJSON_Object
}NODETYPE;


cJSON *cJSON_Parse(const char *value);
cJSON *cJSON_ParseWithOpts(const char *value, const char **s, int require_null_terminated);


#endif