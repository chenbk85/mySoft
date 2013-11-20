#include <beyondy/slab_cache_t.hpp>
#include "document.h"

struct field_data null_fd = { NID, NULL };

const char* fieldnames::get(const char* n)
{
	assert(n != NULL);

	names_type::iterator iter = names_.find(n);
	if (iter == names_.end()) {
		beyondy::auto_lock<typeof lock_> al(lock_);

		if ((iter = names_.find(n)) == names_.end()) {
			names_.insert(n);
			iter = names_.find(n);
			assert(iter != names_.end());
		}
	}

	return iter->c_str();
}

fieldinfos::fieldinfos()
{
	// nothing
}

fieldinfos::fieldinfos(field_id_type count)
{
	fields_.reserve(count);
}

field_id_type fieldinfos::add(const fieldinfo& fi)
{
	if (fi.id() == NID) {
		return add(fi.name(), fi.flags());
	}
	else if (fi.id() == fields_.size()) {
		fields_.push_back(fi);
		n2i_map_[fi.name()] = fi.id();
	}
	else {
		fields_.resize(fi.id() + 1);
		fields_[fi.id()] = fi;
		n2i_map_[fi.name()] = fi.id();
	}

	return fi.id();
}

field_id_type fieldinfos::add(const char* name, int flags)
{
	fieldinfo fi(fields_.size(), name, flags);
	fields_.push_back(fi);
	n2i_map_[fi.name()] = fi.id();
	return fi.id();
}

field_id_type fieldinfos::add(field_id_type id, const char* name, int flags)
{
	if (id == NID) {
		return add(name, flags);
	}

	fieldinfo fi(id, name, flags);
	return add(fi);
}

fieldinfo* fieldinfos::get(int id)
{
	if (id >= 0 && id < fields_.size())
		return &fields_[id];
	return NULL;
}

const fieldinfo* fieldinfos::get(int id) const
{
	if (id >= 0 && id < fields_.size())
		return &fields_[id];
	return NULL;
}

fieldinfo* fieldinfos::get(const char* name)
{
	field_id_type id = n2i(name);
	if (id == NID)
		return NULL;
	return &fields_[id];
}

const fieldinfo* fieldinfos::get(const char* name) const
{
	field_id_type id = n2i(name);
	if (id == NID)
		return NULL;
	return &fields_[id];
}

field_id_type fieldinfos::n2i(const char* name) const
{
	nameid_map_t::const_iterator iter = n2i_map_.find(name);
	if (iter != n2i_map_.end())
		return iter->second;
	return NID;
}

const char* fieldinfos::i2n(field_id_type id) const
{
	if (id >= 0 && id < fields_.size()) {
		return fields_[id].name();
	}

	return NULL;
}

field_id_type fieldinfos::size() const
{
	return (field_id_type)fields_.size();
}

fix_document::fix_document(doc_id_type id, field_id_type count) : id_(id), fields_(count)
{
	for (fields_container::iterator iter = fields_.begin();
		iter != fields_.end();
			++iter) {
		*iter = NULL;
	}
}

int fix_document::add(field_id_type id, const void* data)
{
	assert(id >= 0 && id < fields_.size());
	fields_[id] = data;	// must be NULL before?
	return 0;
}

struct field_data fix_document::iterate(field_id_type i) const
{
	assert (i >= 0 && i < fields_.size());
	struct field_data fd = { i, fields_[i] };
	return fd;
}

struct field_data fix_document::get(field_id_type id) const
{
	assert(id >= 0 && id < fields_.size());
	struct field_data fd = { id, fields_[id] };
	return fd;
}

const void* fix_document::field_data(field_id_type id) const
{
	assert(id >= 0 && id < fields_.size());
	return fields_[id];
}
	
dyn_document::dyn_document(doc_id_type id) : id_(id)
{
	// nothing
}

int dyn_document::add(field_id_type id, const void* data)
{
	struct field_data k = { id, data };
	fields_container::iterator iter = std::lower_bound(fields_.begin(), fields_.end(), k, field_data_id_comp());
	if (iter == fields_.end()) {
		fields_.push_back(k);
	}
	else if (iter->id == id) {
		iter->data = data;	// free old?
	}
	else {
		fields_.insert(iter, k);
	}

	return 0;
}

struct field_data dyn_document::iterate(field_id_type i) const
{
	assert(i >= 0 && i < fields_.size());
	return fields_[i];
}

struct field_data dyn_document::get(field_id_type id) const
{
	struct field_data k = { id, NULL };
	fields_container::const_iterator iter = std::lower_bound(fields_.begin(), fields_.end(), k, field_data_id_comp());
	if (iter == fields_.end()) {
		return null_fd;
	}
	else if (iter->id == id) {
		return *iter;
	}
	else {
		return null_fd;
	}
}

const void* dyn_document::field_data(field_id_type id) const
{
	struct field_data fd = get(id);
	return (fd.id == NID) ? NULL : fd.data;
}

document_helper::document_helper(fix_document* doc, const fieldinfos& fis)
	: fix_doc_(doc), dyn_doc_(NULL), fis_(fis)
{
	// nothing
}

document_helper::document_helper(dyn_document* doc, const fieldinfos& fis)
	: fix_doc_(NULL), dyn_doc_(doc), fis_(fis)
{
	// nothing
}

int document_helper::add(const char* name, const void *data)
{
	const fieldinfo* fip = fis_.get(name);
	if (fip == NULL)
		return -1;

	void *ndata = document_helper::allocate(fip->data_type(), data);
	if (ndata == NULL)
		return -1;	

	if (fix_doc_ != NULL)
		return fix_doc_->add(fip->id(), ndata);
	else if (dyn_doc_ != NULL)
		return dyn_doc_->add(fip->id(), ndata);

	return -1;
}

int document_helper::add(field_id_type id, const void *data)
{
	const fieldinfo* fip = fis_.get(id);
	if (fip == NULL)
		return -1;

	void *ndata = document_helper::allocate(fip->data_type(), data);
	if (ndata == NULL)
		return -1;	

	if (fix_doc_ != NULL)
		return fix_doc_->add(id, ndata);
	else if (dyn_doc_ != NULL)
		return dyn_doc_->add(id, ndata);

	return -1;
}

struct field_data document_helper::iterate(field_id_type i) const
{
	if (fix_doc_ != NULL)
		return fix_doc_->iterate(i);
	else if (dyn_doc_ != NULL)
		return dyn_doc_->iterate(i);
	return null_fd;
}

struct field_data document_helper::get(field_id_type id) const
{
	if (fix_doc_ != NULL)
		return fix_doc_->get(id);
	else if (dyn_doc_ != NULL)
		return dyn_doc_->get(id);
	return null_fd;
}

struct field_data document_helper::get(const char* name) const
{
	field_id_type id = fis_.n2i(name);
	if (fix_doc_ != NULL)
		return fix_doc_->get(id);
	else if (dyn_doc_ != NULL)
		return dyn_doc_->get(id);
	return null_fd;
}

doc_id_type document_helper::id() const
{
	if (fix_doc_ != NULL)
		return fix_doc_->id();
	else if (dyn_doc_ != NULL)
		return dyn_doc_->id();
	return -1;
}

void document_helper::id(doc_id_type id)
{
	if (fix_doc_ != NULL)
		fix_doc_->id(id);
	else if (dyn_doc_ != NULL)
		dyn_doc_->id(id);
	return;
}

field_id_type document_helper::size() const
{
	if (fix_doc_ != NULL)
		return fix_doc_->size();
	else if (dyn_doc_ != NULL)
		return dyn_doc_->size();
	return NID;
}
	
const void* document_helper::field_data(field_id_type id) const
{
	if (fix_doc_ != NULL)
		return fix_doc_->field_data(id);
	else if (dyn_doc_ != NULL)
		return dyn_doc_->field_data(id);
	return NULL;
}
const void* document_helper::field_data(const char* name) const
{
	field_id_type id = fis_.n2i(name);
	if (fix_doc_ != NULL)
		return fix_doc_->field_data(id);
	else if (dyn_doc_ != NULL)
		return dyn_doc_->field_data(id);
	return NULL;
}

void* document_helper::allocate(int type, const void *ref)
{
	void *ptr;
	size_t size;

	switch (type) {
	case TDT_LONG:
		size = sizeof(long);
		ptr = slab_object<long>::instance()->allocate();
		break;
	case TDT_DOUBLE:
		size = sizeof(double);
		ptr = slab_object<double>::instance()->allocate();
		break;
	case TDT_STRING:
		size = (ref == NULL) ? 0 : strlen((const char *)ref) + 1;
		ptr = slab_malloc(size);
		break;
	default:
		break;
	}

	memcpy(ptr, ref, size);
	return ptr;	
}

void document_helper::deallocate(int type, void *ptr)
{
	switch (type) {
	case TDT_LONG:
		slab_object<long>::instance()->destroy();
		break;
	case TDT_DOUBLE:
		slab_object<double>::instance()->destroy();
		break;
	case TDT_STRING:
		slab_free(ptr);
		break;
	default:
		assert(0);
		break;
	}

	return;
}

