#include <string>
#include <list>
#include <vector>
#include <iostream>
#include <sstream>

struct json_pair;

struct json_object {
	json_object() {}
	json_object(const struct json_object& x) : prs_(x.prs_) {}
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

struct json_value {
	json_value_type type_;
	union {
		long int_;
		double float_;
		std::string *str_;
		struct json_object *obj_;
		std::vector<struct json_value> *arr_;
	} un_;

	json_value() : type_(JVT_NULL) {}
	json_value(bool b) : type_(b ? JVT_TRUE : JVT_FALSE) {}
	json_value(long i) : type_(JVT_INTEGER) { un_.int_ = i; }
	json_value(double i) : type_(JVT_FLOAT) { un_.float_ = i; }
	json_value(const char* i) : type_(i == NULL ? JVT_NULL : JVT_STRING) { if (i) un_.str_ = new std::string(i); }
	json_value(const std::string& i) : type_(JVT_STRING) { un_.str_ = new std::string(i); }
	json_value(const struct json_object& obj) : type_(JVT_OBJECT) { un_.obj_ = new json_object(obj); }
	json_value(const std::vector<struct json_value>& arr) : type_(JVT_ARRAY) { un_.arr_ = new std::vector<json_value>(arr); }

	json_value(const json_value& x);
	~json_value();
};

json_value::json_value(const json_value& x)
		: type_(x.type_)
{
	switch (type_) {
	case JVT_TRUE:
	case JVT_FALSE:
	case JVT_NULL:
		// nothing
		break;
	case JVT_INTEGER:
		un_.int_ = x.un_.int_;
		break;
	case JVT_FLOAT:
		un_.float_ = x.un_.float_;
		break;
	case JVT_STRING:
		un_.str_ = new std::string(*x.un_.str_);
		break;
	case JVT_OBJECT:
		un_.obj_ = new json_object(*x.un_.obj_);
		break;
	case JVT_ARRAY:
		un_.arr_ = new std::vector<json_value>(*x.un_.arr_);
		break;
	default:
		exit(1);
		break;
	}
}

json_value::~json_value()
{
	if (type_ == JVT_STRING && un_.str_ != NULL) {
		delete un_.str_;
	}
	else if (type_ == JVT_OBJECT && un_.obj_ != NULL) {
		delete un_.obj_;
	}
	else if (type_ == JVT_ARRAY && un_.arr_ != NULL) {
		delete un_.arr_;
	}
}

struct json_pair {
	std::string nam_;
	struct json_value val_;

	json_pair(const std::string& nam, const struct json_value& val) : nam_(nam), val_(val) {}

};

std::ostream& operator<<(std::ostream& out, const struct json_object& obj);
std::ostream& operator<<(std::ostream& out, const struct json_value& val);
std::ostream& operator<<(std::ostream& out, const std::vector<struct json_value>& arr);

std::ostream& operator<<(std::ostream& out, const struct json_object& obj)
{
	out << "{";
	bool first = true;
	for (std::list<struct json_pair>::const_iterator iter = obj.prs_.begin();
		iter != obj.prs_.end();
			++iter) {
		if (first) { first = false; } else { out << ", "; }
		out << iter->nam_ << ": " << iter->val_;
	}

	out << "}";
	return out;
}

std::ostream& operator<<(std::ostream& out, const struct json_value& val)
{
	switch (val.type_) {
	case JVT_TRUE:
		out << "true";
		break;
	case JVT_FALSE:
		out << "false";
		break;
	case JVT_NULL:
		out << "null";
		break;
	case JVT_INTEGER:
		out << val.un_.int_;
		break;
	case JVT_FLOAT:
		out << val.un_.float_;
		break;
	case JVT_STRING:
		out << '"' << *val.un_.str_ << '"';
		break;
	case JVT_OBJECT:
		out << *val.un_.obj_;
		break;
	case JVT_ARRAY:
		out << *val.un_.arr_;
		break;	
	default:
		out << "ukn-type";
	}

	return out;
}

std::ostream& operator<<(std::ostream& out, const std::vector<struct json_value>& arr)
{
	out << "[";
	bool first = true;
	for (std::vector<struct json_value>::const_iterator iter = arr.begin();
		iter != arr.end();
			++iter) {
		if (first) { first = false; } else { out << ", "; }
		out << *iter;
	}

	out << "]";
	return out;
}

struct json_pair make_json_pair(const std::string& nam, bool b)
{
	return json_pair(nam, b);
}

struct json_pair make_json_pair(const std::string& nam)
{
	return json_pair(nam, json_value());
}

struct json_pair make_json_pair(const std::string& nam, long i)
{
	return json_pair(nam, i);
}
struct json_pair make_json_pair(const std::string& nam, double d)
{
	return json_pair(nam, d);
}

struct json_pair make_json_pair(const std::string& nam, const std::string& val)
{
	return json_pair(nam, val);
}

struct json_pair make_json_pair(const std::string& nam, const char* val)
{
	if (val == NULL) return make_json_pair(nam);
	return make_json_pair(nam, std::string(val));
}

struct json_pair make_json_pair(const std::string& nam, const std::vector<struct json_value>& arr)
{
	return json_pair(nam, arr);
}

struct json_pair make_json_pair(const std::string& nam, const json_object& obj)
{
	return json_pair(nam, obj);
}

int main(int argc, char **argv)
{
	struct json_object obj1, obj2;
	struct json_value val1;
	std::vector<struct json_value> bs;

	//val1.type_ = JVT_TRUE;
	//bs.push_back(val1);
	bs.push_back(json_value(true));
	//val1.type_ = JVT_FALSE;
	//bs.push_back(val1);
	bs.push_back(json_value(false));
	//val1.type_ = JVT_NULL;
	//bs.push_back(val1);
	bs.push_back(json_value());
	val1.type_ = JVT_INTEGER;
	val1.un_.int_ = 1234;
	//bs.push_back(val1);
	bs.push_back(json_value(1234l));
	val1.type_ = JVT_FLOAT;
	val1.un_.float_ = 1234.34;
	//bs.push_back(val1);
	bs.push_back(json_value(1234.56));
	val1.type_ = JVT_STRING;
	val1.un_.str_ = new std::string("China");
	bs.push_back(val1);
	bs.push_back(json_value("USA"));
	bs.push_back(json_value((char *)NULL));
	val1.type_ = JVT_OBJECT;
	obj2.prs_.push_back(make_json_pair("author", "mark"));
	obj2.prs_.push_back(make_json_pair("price", 33.4));
	val1.un_.obj_ = new struct json_object; *val1.un_.obj_ = obj2;
	bs.push_back(val1);

	obj1.prs_.push_back(make_json_pair("name", std::string("robert")));
	obj1.prs_.push_back(make_json_pair("age", 40L));
	obj1.prs_.push_back(make_json_pair("height", 164.4));
	obj1.prs_.push_back(make_json_pair("books", bs));

	std::cerr << obj1 << std::endl;
}

