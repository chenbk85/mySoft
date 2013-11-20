#ifndef _MEMORYSTREAM_H_
#define _MEMORYSTREAM_H_

#include <sstream>
#include <byteswap.h>
#include <arpa/inet.h>

#include "mstar_protocol.h"

class MemoryStream {
public:
	// encode
	MemoryStream(): rptr_(0), wptr_(0), capacity_(0), auto_delete_(false), data_(NULL)
	{
	};
	MemoryStream(size_t size)
		: rptr_(0), wptr_(0), capacity_(size), auto_delete_(true)
	{
		data_ = new unsigned char[size];
	}

	// for decode
	MemoryStream(unsigned char* data, size_t size, bool auto_delete = false, bool decode = true)
		: rptr_(0), wptr_(size), capacity_(size), auto_delete_(auto_delete), data_(data)
	{
		if (!decode)
		{
			wptr_ = 0;
		}
	}
	
	~MemoryStream()
	{
		if (auto_delete_ && data_ != NULL)
		{
			delete [] data_;
			data_ = NULL;
		}
	}
public:
	void auto_delete(bool ad) { auto_delete_ = ad; }
	bool auto_delete() const { return auto_delete_; }

	void reset_data(unsigned char* data, size_t size, bool auto_delete = false)
	{
		if (auto_delete_ && data_ != NULL)
		{
			delete [] data_;
			data_ = NULL;
		}
		rptr_ = 0;
		wptr_ = 0;
		capacity_ = size;
		auto_delete_ = auto_delete;
		data_ = data;
	}
	
	unsigned char* data() const { return data_ + rptr_; }
	size_t size() const { return wptr_ - rptr_; }
	size_t capacity() const { return capacity_; }
public:
	bool write_check(int size){
		if (data_ == NULL || wptr_ + size > capacity_){
			return false;
		}

		return true;
	}
	
	bool read_check(int size){
		if (data_ == NULL || rptr_ + size > capacity_){
			return false;
		}

		return true;
	}
	
	
	bool write(unsigned char val){
		if (!write_check(1))return false;
		*(uint8_t *)(data_ + wptr_) = val ;
		wptr_ += sizeof(val);
		
		return true;
	}

	bool write(unsigned short val){
		if (!write_check(sizeof(val)))return false;
		*(uint16_t *)(data_ + wptr_) = htons(val);
		wptr_ += sizeof(val);
		
		return true;
	}
	
	bool write(unsigned int val){
		if (!write_check(sizeof(val)))return false;
		*(uint32_t *)(data_ + wptr_) = htonl(val);
		wptr_ += sizeof(val);
		
		return true;
	}
	
	bool write(unsigned long val){
		if (!write_check(sizeof(val)))return false;
		
		uint64_t uint64_val = 0;
		
		memcpy(&uint64_val, &val, sizeof(val));
		uint64_val = hton64(uint64_val);
		
		memcpy(data_ + wptr_, &uint64_val, sizeof(uint64_val));
		wptr_ += sizeof(val);
		return true;
	}
	
	bool write(float val){
		if (!write_check(sizeof(val)))return false;
		uint32_t long_val = 0;
		memcpy(&long_val, &val, sizeof(val));

		return write(long_val);
	}

	bool write(long long val){
		if (!write_check(sizeof(val)))return false;
		uint64_t long_long_val = 0;
		memcpy(&long_long_val, &val, sizeof(val));

		return write(long_long_val);
	}

	bool write(unsigned long long val){
		if (!write_check(sizeof(val)))return false;

		uint64_t uint64_val = 0;
		
		memcpy(&uint64_val, &val, sizeof(val));
		uint64_val = hton64(uint64_val);
		
		memcpy(data_ + wptr_, &uint64_val, sizeof(uint64_val));
		wptr_ += sizeof(val);

		return true;
	}
	bool write(double val){
		if (!write_check(sizeof(val)))return false;
		//*(uint64_t *)(data_ + wptr_) = hton64(*(uint64_t *)&val);

		uint64_t uint64_val = 0;
		
		memcpy(&uint64_val, &val, sizeof(val));
		uint64_val = hton64(uint64_val);
		
		memcpy(data_ + wptr_, &uint64_val, sizeof(uint64_val));
		wptr_ += sizeof(val);
		
		return true;
	}
	
	bool write(const unsigned char *val, unsigned int len){
		if (!write(len)) return false;
		if (!write_check(len)) return false;
		
		memcpy((void *)(data_ + wptr_), val, len);

		wptr_ += len;
		return true;
	}
	
	bool write(const std::string& val){
		return write((unsigned char *)val.c_str(), val.size() + 1);
	}

	bool read(unsigned char& val)
	{
		if (!read_check(sizeof(val)))return false;
		val = *(uint8_t *)(data_ + rptr_);
		rptr_ += sizeof(val);
		return true;
	}
	
	bool read(unsigned short& val){
		if (!read_check(sizeof(val)))return false;
		
		val = ntohs(*(uint16_t *)(data_ + rptr_));
		rptr_ += sizeof(val);
		return true;
	}
	
	bool read(unsigned int& val){
		if (!read_check(sizeof(val)))return false;
		val = ntohl(*(uint32_t *)(data_ + rptr_));
		rptr_ += sizeof(val);
		
		return true;
	}
	
	bool read(unsigned long& val){
		if (!read_check(sizeof(val)))return false;
		*(uint64_t *)&val = ntoh64(*(uint64_t *)(data_ + rptr_));
		rptr_ += sizeof(val);
		return true;
	}

	bool read(long long& val){
		if (!read_check(sizeof(val)))return false;
		*(uint64_t *)&val = ntoh64(*(uint64_t *)(data_ + rptr_));
		rptr_ += sizeof(val);
		return true;
	}
	
	bool read(float& val){
		unsigned int v = 0;
		if (!read(v)) return false;
		*(uint32_t *)&val = v;
		return true;
	}
	
	bool read(double& val){
		if (!read_check(sizeof(val)))return false;
		*(uint64_t *)&val = ntoh64(*(uint64_t *)(data_ + rptr_));
		rptr_ += sizeof(val);
		return true;
	}
	
	bool read(unsigned char *&val, unsigned int& len){
		if (!read(len)) return false;
		if (!read_check(len)) return false;
		
		memcpy(val, data_ + rptr_,  len);

		rptr_ += len;
		return true;
	}
	
	bool read(char *&val, unsigned int& len){
		if (!read(len)) return false;
		
		if (!read_check(len)) return false;
		
		memcpy(val, data_ + rptr_,  len);

		rptr_ += len;
		return true;
	}
	
	bool read(std::string& val){
		unsigned int len;
		if (!read(len)) return false;
		if (!read_check(len)) return false;
		val.assign((char *)(data_ + rptr_), len - 1);
		rptr_ += len;
		
		return true;
	}
	
private:
	size_t rptr_;
	size_t wptr_;
	size_t capacity_;
	bool auto_delete_;
	unsigned char* data_;
};

static inline bool Serialize(MemoryStream& stream, unsigned char val) 
{
	return stream.write(val);
}

static inline bool Serialize(MemoryStream& stream, char val)
{
	return Serialize(stream, (unsigned char)val);
}

static inline bool Serialize(MemoryStream& stream, bool val) 
{
	unsigned char v = val ? 1 : 0;
	return Serialize(stream, v);
}

static inline bool Serialize(MemoryStream& stream, unsigned short val) 
{
	return stream.write(val);
}

static inline bool Serialize(MemoryStream& stream, short val)
{
	return Serialize(stream, (unsigned short *)val);
}

static inline bool Serialize(MemoryStream& stream, unsigned int val) 
{
	return stream.write(val);
}

static inline bool Serialize(MemoryStream& stream, int val)
{
	return Serialize(stream, (unsigned int)val);
}

static inline bool Serialize(MemoryStream& stream, unsigned long val) 
{
	return stream.write(val);
}

static inline bool Serialize(MemoryStream& stream, long val)
{
	return Serialize(stream, (unsigned long)val);
}

static inline bool Serialize(MemoryStream& stream, float val) 
{
	return stream.write(val);
}

static inline bool Serialize(MemoryStream& stream, double val) 
{
	return stream.write(val);
}

static inline bool Serialize(MemoryStream& stream, long long val) 
{
	return stream.write(val);
}

static inline bool Serialize(MemoryStream& stream, const std::string& val) 
{
	return stream.write(val);
}

static inline bool Serialize(MemoryStream& stream, const unsigned char* val, const unsigned int length) 
{
	return stream.write(val, length);
}


static inline bool Serialize(MemoryStream& stream, const char* val, const unsigned int length) 
{
	return stream.write((const unsigned char*)val, length);
}

static inline bool Deserialize(MemoryStream& stream, unsigned char& val) 
{
	return stream.read(val);
}

static inline bool Deserialize(MemoryStream& stream, bool& val) 
{
	unsigned char v;
	if (!Deserialize(stream, v))return false;
	val = (v != 0 ? true : false);
	return true;
}

static inline bool Deserialize(MemoryStream& stream, char& val) 
{
	unsigned char v;
	if (!Deserialize(stream, v))return false;
	val = (char)v;
	return true;
}

static inline bool Deserialize(MemoryStream& stream, unsigned short& val) 
{
	return stream.read(val);
}

static inline bool Deserialize(MemoryStream& stream, short& val) 
{
	unsigned short v;
	if (!Deserialize(stream, v))return false;
	val = (short)v;
	return true;
}

static inline bool Deserialize(MemoryStream& stream, unsigned int& val) 
{
	return stream.read(val);
}

static inline bool Deserialize(MemoryStream& stream, int& val) 
{
	unsigned int v;
	if (!Deserialize(stream, v))return false;
	val = (int)v;
	return true;
}

static inline bool Deserialize(MemoryStream& stream, unsigned long& val) 
{
	return stream.read(val);
}

static inline bool Deserialize(MemoryStream& stream, long& val) 
{
	unsigned long v;
	if (!Deserialize(stream, v))return false;
	val = (long)v;
	return true;
}

static inline bool Deserialize(MemoryStream& stream, float& val) 
{
	return stream.read(val);
}

static inline bool Deserialize(MemoryStream& stream, double& val) 
{
	return stream.read(val);
}

static inline bool Deserialize(MemoryStream& stream, std::string& val) 
{
	return stream.read(val);
}

static inline bool Deserialize(MemoryStream& stream, long long& val) 
{
	return stream.read(val);
}

static inline bool Deserialize(MemoryStream& stream, unsigned char* val, unsigned length) 
{
	return stream.read(val, length);
}

static inline bool Deserialize(MemoryStream& stream, char* val, unsigned length) 
{
	return stream.read(val, length);
}
#endif
