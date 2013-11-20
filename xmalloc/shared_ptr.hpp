
template <typename Tp>
class shared_ptr {

public:
	shared_ptr() : objptr_(0), refcnt_(0)
	{
		// nothing
	}
	shared_ptr(Tp* objptr) : objptr_(objptr), refcnt_(1)
	{
		// nothing
	}
	shared_ptr(const shared_ptr<Tp>& x) : objptr_(x.objptr_), refcnt_(x.refcnt_)
	{
		if (refcnt_ > 0) {
			++refcnt_;
			++const_cast<shared_ptr<Tp>&>(x).refcnt_;
		}
	}
	~shared_ptr()
	{
		 if (refcnt_ > 0 && --refcnt_ == 0) delete objptr_;
	}

	shared_ptr<Tp>& operator=(shared_ptr<Tp>& x) {
		if (x.refcnt_ > 0) ++x.refcnt_;
		this.objptr_ = x.objptr_;
		this.refcnt_ = x.refcnt_;
		return *this;
	}

	Tp* operator*() { return objptr_; }
	const Tp* operator*() const { return objptr_; }

	template <typename out>
	friend out& operator<<(out& o, const shared_ptr<Tp>& x)
	{
		o << "[" << x.refcnt_ << "]: " << (void *)x.objptr_;
		return o;
	}
private:
	Tp *objptr_;
	long refcnt_;
};
