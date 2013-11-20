template <typename Tp>
class posting_table {
	char* name;
	bstree<Tp, docid_freq_set*> ids_;
};


template <typename Tp>
class less_than_compare {
public:
	typedef bstree<Tp, docid_freq_set*>::const_iterator const_iterator;
	const_iterator begin() const;
	const_iterator end() const;
private:
	Tp val_;
};

template <typename Tp>
class reverse_less_than_compare {
};

template <typename Tp>
class less_equal_than_compare {}
class reverse_less_equal_than_compare {}

class larger_than_compare {}
class reverse_larger_than_compare {}

class between_inclusive_compare {}
class reverse_between_inclusive_compare {}

class between_exclusive_compare {}
class reverse_between_exclusive_compare {}

class query {
public:
	void expand();
	virtual int next_doc();
private:
};

class and_query : public query {
public:
	virtual int next_doc()
	{
		std::vector<query*>::iterator iter = sqs_.begin();
		int did = iter->next_doc(); ++iter;
		size_t matched = 1;

		while (1) {
			int nid = iter->skip_doc(did);
			if (nid == BUTT_ID) return BUTT_ID;
			if (nid > did) {
				did = nid;
				matched = 1;
			}
			else if (++matched == sqs_.size()) {
				return did;
			}
		
			if (++iter == sqs_.end())
				iter = sqs_.begin();
		}
	}
private:
	std::vector<query*> sqs_;
};

class or_query : public query {
};

class not_query : public query {
};

template <typename Tp, typename Comp>
class unary_query : public {
public:
	unary_query(typename posting_table<Tp>& pt, Tp val);

	virtual int start() {
		iter = Comp().begin();
		end = Comp().end();
	}

	virtual int skip_to(int doc) {
		while (*iter < doc && iter != end)
			++iter;
		if (iter == end) return BUTT_ID;
		return *iter;
	}

	virtual int next_doc() {
		int id = *iter;
		++iter;
		return id;
	}
private:
	const posting_table& pt_;
	Tp val_;	
	typename posting_table<Tp>::const_iterator iter_, end_;
};

template <typename Tp>
class larger_query : public query {

private:
	Tp val_;
};

template <>
class larger_than_query<const char*> : public query {
public:
	larger_than_query(const string_posting_table& pt, const char* val)
		: pt_(pt), val_(val)
	{}
	virtual int start()
	{
		iter = compare().begin();
		end = compare().end();
	}
	virtual int skip_to(int doc)
	{
	}
	virtual int next_doc()
	{
		
	}
private:
	const string_posting_table& pt_;
	const char* val_;
	const_iterator iter_, end_;
};

