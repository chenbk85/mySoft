/* protocol.h
 *
**/
#ifndef __PROTOCOL__H
#define __PROTOCOL__H

#include <map>
#include <string>

#include "message_block.h"

class protocol {
public:
	virtual ~protocol() { /* nothing */ }
public:
	// > 0: message's size;
	// = 0: message is not large enough to determine the whole size
	// < 0: something wrong, and set errno
	virtual ssize_t calc_size(MessageBlock& mb) const = 0; 
	virtual size_t head_size() const = 0;

	static int _register(const char* name, protocol* proto);
	static int unregister(const char* name);
	static protocol* get(const char* name);
private:
	typedef std::map<std::string, protocol *> proto_map_t;
	typedef proto_map_t::iterator proto_map_iter_t;
	static proto_map_t proto_maps;
};

#endif /*! __PROTOCO__H */
