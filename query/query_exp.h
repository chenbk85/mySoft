/* query_exp.hpp
**/

template <typename OBJ>
class query_exp {
public:
	virtual bool test(const OBJ& obj) = 0;
	virtual std::string to_string() = 0;
};

template <typename OBJ>
class and_query_exp : public query_exp<OBJ> {
public:
	and_query_exp(query_exp<OBJ>* left, query_exp<OBJ>* right)
		: left_(left), right_(right)
	{}

	virtual bool test(const OBJ& obj)
	{
		if (left_->test(obj) && right_->test(obj)) 
			return true;
		return false;
	}

	virtual std::string to_string()
	{
		return "(" + left_->to_string() + " AND " + right_->to_string() + ")";
	}
private:
	query_exp<OBJ> *left_, *right_;
};

template <typename OBJ>
class or_query_exp : public query_exp<OBJ> {
public:
	 or_query_exp(query_exp<OBJ>* left, query_exp<OBJ>* right)
		: left_(left), right_(right)
	{}

	virtual bool test(const OBJ& obj)
	{
		if  (left_->test(obj) || right_->test(obj))
			return true;
		return false;
	}

	virtual std::string to_string()
	{
		return "(" + left_->to_string() + " OR " + right_->to_string() + ")";
	}
private:
	query_exp<OBJ> *left_, *right_;
};

template <typename OBJ>
class not_query_exp : public query_exp<OBJ> {
public:
	not_query_exp(query_exp<OBJ>* left, query_exp<OBJ>* right)
		: left_(left), right_(right)
	{}

	virtual bool test(const OBJ& obj)
	{
		if (left_->test(obj) && !right_->test(obj))
			return true;
		return false;
	}

	virtual std::string to_string()
	{
		return "(" + left_->to_string() + " NOT " + right_->to_string() + ")";
	}
private:
	query_exp<OBJ> *left_, *right_;
};

template <typename Tp>
static std::string to_string(const Tp& v)
{
	return std::string(v);
}

template<> static std::string to_string<int>(const int& v) 
{ char buf[128]; snprintf(buf, sizeof buf, "%d", v); return buf;}
template<> static std::string to_string<unsigned int>(const unsigned int& v) 
{ char buf[128]; snprintf(buf, sizeof buf, "%u", v); return buf;}
template<> static std::string to_string<long>(const long& v) 
{ char buf[128]; snprintf(buf, sizeof buf, "%ld", v); return buf;}
template<> static std::string to_string<unsigned long>(const unsigned long& v) 
{ char buf[128]; snprintf(buf, sizeof buf, "%lu", v); return buf;}
template<> static std::string to_string<double>(const double& v)
{ char buf[128]; snprintf(buf, sizeof buf, "%g", v); return buf;}
template<> static std::string to_string<float>(const float& v)
{ char buf[128]; snprintf(buf, sizeof buf, "%g", v); return buf;}

template <typename OBJ, typename Tp, typename Pred>
class unary_field_exp : public query_exp<OBJ> {
public:
	unary_field_exp(const std::string& field, const Tp& val) : field_(field), val_(val)
	{}

	virtual bool test(const OBJ& obj)
	{
		Pred pred;
		if (pred(*(Tp *)obj.field_data(field_.c_str()), val_))
			return true;
		return false;
	}

	virtual std::string to_string()
	{
		Pred pred;
		return field_ + pred.ops() + ::to_string(val_);
	}
private:
	std::string field_;
	Tp val_;
};

template <typename OBJ, typename Tp, typename Pred>
class binary_field_exp : public query_exp<OBJ> {
public:
	binary_field_exp(const std::string& field, const Tp& v1, const Tp& v2)
		: field_(field), v1_(v1), v2_(v2)
	{}
	
	virtual bool test(const OBJ& obj)
	{
		Pred pred;
		if (pred(*(Tp *)obj.field_data(field_.c_str()), v1_, v2_))
			return true;
		return false;
	}

	virtual std::string to_string()
	{
		Pred pred;
		return field_ + ":" + pred.lops() + ::to_string(v1_) + ", " + ::to_string(v2_) + pred.rops();
	}
private:
	std::string field_;
	Tp v1_, v2_;
};

template <typename Tp> struct field_greater {
	bool operator()(const Tp& x, const Tp& y) const { return x > y; }
	std::string ops() const { return " > "; }
};
template <typename Tp> struct field_greater_equal {
	bool operator()(const Tp& x, const Tp& y) const { return x >= y; }
	std::string ops() const { return " >= "; }
};
template <typename Tp> struct field_less {
	bool operator()(const Tp& x, const Tp& y) const { return x >= y; }
	std::string ops() const { return " < "; } 
};
template <typename Tp> struct field_less_equal {
	bool operator()(const Tp& x, const Tp& y) const { return x >= y; }
	std::string ops() const { return " <= "; };
};
template <typename Tp> struct field_equal_to {
	bool operator()(const Tp& x, const Tp& y) const { return x >= y; }
	std::string ops() const { return " == "; };
};
template <typename Tp> struct field_not_equal_to {
	bool operator()(const Tp& x, const Tp& y) const { return x >= y; }
	std::string ops() const { return " != "; } 
};

template <typename Tp> struct field_between_ee {
	bool operator()(const Tp& v, const Tp& x, const Tp& y) { return v > x && v < y; }
	std::string lops() const { return "("; }
	std::string rops() const { return "]"; }
}; 
template <typename Tp>
struct field_between_ei { bool operator()(const Tp& v, const Tp& x, const Tp& y) { return v > x && v <= y; }
	std::string lops() const { return "("; }
	std::string rops() const { return "]"; }
}; 

template <typename Tp>
struct field_between_ie { bool operator()(const Tp& v, const Tp& x, const Tp& y) { return v >= x && v < y; }
	std::string lops() const { return "["; }
	std::string rops() const { return ")"; }
}; 

template <typename Tp>
struct field_between_ii { bool operator()(const Tp& v, const Tp& x, const Tp& y) { return v >= x && v <= y; }
	std::string lops() const { return " [ "; }
	std::string rops() const { return " ] "; }
}; 
