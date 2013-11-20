

class message2lan {

public:
	generate();
}


int message2lan::generate(const messageinfo& mi, std::ostream& o)
{
	
}

template <typename _Tp, std::size_t _Nm>
struct var_array {
	size_t size;
	std::tr1::array<_Tp, _Nm> arr;
};

message IntradayDaily {
	optional int32 day;
	optional vararray[1440] IntradayPrice prices;
};

message IntradayPrice {
	mandatory int32 time;
	optional double open, close, high, low;
	optional double volume;
};


class IntradayDaily {
	bool isset_prices();
	var_array<IntradayPrice, 1440>& prices();
	var_array<IntradayPrice, 1440>& mutable_prices();
	const var_array<IntradayPrice, 1440>& prices() const;
	void prices(const var_array<IntradayPrice, 1440>& v) { prices = v; }
};

class IntradayPrice {
	bool isset_time();
	double& time();
	double& mutable_time();
	const double& time() const;
	void time(double v) { time = v; }
};

class xxxMessage {

public:
	bool isset_{field_name};
	field_type& {field_name}();
	field_type& mutable_{field_name}();
	const field_type& {field_name}() const;
	field_type& {field_name}(const field_type& n);

protected:
	// fields
	int i32;
	std::string str;
	var_array<type, N> fix_arr;
	var_array<type*, N> var_arr;
	std::vector<type*> dyn_arr;
protected:
	bitmap_t set_bits_[(max-ordinal + 1 + sizeof(bitmap_t) * 8 - 1) / (sizeof(bitmap_t) * 8)];
};

serialize(const class xxxMessage& md, const class messageinfo&  mi, MemoryBuffer& mb)
{
	mb.write_byte(TI_MSG);
	mb.write_byte(TI_INT32|); serialize(md.i32);

	// non POD
	mb.write_byte(TI_ARR|NO_POD);
	mb.write_byte(mi.fix_arr.size);
	for (int i = 0; i < mi.fix_arr.size; ++i) {
		mb.write_byte(mi.fis[5].id);
		serialize();//mb.write_byte(mi.fis[5].arr_tinfo->id);
		
	}
	
	// POD
	mb.write_byte(TI_ARR|POD);
	// write header
	memcpy(..., md.fix_arr.arr.data(), md.fix_arr.size() * sizeof(md.fix_arr.arr[0]));
}

deserialize(const MemoryBuffer& mb, class xxxMessage& md, const MessageInfo& mi)
{
	
}

getInt32(const MemoryBuffer& mb, const std::string field, const class messageinfo& mi, int* idx)
{
	// input must be a message or struct

	int id = mi.field(field.c_str());
	// find the offset of id in mb
	//	start *idx+1
		cnt, fixed16_id[cnt]fix32_offset[cnt] type data...
		var16_id type data...
	// if ok
	// skip id
	switch (type) {
	case TI_INT8/16/32/64:
		read it out and conver to 32;
		break;
	case TI_FLOAT/TI_DOUBLE:
		read it out and convert to int32;
		break;
	case TI_STRING:
		errno = EINVAL; return 0;
	case TI_ARRAY:
	...
		errno = EINVAL; return 0;
	}
}

getInt64/getInt8/getInt16/getFloat32/getFloat64
getString
getData()
getObject()

