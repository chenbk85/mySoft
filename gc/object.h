#include <cassert>

class object {
public:
	object() : _refcnt(1) {}
	virtual ~object() {}
public:
	long addref() { ++_refcnt; assert(_refcnt > 1); }
	long delref() { assert(_refcnt >= 1); if (--_refcnt == 0) delete this; }
	long refcnt() const { assert(_refcnt >= 1); return _refcnt; }
private:
	long _refcnt;
};

template <typename _Tp>
class _object_ptr {
public:
	_object_ptr() : _objptr(0) {
		debug("ctr()");
	}
	~_object_ptr() {
		debug("~ctr()");
		if (_objptr) _objptr->delref();
	}
public:
	template <typename _Tp2>
	_object_ptr<_Tp>(_Tp2* p) : _objptr(p) {
		debug(std::string("ctr(") + typeid(*p).name() + ")");
	}

	_object_ptr<_Tp>(const _object_ptr<_Tp>& p) : _objptr(p.objptr()) {
		debug(std::string("cc()"));
		if (_objptr) _objptr->addref();
	}

	template <typename _Tp2>
	_object_ptr<_Tp>(const _object_ptr<_Tp2>& p) : _objptr(p.objptr()) {
		debug(std::string("cc(") + typeid(_Tp2).name() + ")");
		if (_objptr) _objptr->addref();
	}

	_object_ptr<_Tp>& operator=(const _object_ptr<_Tp>& p) {
		debug(std::string("operator="));
		if (&p != this && _objptr != p.objptr()) { 
			if (_objptr != 0) _objptr->delref();
			if ((_objptr = p.objptr()) != 0) _objptr->addref();
		}

		return *this;
	}

	template <typename _Tp2>
	_object_ptr<_Tp>& operator=(const _object_ptr<_Tp2>& p) {
		debug(std::string("operator=(") + typeid(_Tp2).name() + ")");
		if (_objptr != 0) _objptr->delref();
		if ((_objptr = p.objptr()) != 0) _objptr->addref();
		return *this;
	}
public:
	_Tp* objptr() const { return _objptr; }
	_Tp* objptr() { return _objptr; }
public:
	_Tp* operator->() { return _objptr; }
	const _Tp* operator->() const { return _objptr; }
public:
	void debug(const std::string& op) const {
		std::cout << op << " --- type-id: " << typeid(_Tp).name() << ", addr: " << (void *)_objptr;
		if (_objptr != NULL) std::cout << ", refcnt: " << _objptr->refcnt();
		std::cout << std::endl;
	}
private:
	_Tp *_objptr;
};

typedef _object_ptr<object> object_ptr;
