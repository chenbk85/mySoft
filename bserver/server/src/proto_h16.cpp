#include <errno.h>

#include "proto_h16.h"

ssize_t proto_h16::calc_size(MessageBlock& mb) const
{
	uint32_t sz;
	if (mb.size() < sizeof(sz)) {
		size_t left = sizeof(sz) - mb.size();
		memcpy(&sz, mb.rptr(), mb.size());

		MessageBlock* cont = mb.cont();
		if (cont != NULL && cont->size() >= left) {
			memcpy((byte *)&sz + mb.size(), cont->rptr(), left);
		}
		else {
			return 0;
		}
	}
	else {
		memcpy(&sz, mb.rptr(), sizeof(sz));
	}

	if (sz >= head_size() && sz >= min_ && sz <= max_)
		return sz;

	errno = EINVAL;
	return -1;
}

//static void _register(void) __attribute__((constructor));
//static void unregister() __attribute__((destructor));

static const char* H16NV = "H16/1.0";
static proto_h16 h16;

void proto_h16::_register()
{
	protocol::_register(H16NV, &h16);
}

void proto_h16::unregister()
{
	protocol* proto = protocol::get(H16NV);
	protocol::unregister(H16NV);
	delete proto;
}

