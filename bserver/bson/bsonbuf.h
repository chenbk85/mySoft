
#define BSON_NAME_LEN	128

struct typeinfo {
	int id;
	char name[BSON_NAME_LEN];
};

static struct typeinfo def_tis[] = {
	{ TI_INT8,  "int8" },
	{ TI_INT16, "int16" },
	{ TI_INT32, "int32" },
	{ TI_INT64, "int64" },
};

union podval {
	int8_t i8;
	int16_t i16;
	int32_t i32;
	int64_t i64;
	float f32;
	double f64;
	const char* str;
};

enum array_type {
	ARR_NO = 0,
	ARR_FIXED,
	ARR_VAR,
	ARR_DYN,
	ARR_BUTT,
};

struct fieldinfo {
	int id;
	const struct typeinfo *tinfo;
	size_t offset;
	bool optional;

	union podval def_val;
	char name[BSON_NAME_LEN];

	char xtag[BSON_NAME_LEN];
	char jtab[BSON_NAME_LEN];

	enum array_type arr_type;
	const struct typeinfo* arr_tinfo;
	int arr_size;
	bool arr_xml_child;
	char arr_xml_ptag[BSON_NAME_LEN];
};

// all field must be POD
struct structinfo {
	char name[BSON_NAME_LEN];
	size_t count;
	struct fieldinfo fis[0];
};

struct messageinfo {
	char name[BSON_NAME_LEN];
	size_t count;
	struct fieldinfo fis[0];
};




