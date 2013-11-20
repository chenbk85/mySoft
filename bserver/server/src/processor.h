/* processor.h
 *
**/
#ifndef __PROCESSOR__H
#define __PROCESSOR__H

class MessageRequest;
class MessageResponse;

class processor {
public:
	virtual ~processor() {}
public:
	virtual int init() = 0;
	virtual void fini() = 0;
	virtual int process(int tid, MessageRequest& req, MessageResponse*& rsp) = 0;
public:
	int async_send(MessageResponse* rsp) { return (*async_fun_)(rsp, data_); }
	void set_async_fun(int (*f)(MessageResponse*, void*), void *data) { async_fun_ = f; data_ = data; }
private:

	int (*async_fun_)(MessageResponse*, void*);
	void *data_;
};

extern "C" processor* create();
extern "C" int destory(processor* proc);

#endif /* __PROCESSOR__H */

