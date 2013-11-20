/* protocol.cpp
 *
**/
#include <errno.h>
#include "protocol.h"

protocol::proto_map_t protocol::proto_maps;

int protocol::_register(const char* name, protocol* proto)
{
	assert (name != NULL && proto != NULL);
	std::string _n(name);

	proto_map_iter_t iter = proto_maps.find(_n);
	if (iter == proto_maps.end()) {
		proto_maps.insert(std::make_pair(_n, proto));
		return 0;
	}
	else {
		errno = EEXIST;
		return -1;
	}
}

int protocol::unregister(const char* name)
{
	assert(name != NULL);
	std::string _n(name);

	proto_map_iter_t iter = proto_maps.find(_n);
	if (iter != proto_maps.end()) {
		// TODO: check refcnt
		// TODO: delete it?
		proto_maps.erase(iter);
	}
	else {
		errno = EINVAL;
		return -1;
	}

	return 0;
}

protocol* protocol::get(const char* name)
{
	assert(name != NULL);
	std::string _n(name);

	proto_map_iter_t iter = proto_maps.find(_n);
	if (iter != proto_maps.end()) {
		// TODO: check refcnt
		return iter->second;
	}
	else {
		errno = EINVAL;
		return NULL;
	}
}

