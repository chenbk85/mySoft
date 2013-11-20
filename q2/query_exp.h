/* query_exp.hpp
**/

template <typename OBJ>
class query_exp {
	virtual bool test(const OBJ& obj) = 0;
};

template <typename OBJ>
class and_query_exp : public query_exp<OBJ> {
public:
	and_query_exp(query_exp* left, query_exp* right)
		: _left(left), _right(right)
	{}

	virtual bool test(const OBJ& obj)
	{
		if (_left->test(obj) && _right->test(obj)) 
			return true;
		return false;
	}
private:
	query_exp<OBJ> *_left, *_right;
};

template <typename OBJ>
class or_query_exp : public query_exp<OBJ> {
	public or_query_exp(query_exp* left, query_exp* right)
		: _left(left), _right(right)
	{}

	virtual bool test()
	{
		if  (_left->test(obj) || _right->test(obj))
			return true;
		return false;
	}
private:
	query_exp<OBJ> *_left, *_right;
};

template <typename OBJ>
class not_query_exp : public query_exp<OBJ> {
	public not_query_exp(query_exp* left, query_exp* right)
		: _left(left), _right(right)
	{}

	virtual bool test(const OBJ& obj)
	{
		if (_left->test(obj) && !_right->test(obj))
			return true;
		return false;
	}
private:
	query_exp<OBJ> *_left, *_right;
};

template <typename OBJ, typename Tp, typename Pred>
class unary_field_exp : public query_exp<OBJ> {
	public less_than_exp(const std::string& field, const Tp& val) : _val(val)
	{}
	
	virtual bool test(const OBJ& obj)
	{
		Pred pred;
		if (pred(obj.get(_field), val))
			return true;
		return false;
	}
private:
	std::string _field;
	Tp _val;
};

template <typename OBJ, typename Tp, typename Pred>
class binary_field_exp : public query_exp<OBJ> {
	public between_than_exp(const std::string& field, const Tp& v1, const Tp& v2)
		_field(field), _v1(v1), _v2(v2)
	{}
	
	virtual bool test(const OBJ& obj)
	{
		Pred pred;
		if (pred(obj.get(_field), _v1, _v2))
			return true;
		return false;
	}
private:
	std::string _field;
	Tp _v1, _v2;
};
	| NAME COLON '(' INT ',' INT ')' { $$ = new unary_field_exp<OBJ, long, field_between_ee<long> >($1, $4, $6); }
	| NAME COLON '(' INT ',' INT ']' { $$ = new unary_field_exp<OBJ, long, field_between_ei<long> >($1, $4, $6); }
	| NAME COLON '[' INT ',' INT ')' { $$ = new unary_field_exp<OBJ, long, field_between_ie<long> >($1, $4, $6); }
	| NAME COLON '[' INT ',' INT ']' { $$ = new unary_field_exp<OBJ, long, field_between_ii<long> >($1, $4, $6); }

template <typename Tp>
class field_between_ee { bool operator()(const Tp& v, const Tp& x, const Tp& y) { return v > x && v < y; } };
template <typename Tp>
class field_between_ei { bool operator()(const Tp& v, const Tp& x, const Tp& y) { return v > x && v <= y; } };
template <typename Tp>
class field_between_ie { bool operator()(const Tp& v, const Tp& x, const Tp& y) { return v >= x && v < y; } };
template <typename Tp>
class field_between_ii { bool operator()(const Tp& v, const Tp& x, const Tp& y) { return v >= x && v <= y; } };

