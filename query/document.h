/* document.h
 *
**/
#ifndef __DOCUMENT__H
#define __DOCUMENT__H

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <string>
#include <vector>
#include <stdexcept>
#include <iomanip>
#include <tr1/unordered_map>
#include <tr1/unordered_set>
#include <sys/time.h>

#include <beyondy/mutex_cond.hpp>
#include <beyondy/singleton.hpp>
#include <beyondy/slab_cache.h>

#ifndef FIELD_ID_TYPE
#define FIELD_ID_TYPE 
typedef short field_id_type;
#endif /*! FIELD_ID_TYPE */

#ifndef DOC_ID_TYPE
#define DOC_ID_TYPE
typedef int doc_id_type;
#endif /*! DOC_ID_TYPE */

#define NID		-1

#define TDT_MASK	0x7
#define TDT_UKN		0x0
#define TDT_LONG	0x1
#define TDT_DOUBLE	0x2
#define TDT_STRING	0x3

class fieldnames {
public:
	fieldnames() : lock_(NULL) {}
public:
	const char* get(const char* n);
private:
	beyondy::thread_mutex lock_;

	typedef std::tr1::unordered_set<std::string> names_type;
	names_type names_;
};

class fieldinfo {
public:
	fieldinfo() : id_(NID), name_(NULL), flags_(0) { }
	fieldinfo(field_id_type id, const char* name, int flags) : id_(id), name_(singleton<fieldnames>::instance()->get(name)), flags_(flags) { }

	field_id_type id() const { return id_; }
	void id(field_id_type i) { id_ = i; }

	const char* name() const { return name_; }
	void name(const char* n) { name_ = singleton<fieldnames>::instance()->get(n); }

	int flags() const { return flags_; }
	void flags(int f) { flags_ = f; }

	int data_type() const { return flags_ & TDT_MASK; }
	void date_type(int f) { flags_ = (flags_ & ~TDT_MASK) | (f & TDT_MASK); }
private:
	field_id_type id_;
	const char* name_;	// come from fieldnames
	int flags_;
};

class fieldinfos {
public:
	fieldinfos();
	fieldinfos(field_id_type count);
public:
	field_id_type add(const fieldinfo& fi);
	field_id_type add(const char* name, int flags);
	field_id_type add(field_id_type id, const char* name, int flags);

	fieldinfo* get(int id);
	const fieldinfo* get(int id) const;

	fieldinfo* get(const char* name);
	const fieldinfo* get(const char* name) const;

	field_id_type n2i(const char* name) const;
	const char* i2n(field_id_type id) const;

	field_id_type size() const;
private:
	std::vector<fieldinfo> fields_;
	
	typedef std::tr1::unordered_map<std::string, field_id_type> nameid_map_t;
	nameid_map_t n2i_map_;
};

struct field_data {
	field_id_type id;
	const void* data;
};

struct field_data_id_comp {
	bool operator()(const field_data& x, const field_data& y) { return x.id < y.id; }
};

// document with fixed field count.
// but field's data is variable.
class fix_document {
public:
	fix_document(doc_id_type id, field_id_type count);
public:
	//int add(const char* name, const void* data);
	int add(field_id_type id, const void* data);

	struct field_data iterate(field_id_type i) const;

	struct field_data get(field_id_type id) const;
	//struct field_data get(const char* name) const;

	doc_id_type id() const { return id_; }
	void id(doc_id_type id) { id_ = id; }

	field_id_type size() const { return fields_.size(); }

	const void* field_data(field_id_type id) const;
	//const void* field_data(const char* name) const;
private:
	doc_id_type id_;

	typedef std::vector<const void*> fields_container;
	fields_container fields_;
};

class dyn_document {
public:
	dyn_document(doc_id_type id);
public:
	//int add(const char* name, const void* data);
	int add(field_id_type id, const void* data);

	struct field_data iterate(field_id_type i) const;

	struct field_data get(field_id_type id) const;
	//struct field_data get(const char* name) cosnt;

	doc_id_type id() const { return id_; }
	void id(doc_id_type id) { id_ = id; }

	field_id_type size() const { return fields_.size(); }

	const void* field_data(field_id_type id) const;
	//const void* field_data(const char* name) const;
private:
	doc_id_type id_;

	typedef std::vector<struct field_data> fields_container;
	fields_container fields_;
};

class document_helper {
public:
	document_helper(dyn_document* doc, const fieldinfos& fis);
	document_helper(fix_document* doc, const fieldinfos& fis);
public:
	int add(const char* name, const void *data);
	int add(field_id_type id, const void *data);

	struct field_data iterate(field_id_type i) const;

	struct field_data get(field_id_type id) const;
	struct field_data get(const char* name) const;

	doc_id_type id() const;
	void id(doc_id_type i);

	field_id_type size() const;

	const void* field_data(field_id_type id) const;
	const void* field_data(const char* name) const;
public:
	static void* allocate(int type, const void *ref);
	static void deallocate(int type, void *ptr);
private:
	fix_document* fix_doc_;
	dyn_document* dyn_doc_;
	const fieldinfos& fis_;
};

extern struct field_data null_fd;

#endif /*! __DOCUMENT__H */

