#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <stdexcept>
#include <iomanip>
#include <tr1/unordered_map>
#include <tr1/unordered_set>
#include <set>
#include <sys/time.h>
#include <xmlconfig.h>
#include <stx/btree_map.h>
#include <malloc.h>

typedef short field_id_type;
typedef int doc_id_type;

#define CHUNK_SIZE	(16*1024*1024)
#define NID		-1

#define TDT_MASK	0x7
#define TDT_UKN		0x0
#define TDT_LONG	0x1
#define TDT_DOUBLE	0x2
#define TDT_STRING	0x3

class nofree_allocator {
public:
	nofree_allocator(size_t chunk_size)
		: chunk_size_(chunk_size),
		  cur_chunks_(NULL),
		  pos_(0)
	{
		// nothing
	}
	
	template <typename Tp>
	Tp* allocate()
	{
		void *addr = malloc(sizeof(Tp));
		return new(addr) Tp();
	}

	template <typename Tp>
	Tp* allocate(size_t count)
	{
		void* addr = malloc(count * sizeof(Tp));
		return new(addr) Tp[count];
	}
private:
	void* malloc(size_t size)
	{
		if (cur_chunks_ == NULL || (chunk_size_ - pos_ < size)) {
			cur_chunks_ = (void *)::malloc(chunk_size_);
			pos_ = 0;
			chunks_.push_back(cur_chunks_);
		}

		void *addr = (void  *)((char *)cur_chunks_ + pos_);
		pos_ += size;
		return addr;
	}
private:
	size_t chunk_size_;
	void *cur_chunks_;
	ptrdiff_t pos_;
	std::vector<void *> chunks_;
};

static nofree_allocator allocator(CHUNK_SIZE);


template <typename SIZE_TYPE, typename Tp>
class dyn_array {
public:
	typedef SIZE_TYPE size_type;
	typedef Tp data_type;

	dyn_array()
		: count_(0), allocated_(0), data_(NULL)
	{
		// nothing
	}
	
	dyn_array(size_type count)
		: count_(count), allocated_(count)
	{
		data_ = new data_type[count_];
	}

	dyn_array(const dyn_array& da)
	{
		if (this != &da) {
			this->reset(da.count_, da.allocated_, da.data_);
			const_cast<dyn_array&>(da).reset(0, 0, NULL);
		}
	}

	dyn_array<SIZE_TYPE, Tp>& operator=(const dyn_array<SIZE_TYPE, Tp>& da)
	{
		if (this != &da) {
			this->reset(da.count_, da.allocated_, da.data_);
			const_cast<dyn_array&>(da).reset(0, 0, NULL);
		}

		return *this;
	}

	~dyn_array()
	{
		if (data_ != NULL)
			delete[] data_;
	}

	size_type count() const
	{
		return count_;
	}
	
	data_type* data() const
	{
		return data_;
	}

	data_type& operator[](int index) 
	{
		assert(index >= 0 && index < count_);
		return data_[index];
	}
	
	const data_type& operator[](int index) const
	{
		assert(index >= 0 && index < count_);
		return data_[index];
	}

	int push_back(const Tp& v)
	{
		if (count_ == allocated_) {
			
			// TODO: how to effectly expand??
			
			size_type ncount = (count_ < 1) ? 1 : (count_ * 2);
			Tp* ndata = new data_type[ncount];
			if (ndata == NULL)
				return -1;

			for (size_type i = 0; i < count_; ++i) {
				ndata[i] = data_[i];
			}

			if (data_ != NULL) {
				delete [] data_;
			}

			allocated_ = ncount;
			data_ = ndata;
		}

		data_[count_++] = v;
		return  0;
	}

	int insert(size_type pos)
	{
		if (count_ == allocated_) {

			// TODO: how to effectly expand??
			
			size_type ncount = (count_ < 1) ? 1 : (count_ * 2);
			Tp* ndata = new data_type[ncount];
			if (ndata == NULL)
				return -1;

			for (size_type i = 0; i < pos; ++i) {
				ndata[i] = data_[i];
			}

			for (size_type i = count_; i > pos; --i) {
				ndata[i] = data_[i - 1];
			}
			
			if (data_ != NULL) {
				delete[] data_;
			}

			allocated_ = ncount;
			data_ = ndata;
		}
		else for (size_type i = count_; i > pos; --i) {
			data_[i] = data_[i - 1];
		}

		++count_;
		return 0;
	}

	int insert(size_type pos, const Tp& v)
	{
		if (insert(pos) < 0)
			return 0;
		data_[pos] = v;
		return 0;
	}

	void reset(size_type count, size_type allocated, data_type* data)
	{
		count_ = count;
		allocated_ = allocated;
		data_ = data;
	}

	int reserve(size_type capacity)
	{
		if (capacity < count_) {
			assert("can not reserve to less than the valid count!" == NULL);
			return -1;
		}

		if (allocated_ == capacity) {
			return 0;	// do nothing
		}
		
		// re-set anyway
		Tp* ndata = new data_type[capacity];
		if (ndata == NULL)
			return -1;

		for (size_type i = 0; i < count_; ++i) {
			ndata[i] = data_[i];
		}

		if (data_ != NULL) {
			delete[] data_;
		}

		data_ = ndata;
		allocated_ = capacity;
		return 0;
	}

	template <typename CMP>
	data_type* qsearch(const data_type& val, size_type& pos)
	{
		size_type i = 0, j = count_;
		CMP cmp;

		while (i < j) {
			size_type m = (i + j) / 2;
			int retval = cmp(data_[m], val);
			if (retval > 0) {
				j = m;
			}
			else if (retval < 0) {
				i = m + 1;
			}
			else {
				pos = m;
				return &data_[m];
			}
		}

		pos = i;
		return NULL;
	}

	template <typename CMP>
	const data_type* qsearch(const data_type& val, size_type& pos) const
	{
		size_type i = 0, j = count_;
		CMP cmp;

		while (i < j) {
			size_type m = (i + j) / 2;
			int retval = cmp(data_[m], val);
			if (retval > 0) {
				j = m;
			}
			else if (retval < 0) {
				i = m + 1;
			}
			else {
				pos = m;
				return &data_[m];
			}
		}

		pos = i;
		return NULL;
	}
private:
	size_type count_;
	size_type allocated_;
	data_type* data_;
};

class fieldinfo {
public:
	fieldinfo()
		: name_(NULL), index_(NID), flags_(0)
	{
		// nothing
	}

	fieldinfo(const char* name, int flags)
		: name_(name), index_(-1), flags_(flags)
	{
		// nothing
	}

	field_id_type index() const { return index_; }
	void index(field_id_type i) { index_ = i; }

	const char* name() const { return name_; }
	int flags() const { return flags_; }
	int data_type() const { return flags_ & TDT_MASK; }
private:
	const char* name_;
	field_id_type index_;
	int flags_;
};

class fieldinfos {
public:
	fieldinfos() : fields_() {}
	fieldinfos(field_id_type count) : fields_(count) {}
public:
	int add(const fieldinfo& fi);
	int add(field_id_type index, const fieldinfo& fi);

	field_id_type name2index(const char* name) const
	{
		std::tr1::unordered_map<std::string, field_id_type>::const_iterator iter = name2id_map_.find(name);
		if (iter != name2id_map_.end()) return iter->second;
		return NID;
	}

	field_id_type count() const { return fields_.count(); }

	fieldinfo& get(int index) { return fields_[index]; }
	const fieldinfo& get(int index) const { return fields_[index]; }

	fieldinfo* get(const char* name)
	{
		field_id_type index = name2index(name);
		if (index == NID)
			return NULL;
		return &fields_[index];
	}

	const fieldinfo* get(const char* name) const
	{
		field_id_type index = name2index(name);
		if (index == NID)
			return NULL;
		return &fields_[index];
	}
private:
	std::tr1::unordered_map<std::string, field_id_type> name2id_map_;
	dyn_array<field_id_type, fieldinfo> fields_;
};

int fieldinfos::add(const fieldinfo& fi)
{
	field_id_type pos = NID, i = 0;

	for (/* ** */; i < fields_.count(); ++i) {
		if (fields_[i].name() == NULL) {
			if (pos == NID) {
				pos = i;
			}
		}
		else if (strcmp(fields_[i].name(), fi.name()) == 0) {
			break;
		}
	}

	if (i < fields_.count()) {
		assert("duplicated field" == NULL);
		return -1;
	}

	if (pos == NID) {
		pos = fields_.count();
		if (fields_.push_back(fi) < 0)
			return -1;
	}
	else {
		fields_[pos] = fi;
	}

	fields_[pos].index(pos);
	name2id_map_[fi.name()] = pos;
	return pos;
}

int fieldinfos::add(field_id_type index, const fieldinfo& fi)
{
	if (index < 0 || index > fields_.count()) {
		assert("invalid index or too  many fields than expected!" == NULL);
		return -1;
	}

	if (fields_[index].name() != NULL && strcmp(fields_[index].name(), fi.name()) != 0) {
		// can not overwrite???
		assert("can not overwrite fieldinfo?!" == NULL);
		return -1;
	}
	
	fields_[index] = fi;
	fields_[index].index(index);
	name2id_map_[fi.name()] = index;

	return 0;
}

struct field_data {
	field_id_type index_;
	const void* data_;
};

struct doc_field_id_compare {
	int operator()(const field_data& x, const field_data& y)
	{
		return x.index_ - y.index_;
	}
};

class document {
public:
	document();
	document(field_id_type count);
public:
	int add(field_id_type index, const void *data);

	const struct field_data& get(field_id_type index) const { return fields_[index]; }
	struct field_data& get(field_id_type index) { return fields_[index]; }

	doc_id_type id() const { return id_; }
	void id(doc_id_type i) { id_ = i; }

	field_id_type field_count() const { return fields_.count(); }
	const void* field_data(field_id_type index) const;
private:
	doc_id_type id_;
	dyn_array<field_id_type, struct field_data> fields_;
};

document::document()
	: fields_()
{
	// nothing
}

document::document(field_id_type count)
	: fields_(count)
{
	for (field_id_type i = 0; i < count; ++i) {
		fields_[i].index_ = i;
		fields_[i].data_ = NULL;
	}
}

int document::add(field_id_type index, const void* data)
{
	struct field_data key = { index, data };
	field_id_type pos;
	struct field_data* fd = fields_.qsearch<doc_field_id_compare>(key, pos);	
	if (fd == NULL) {
		return fields_.insert(pos, key);
	}
	else if (fd->data_ == NULL) {
		fd->data_ = data;
		return 0;
	}

	assert("add duplicated field" == NULL);
	return -1;
}

const void* document::field_data(field_id_type index) const
{
	struct field_data key = { index, NULL };
	field_id_type pos;
	const struct field_data* fd = fields_.qsearch<doc_field_id_compare>(key, pos);	
	if (fd == NULL)
		return NULL;
	return fd->data_;
}

class posting_data {
public:
	posting_data() : data_(NULL), ids_() {}
	posting_data(const void *data) : data_(data), ids_() {}

	posting_data(const posting_data& o) : data_(o.data_), ids_(o.ids_) {}
	posting_data& operator=(const posting_data& o) { if (this != &o) { data_ = o.data_; ids_ = o.ids_; } return *this; }
public:
	const void* data() const
	{
		return data_;
	}
	
	doc_id_type count() const
	{
		return ids_.count();
	}
	
	doc_id_type ids(doc_id_type index) const
	{
		return ids_[index];
	}

	int add_doc(doc_id_type doc)
	{
		return ids_.push_back(doc);
	}

	int set(const void *data, doc_id_type doc)
	{
		data_ = data;
		return add_doc(doc);
	}
private:
	const void *data_;
	dyn_array<doc_id_type, doc_id_type> ids_;
};

template <typename Tp>
struct posting_data_compare {
	int operator()(const posting_data& x, const posting_data& y) {
		if (*(Tp *)x.data() < *(Tp *)y.data())
			return -1;
		else if (*(Tp *)x.data() > *(Tp *)y.data())
			return +1;
		return 0;
	}
};

template <>
struct posting_data_compare<const char*> {
	int operator()(const posting_data& x, const posting_data& y) {
		return strcmp((const char *)x.data(), (const char *)y.data());
	}
};

class sKey {
public:
	sKey() {}
	sKey(const void* data) : s_((const char *)data) {}
	bool operator<(const sKey& o) const {
		return (strcmp(s_, o.s_) < 0);
	}
private:
	const char* s_;
};

class vKey {
public:
	vKey() {}
	vKey(const void* data) : v_(data) {}
	bool operator<(const vKey& o) const {
		return *(const unsigned long *)v_ < *(const unsigned long *)o.v_;
	}
private:
	const void *v_;
};

struct posting_btree_traits
{
	static bool const selfverify = false;
	static bool const debug = false;
	static int const leafslots = 32; //64; //256; //1024;
	static int const innerslots = 32; //64;//256; //1024;
//1024: 384#/s
//256: 844#/s
//64: 1196/s
//32: 1270/s
//8:
};


class posting_table {
public:
	posting_table()
		: fi_(NULL),
		  postings_v_()
	{}
	posting_table(const posting_table& x) : fi_(x.fi_), postings_v_(x.postings_v_), postings_s_(x.postings_s_)
	{}
	posting_table(const fieldinfo* fi)
		: fi_(fi),
		  postings_v_()
	{}
public:
	void set_fieldinfo(const fieldinfo* fi) { fi_ = fi; }
	int add(const void *data, doc_id_type did);
public:
	template <typename _OSTR>
	void dump(_OSTR& out) const;

//	posting_data* qsearch(const void* data, doc_id_type& pos);
//	const posting_data* qsearch(const void* data, doc_id_type& pos) const;
private:
	const fieldinfo* fi_;
	typedef stx::btree_map<vKey, posting_data, std::less<vKey>, posting_btree_traits> vTable;
	typedef stx::btree_map<sKey, posting_data, std::less<sKey>, posting_btree_traits> sTable;
	vTable postings_v_;
	sTable postings_s_;
};

template <typename _OSTR>
void posting_table::dump(_OSTR& out) const
{
	out << "Name: " << fi_->name() << ", flags: 0x" << std::hex << (fi_->flags() & ~ TDT_MASK) << std::dec << ", type: ";
	switch (fi_->data_type()) {
	case TDT_LONG:
		out << "long"; break;
	case TDT_DOUBLE:
		out << "double" ; break;
	case TDT_STRING:
		out << "string"; break;
	case TDT_UKN:
		out << "ukn"; break;
	default:
		out << "uspt"; break;
	}

	out << ", count: " << postings_v_.size() << std::endl;
	for (vTable::const_iterator iter = postings_v_.begin();
		iter != postings_v_.end();
			++iter) {
		posting_data& pd = iter->second;
		out << "\t";
		switch (fi_->data_type()) {
		case TDT_LONG:
			out << *(long *)pd.data(); break;
		case TDT_DOUBLE:
			out << *(double *)pd.data(); break;
		case TDT_STRING:
			out << (const char *)pd.data(); break;
		case TDT_UKN:
		default:
			out << "0x" << pd.data(); break;
		}

		out << ": ";
		for (doc_id_type j = 0; j < pd.count(); ++j) {
			out << " " << pd.ids(j);
		}

		out << std::endl;
	}
}
#ifdef Q
posting_data* posting_table::qsearch(const void *data, doc_id_type& pos)
{
	if (fi_->data_type() == TDT_LONG) {
		return postings_.qsearch<posting_data_compare<long> >(data, pos);
	}
	else if (fi_->data_type() == TDT_DOUBLE) {
		return postings_.qsearch<posting_data_compare<double> >(data, pos);
	}
	else if (fi_->data_type() == TDT_STRING) {
		return postings_.qsearch<posting_data_compare<const char *> >(data, pos);
	}
	else {
		assert("unsupport data type!" == NULL);
		return NULL;
	}

	return NULL;
}

const posting_data* posting_table::qsearch(const void *data, doc_id_type& pos) const
{
	if (fi_->data_type() == TDT_LONG || fi_->data_type() == TDT_DOUBLE) {
		return postings_.qsearch<posting_data_compare<long> >(data, pos);
	}
	else if (fi_->data_type() == TDT_DOUBLE) {
		return postings_.qsearch<posting_data_compare<double> >(data, pos);
	}
	else if (fi_->data_type() == TDT_STRING) {
		return postings_.qsearch<posting_data_compare<const char *> >(data, pos);
	}
	else {
		assert("unsupport data type!" == NULL);
		return NULL;
	}

	return NULL;
}
#endif
int posting_table::add(const void* data, doc_id_type did)
{
	if (fi_->data_type() == TDT_LONG || fi_->data_type() == TDT_DOUBLE) {
		vKey vk(data);
#if 0
		stx::btree_map<vKey, posting_data>::iterator iter = postings_v_.find(vk);
		if  (iter != postings_v_.end()) {
			// append to the existing posting_data
			iter->second.add_doc(did);
		}
		else {
			// insert new
			postings_v_.insert(vk).first.data().add_doc(did);
		}
#else
			postings_v_[vk].add_doc(did);
#endif
	}
	else {
		sKey sk(data);
		postings_s_[sk].add_doc(did);
	}

	return 0;
}

class hits {
public:
//	typedef std::tr1::unordered_set<doc_id_type> ids_type;
	typedef std::set<doc_id_type> ids_type;
public:
	int add(doc_id_type id)
	{
		ids_.insert(id);
	}

	void swap(hits& h)
	{
		ids_.swap(h.ids_);
	}

	int set_intersection(const hits& h)
	{
		ids_type nids;
		std::insert_iterator<ids_type> insertor(nids, nids.begin());
		std::set_intersection(ids_.begin(), ids_.end(),
				      h.ids_.begin(), h.ids_.end(),
				      insertor);
//std::cout << "old=" << ids_.size() << ", h-size=" << h.ids_.size() << ", n-size=" << nids.size() << std::endl;
		ids_.swap(nids);
		return 0;
	}

	int set_union(const hits& h)
	{
		for (ids_type::const_iterator iter = h.ids_.begin();
			iter != h.ids_.end();
				++iter) {
			ids_.insert(*iter);
		}

		return 0;
	}

	int set_sub(const hits& h)
	{
		for (ids_type::const_iterator iter = h.ids_.begin();
			iter != h.ids_.end();
				++iter) {
			if (ids_.find(*iter) != ids_.end()) {
				ids_.erase(*iter);
			}
		}

		return 0;
	}

	doc_id_type count() const { return ids_.size(); }
	const ids_type& ids() const { return ids_; }
	ids_type& ids() { return ids_; }
private:
	ids_type ids_;
};

#if 0
#define OT_AND	0
#define OT_OR	1
#define OT_SUB	2

class expression {
public:
	expression(const posting_table& pt) : pt_(pt) {}
	virtual int execute(hits& h) = 0;
protected:
	const posting_table& pt_;
};

template <typename Tp>
class expr_less : public expression {
public:
	expr_less(const posting_table& pt, const Tp& val)
		: expression(pt), val_(val) 
	{
		// nothing
	}

	virtual int execute(hits& h) 
	{
		doc_id_type pos;
		pt_.qsearch(&val_, pos);

		for (doc_id_type i = 0; i < pos; ++i) {
			const posting_data* pd = pt_.get(i);
			for (doc_id_type j = 0; j < pd->count(); ++j) {
				h.add(pd->ids(j));
			}
		}

		return 0;
	}
private:
	Tp val_;
};

template <>
class expr_less<std::string> : public expression {
public:
	expr_less(const posting_table& pt, const std::string& val)
		: expression(pt), val_(val)
	{
		// nothing
	}

	virtual int execute(hits& h) 
	{
		doc_id_type pos;
		pt_.qsearch(val_.c_str(), pos);

		for (doc_id_type i = 0; i < pos; ++i) {
			const posting_data* pd = pt_.get(i);
			for (doc_id_type j = 0; j < pd->count(); ++j) {
				h.add(pd->ids(j));
			}
		}

		return 0;
	}
private:
	std::string val_;
};

template <typename Tp>
class expr_inclusive_between : public expression {
public:
	expr_inclusive_between(const posting_table& pt, const Tp& start, const Tp& last)
		: expression(pt), start_(start), last_(last)
	{
		// nothing
	}

	virtual int execute(hits& h)
	{
		doc_id_type s, l;

		pt_.qsearch(&start_, s);
		if (pt_.qsearch(&last_, l) == NULL) --l;
		
		for (doc_id_type i = s; i <= l; ++i) {
			const posting_data* pd = pt_.get(i);
			for (doc_id_type j = 0; j < pd->count(); ++j) {
				h.add(pd->ids(j));
			}
		}

		return 0;
	}
private:
	Tp start_, last_;
};

template <>
class expr_inclusive_between<std::string> : public expression {
public:
	expr_inclusive_between(const posting_table& pt, const std::string& start, const std::string& last)
		: expression(pt), start_(start), last_(last)
	{
		// nothing	
	}

	virtual int execute(hits& h)
	{
		doc_id_type s, l;

		pt_.qsearch(start_.c_str(), s);
		if (pt_.qsearch(last_.c_str(), l) == NULL) --l;
		
		for (doc_id_type i = s; i <= l; ++i) {
			const posting_data* pd = pt_.get(i);
			for (doc_id_type j = 0; j < pd->count(); ++j) {
				h.add(pd->ids(j));
			}
		}

		return 0;
	}
private:
	std::string start_, last_;
};

class query {
public:
	query(query* l, query* r, int __op)
		: left_(l), right_(r)
	{
		__exp_op_.__op = __op;
	}
	query(expression* __exp)
		: left_(NULL), right_(NULL)
	{
		__exp_op_.__exp = __exp;
	}
		
public:
	const query* left() const { return left_; }
	query* left() { return left_; }
	void left(query* left) { left_ = left; }

	const query* right() const { return right_; }
	query* right() { return right_; }
	void right(query* right) { right_ = right; }

	int op() const { return __exp_op_.__op; }
	expression* exp() const { return __exp_op_.__exp; }
private:
	query* left_;
	query* right_;
	union {
		int __op;
		expression* __exp;
	} __exp_op_;
};
#endif
class index_data {
public:
	index_data(const fieldinfos& fis);
public:
	int add(const document* doc);
	int del(const document* doc);
	int update(const document* doc);
public:
	const fieldinfos& fis() const { return fis_; }
//	int search(hits& h, const query& q);
	
	template <typename _OSTR>
	void dump(_OSTR& out);
//private:
public:
	posting_table& get_posting_table(const char* name);
private:
	const fieldinfos& fis_;
	field_id_type count_;
	posting_table* pts_;
};

template <typename _OSTR>
void index_data::dump(_OSTR& out)
{
	for (field_id_type i = 0; i < count_; ++i) {
		out << i << " => ";
		pts_[i].dump(out);
	}
}

posting_table& index_data::get_posting_table(const char* name)
{
	field_id_type index = fis_.name2index(name);
	assert (index >= 0 && index < count_);
	return pts_[index];
}

index_data::index_data(const fieldinfos& fis)
	: fis_(fis),
	  count_(fis.count())
{
	pts_ = allocator.allocate<posting_table>(count_);
	if (pts_ == NULL) {
		throw std::runtime_error("out of memory in index_data ctr");
	}

	for (field_id_type i = 0; i < count_; ++i) {
		pts_[i].set_fieldinfo(&fis.get(i));
	}
}

int index_data::add(const document* doc)
{
	for (field_id_type i = 0; i < doc->field_count(); ++i) {
		const struct field_data& fd = doc->get(i);
		if (fd.data_ == NULL)
			continue;
		posting_table& pt = get_posting_table(fis_.get(fd.index_).name());

		// TODO: tokenize???
//std::cout << "add posting: " << fis_.get(i).name() << std::endl;
		if (pt.add(fd.data_, doc->id()) < 0) {
			return -1;
		}
	}

	return 0;
}
#if Q
int index_data::search(hits& h, const query& q)
{
	if  (q.left() == NULL && q.right() == NULL) {
		q.exp()->execute(h);
		return 0;
	}

	hits l, r;
	if (q.left() != NULL && search(l, *q.left()) < 0)
		return -1;
	if (q.right() != NULL && search(r, *q.right()) < 0)
		return -1;
	if (q.op() == OT_AND) {
		h.swap(l);
		h.set_intersection(r);
	}
	else if (q.op() == OT_OR) {
		h.swap(l);
		h.set_union(r);
	}
	else if (q.op() == OT_SUB) {
		h.swap(l);
		h.set_sub(r);
	}
	else {
		assert("invalid op!" ==  NULL);
		return -1;
	}

	return 0;
}
#endif
static int load_fieldinfos(fieldinfos& fis)
{
	xmlconfig config("./data.xml");
	xmlconfig::icontext ctx = config.get_context("/field-list/field");

	while (ctx != xmlconfig::end) {
		std::string id, type;
		int d_type;

		id = ctx.get_string("id", "UKN");
		type = ctx.get_string("type", "string");
		if (type == "string") {
			d_type = TDT_STRING;
		}
		else if  (type == "double") {
			d_type = TDT_DOUBLE;
		}
		else if (type == "long") {
			d_type = TDT_LONG;
		}
		else {
			d_type = TDT_UKN;
		}

		fis.add(fieldinfo(strdup(id.c_str()), d_type));
		++ctx;
	}

	return 0;
}

static doc_id_type id = 0;
static int load_document(index_data& index, const char* file)
{
	static struct timeval t1, t2;
	FILE* fp = fopen(file, "r");
	if (fp == NULL) {
		perror("fopen");
		return -1;
	}

std::cout << "loading ... " << file << std::endl;
if (t1.tv_sec == 0) gettimeofday(&t1, NULL);

	document* doc = allocator.allocate<document>();
	char line[8192];
	while (fgets(line, sizeof(line), fp) != NULL) {
		size_t len = strlen(line);
		while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r'))
			line[--len] = 0;
		if (len == 0) {
			if (doc != NULL) {
				doc->id(id++);
if (id % 1000 == 0) {
	gettimeofday(&t2, NULL);
	double ms = (t2.tv_sec - t1.tv_sec) * 1000.0 + (t2.tv_usec - t2.tv_usec) / 1000.0;
	if (ms < 1e-8) ms = 1e-8;
	std::cout << std::string(ctime(&t2.tv_sec), 24) << " add " << id << " docs, rate: " << (1000000.0 / ms) << "#/s" << std::endl;
	t1 = t2;
}
//std::cout << "********** add doc: " << doc->field_count() << std::endl;
				index.add(doc);
			}

			doc = allocator.allocate<document>();
		}
		else {
			char *vptr = strchr(line, ':');
			assert(vptr != NULL);
			std::string name(line, vptr - line);
			++vptr;	

			const fieldinfo* fi = index.fis().get(name.c_str());
			assert(fi != NULL);
			field_id_type id = fi->index();
			assert(id != NID);

			if (fi->data_type() == TDT_LONG) {
				long *l = allocator.allocate<long>();
				*l = strtol(vptr, NULL, 0);
//std::cout << "add long: " << name << " = " << *l << std::endl;
				doc->add(id, l);
			}
			else if (fi->data_type() == TDT_DOUBLE) {
				double *d = allocator.allocate<double>();
				*d = strtod(vptr,  NULL);
//std::cout << "add double: " << name << " = " << *d << std::endl;
				doc->add(id, d);
			}
			else if (fi->data_type() == TDT_STRING) {
				char *s = allocator.allocate<char>(len - (vptr - line) + 1);
				strcpy(s, vptr);
//std::cout << "add string: " << name << " = " << s << std::endl;
				doc->add(id, s);
			}
		}
	}

	return 0;
}

static void show_minfo()
{
	struct mallinfo mi = mallinfo();
	fprintf(stderr, "mallinfo: arena=%ld ordblks=%ld smblks=%ld hblks=%ld hblkhd=%ld usmblks=%ld fsmblks=%ld uordblks=%ld fordblks=%ld keepcost=%ld\n", (long)mi.arena, (long)mi.ordblks, (long)mi.smblks, (long)mi.hblks, (long)mi.hblkhd, (long)mi.usmblks, (long)mi.fsmblks, (long)mi.uordblks, (long)mi.fordblks, (long)mi.keepcost);
}

int main(int argc, char **argv)
{
	int ch;
	while ((ch = getopt(argc, argv, "l:n:h")) != EOF) {
		switch (ch) {
		case 'l':
			//posting_btree_traits::leafslots = atoi(optarg);
			break;
		case 'n':
			//posting_btree_traits::innerslots = atoi(optarg);
			break;
		case 'h':
			fprintf(stderr, "Usage: %s -l leaf-count -n inner-count file [...]\n", argv[0]);
			exit(1);
		}
	}

	struct timeval t1, t2;
	fieldinfos fis; //(5);
	load_fieldinfos(fis);

	index_data index(fis);

	gettimeofday(&t1, NULL);
	for (int i = optind; i < argc; ++i) {
		load_document(index, argv[i]);
	}

	gettimeofday(&t2, NULL);
	double ms = (t2.tv_sec - t1.tv_sec) * 1000.0 + (t2.tv_usec - t1.tv_usec) / 1000.0;
	std::cout << "total docs: " << id << ", average rate: " << (id * 1000.0 / ms) << "#/s." << std::endl;

	show_minfo();

//	index.dump(std::cout);
/*
	while ( 1 ) {
		std::cout << "please input expressions ([AND|OR|SUB] name {s|d|i} from to | quit): ";
		std::cout.flush();

		std::string line;
		std::getline(std::cin, line);

		if (line == "quit") break;

		query *q = NULL;
		expression* e2;

		int op;
		std::string n, d, f, t;

		const char* sep = " \t";
		char* nptr = strtok((char *)line.c_str(), sep);
		while (nptr != NULL) {
			if (q != NULL) {
				op = (*nptr == 'A') ? OT_AND : (*nptr == 'O' ? OT_OR : OT_SUB);
				if ((nptr = strtok(NULL, sep)) == NULL) break;
			}

			n = nptr; if ((nptr = strtok(NULL, sep)) == NULL) break;
			d = nptr; if ((nptr = strtok(NULL, sep)) == NULL) break;
			f = nptr; if ((nptr = strtok(NULL, sep)) == NULL) break;
			t = nptr; if ((nptr = strtok(NULL, sep)) == NULL) {}

			if (d == "s") {
				e2 = new expr_inclusive_between<std::string>(index.get_posting_table(n.c_str()),
						f, t);
			}
			else if (d == "d") {
				e2 = new expr_inclusive_between<double>(index.get_posting_table(n.c_str()),
						strtod(f.c_str(), NULL), strtod(t.c_str(), NULL));
			}
			else if (d == "i") {
				e2 = new expr_inclusive_between<double>(index.get_posting_table(n.c_str()),
						strtol(f.c_str(), NULL, 0), strtol(t.c_str(), NULL, 0));
			}

			query* q2 = new query(e2);
			if (q != NULL) {
				q = new query(q, q2, op);
			}
			else {
				q = q2;
			}
		}

		hits h;
		struct timeval t1, t2;
		
		gettimeofday(&t1, NULL);
		index.search(h, *q);
		gettimeofday(&t2, NULL);

		double ms = (t2.tv_sec - t1.tv_sec) * 1000000.0 + (t2.tv_usec - t1.tv_usec);
		std::cout << "search cost: " << (ms/1000.0) << "ms, matched: " << h.count() << std::endl << "\t";
		int j = 0;
		for (hits::ids_type::iterator iter = h.ids().begin(); iter != h.ids().end() && j < 10; ++iter, ++j)
			std::cout << " " << *iter;
		std::cout << std::endl;
	}
*/
	std::cout << "exit in 30 seconds ..." << std::endl;
	sleep(30);

	return 0;	
}
// usage:
// 	load fieldinfos 
//	load each document {
//	}
//	add-doc
//	optimize()???
//
//	load indices
//	query???
