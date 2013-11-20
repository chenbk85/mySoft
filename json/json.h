#include <string>
#include <list>
#include <vector>
#include <iostream>
#include <sstream>

class json_object;
class json_pair;
class json_value;

class json_object {
public:
	json_object() {}
	json_object(const struct json_object& x) : prs_(x.prs_) {}
private:
	std::list<struct json_pair> prs_;
};

typedef enum {
	JVT_NULL = 0,
	JVT_TRUE,
	JVT_FALSE,
	JVT_INTEGER,
	JVT_FLOAT,
	JVT_STRING,
	JVT_OBJECT,
	JVT_ARRAY,
	JVT_BUTT
} json_value_type;

class json_value {
public:
	json_value() : type_(JVT_NULL) {}
	json_value(bool b) : type_(b ? JVT_TRUE : JVT_FALSE) {}
	json_value(long i) : type_(JVT_INTEGER) { un_.int_ = i; }
	json_value(double i) : type_(JVT_FLOAT) { un_.float_ = i; }
	json_value(const std::string& i) : type_(JVT_STRING) { un_.str_ = new std::string(i); }
	json_value(const struct json_object& obj) : type_(JVT_OBJECT) { un_.obj_ = new json_object(obj); }
	json_value(const std::vector<struct json_value>& arr);
	json_value(const json_value& x);

	~json_value();
private:
	json_value_type type_;
	union {
		long int_;
		double float_;
		std::string *str_;
		struct json_object *obj_;
		std::vector<struct json_value> *arr_;
	} un_;

};

class json_pair {
public:
	json_pair(const std::string& nam, const struct json_value& val) : nam_(nam), val_(val) {}
private:
	std::string nam_;
	struct json_value val_;
};

std::ostream& operator<<(std::ostream& out, const struct json_object& obj);
std::ostream& operator<<(std::ostream& out, const struct json_value& val);
std::ostream& operator<<(std::ostream& out, const std::vector<struct json_value>& arr);

std::istream& operator>>(std::istream& in, struct json_object obj);

int str2json(const char* s, json_object& obj);
int str2json(const std::string& s, json_object& obj);

int json2str(const json_object& obj, char *s, size_t size);
int json2str(const json_object& obj, std::string& s);
