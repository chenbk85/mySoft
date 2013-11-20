#ifndef __PROTO__H16__H
#define __PROTO__H16__H

#include <cstdlib>
#include <numeric>
#include <limits>

#include "protocol.h"
#include "message_block.h"

struct proto_h16_head {
	uint32_t len;
	uint16_t cmd;
	uint16_t ver;
	uint32_t syn;
	uint32_t ack;
};

struct proto_h16_res : public proto_h16_head {
	int32_t ret_;
};

class proto_h16 : public protocol {
public:
	proto_h16(size_t min = 0, size_t max = std::numeric_limits<ssize_t>::max())
		: min_(min), max_(max)
	{}
public:
	virtual ssize_t calc_size(MessageBlock& mb) const;
	virtual size_t head_size() const { return sizeof(struct proto_h16_head); }

	static void _register();
	static void unregister();
private:
	size_t min_;
	size_t max_;
};

#endif /*! __PROTO__H16__H */

