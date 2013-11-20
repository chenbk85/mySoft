#include "connector.h"

int ActiveConnection::on_writable()
{
	int retval;
	if (mrsp_ == NULL) {
		// check error
		// fall throught
	}
	else if ((retval = send_current_response()) != 0) {
		return retval;
	}

	// TODO: why loop here
	while (true) {
		MessageResponse* rsp = connector_->PendingQueue()->get(0);
		if (rsp != NULL) {
			mrsp_ = rsp;
			mbs_ = mrsp_;

			if ((retval = send_current_response()) == 0) {
				continue;
			}
			else {
				break;
			}
		}
		else {
			emgr_->mod_poller(this, EPOLLIN);
			break;
		}
	}

	return 0;
}

void ActiveConnection::destory()
{
	assert(connector_ != NULL);
	connector_->destory(this);
}
