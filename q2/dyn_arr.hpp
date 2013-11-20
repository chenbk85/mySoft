template <typename SIZE_TYPE, typename Tp>
class dyn_array {
public:
	typedef SIZE_TYPE size_type;
	typedef Tp data_type;

	dyn_array()
		: count_(0), allocated_(0), data_(NULL)
	{
		// nothing
	}
	
	dyn_array(size_type count)
		: count_(count), allocated_(count)
	{
		data_ = new data_type[count_];
	}

	dyn_array(const dyn_array& da)
	{
		if (this != &da) {
			this->reset(da.count_, da.allocated_, da.data_);
			const_cast<dyn_array&>(da).reset(0, 0, NULL);
		}
	}

	dyn_array<SIZE_TYPE, Tp>& operator=(const dyn_array<SIZE_TYPE, Tp>& da)
	{
		if (this != &da) {
			this->reset(da.count_, da.allocated_, da.data_);
			const_cast<dyn_array&>(da).reset(0, 0, NULL);
		}

		return *this;
	}

	~dyn_array()
	{
		if (data_ != NULL)
			delete[] data_;
	}

	size_type count() const
	{
		return count_;
	}
	
	data_type* data() const
	{
		return data_;
	}

	data_type& operator[](int index) 
	{
		assert(index >= 0 && index < count_);
		return data_[index];
	}
	
	const data_type& operator[](int index) const
	{
		assert(index >= 0 && index < count_);
		return data_[index];
	}

	int push_back(const Tp& v)
	{
		if (count_ == allocated_) {
			
			// TODO: how to effectly expand??
			
			size_type ncount = (count_ < 1) ? 1 : (count_ * 2);
			Tp* ndata = new data_type[ncount];
			if (ndata == NULL)
				return -1;

			for (size_type i = 0; i < count_; ++i) {
				ndata[i] = data_[i];
			}

			if (data_ != NULL) {
				delete [] data_;
			}

			allocated_ = ncount;
			data_ = ndata;
		}

		data_[count_++] = v;
		return  0;
	}

	int insert(size_type pos)
	{
		if (count_ == allocated_) {

			// TODO: how to effectly expand??
			
			size_type ncount = (count_ < 1) ? 1 : (count_ * 2);
			Tp* ndata = new data_type[ncount];
			if (ndata == NULL)
				return -1;

			for (size_type i = 0; i < pos; ++i) {
				ndata[i] = data_[i];
			}

			for (size_type i = count_; i > pos; --i) {
				ndata[i] = data_[i - 1];
			}
			
			if (data_ != NULL) {
				delete[] data_;
			}

			allocated_ = ncount;
			data_ = ndata;
		}
		else for (size_type i = count_; i > pos; --i) {
			data_[i] = data_[i - 1];
		}

		++count_;
		return 0;
	}

	int insert(size_type pos, const Tp& v)
	{
		if (insert(pos) < 0)
			return 0;
		data_[pos] = v;
		return 0;
	}

	void reset(size_type count, size_type allocated, data_type* data)
	{
		count_ = count;
		allocated_ = allocated;
		data_ = data;
	}

	int reserve(size_type capacity)
	{
		if (capacity < count_) {
			assert("can not reserve to less than the valid count!" == NULL);
			return -1;
		}

		if (allocated_ == capacity) {
			return 0;	// do nothing
		}
		
		// re-set anyway
		Tp* ndata = new data_type[capacity];
		if (ndata == NULL)
			return -1;

		for (size_type i = 0; i < count_; ++i) {
			ndata[i] = data_[i];
		}

		if (data_ != NULL) {
			delete[] data_;
		}

		data_ = ndata;
		allocated_ = capacity;
		return 0;
	}

	template <typename CMP>
	data_type* qsearch(const data_type& val, size_type& pos)
	{
		size_type i = 0, j = count_;
		CMP cmp;

		while (i < j) {
			size_type m = (i + j) / 2;
			int retval = cmp(data_[m], val);
			if (retval > 0) {
				j = m;
			}
			else if (retval < 0) {
				i = m + 1;
			}
			else {
				pos = m;
				return &data_[m];
			}
		}

		pos = i;
		return NULL;
	}

	template <typename CMP>
	const data_type* qsearch(const data_type& val, size_type& pos) const
	{
		size_type i = 0, j = count_;
		CMP cmp;

		while (i < j) {
			size_type m = (i + j) / 2;
			int retval = cmp(data_[m], val);
			if (retval > 0) {
				j = m;
			}
			else if (retval < 0) {
				i = m + 1;
			}
			else {
				pos = m;
				return &data_[m];
			}
		}

		pos = i;
		return NULL;
	}
private:
	size_type count_;
	size_type allocated_;
	data_type* data_;
};


