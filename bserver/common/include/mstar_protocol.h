#ifndef __MSTAR_PROTOCOL__H
#define __MSTAR_PROTOCOL__H

#if defined WIN32
#include <assert.h>
#include <new>
#include <stdexcept>
#include <algorithm>
#include <Winsock2.h>


#define __WORDSIZE          32
#define ENOMEM              -1
#define EINVAL              -1
#else
#include <sys/types.h>
#include <stdint.h>
#include <assert.h>
#include <errno.h>
#include <string>
#include <endian.h>
#include <byteswap.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdexcept>
#include <string.h>
#endif

#if defined WIN32
typedef char                int8_t;
typedef unsigned char       uint8_t;
typedef short               int16_t;
typedef unsigned short      uint16_t;
typedef int                 int32_t;
typedef unsigned int        uint32_t;
typedef long long           int64_t;
typedef unsigned long long  uint64_t;
#endif

#define MP_VERSION_1	    1

//typedef long double uint128_t;

typedef struct 
{
    unsigned long long high_;
    unsigned long long low_;
} uint128_t;
    
static inline bool operator==(const uint128_t& left, const uint128_t& right)
{
    return (left.high_ == right.high_) && (left.low_ == right.low_);
}

static inline bool operator<(const uint128_t& left, const uint128_t& right)
{
    return (left.high_ < right.high_) || (left.high_ == right.high_ && left.low_ < right.low_);
}

#if defined WIN32 
static bool isBigEnding()
{
    union
    {
        int   i;
        char  c;
    }u={1};

    return (u.c != u.i);
}
#endif

#if defined WIN32
static uint64_t hton64(uint64_t val)
{
    if (isBigEnding())
    {
        return val;
    }
    else
    {
        union
        {
            uint64_t __u64;
            uint32_t __u32[2];
        }j, k;

        j.__u64 = val;
        k.__u32[0] = htonl(j.__u32[1]);
        k.__u32[1] = htonl(j.__u32[0]);
        return k.__u64;
    }
}
#else
static uint64_t hton64(uint64_t val)
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
        return bswap_64(val);
#elif __BYTE_ORDER == __BIG_ENDIAN
	return val;
#else
#error "Unknown __BYTE_ORDER???"
#endif
}
#endif

static uint64_t ntoh64(uint64_t val)
{
	return hton64(val);
}

#if defined WIN32
static uint128_t hton128(uint128_t val)
{
    if (isBigEnding())
    {
        return val;
    }
    else
    {
        union
        {
            uint128_t __u16;
            uint64_t __u8[2];
        } j, k;

        j.__u16 = val;
        k.__u8[0] = hton64(j.__u8[1]);
        k.__u8[1] = hton64(j.__u8[0]);

        return k.__u16;
    }
}
#else
static uint128_t hton128(uint128_t val)
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
	union {
		uint128_t __u16;
		uint64_t __u8[2];
	} j, k;

	j.__u16 = val;
	k.__u8[0] = hton64(j.__u8[1]);
	k.__u8[1] = hton64(j.__u8[0]);

	return k.__u16;
#elif __BYTE_ORDER == __BIG_ENDIAN
	return val;
#endif
}
#endif

static uint128_t ntoh128(uint128_t val)
{
	return hton128(val);
}

//
// general protocl header
//

#define MSTAR_HEAD_SIZE	(3 * sizeof(uint32_t) + 2 * sizeof(uint16_t))	// 16

struct mstar_proto_head
{
	uint32_t len_;		// the total length of this message
	uint16_t cmd_;		// the user defined command of this message
	uint16_t ver_;		// message¡¯s version, Now is MP_VERSION_1.
	uint32_t syn_;		// the sender¡¯s sequential No, useful in asynchronous comm..
	uint32_t ack_;		// the responser¡¯s acknowledge No.,

	mstar_proto_head()
		: len_(sizeof(mstar_proto_head)),
		  cmd_(0),
		  ver_(MP_VERSION_1),
		  syn_(0),
		  ack_(0)
	{
		// nothing
	}

	mstar_proto_head(const mstar_proto_head& o, uint16_t rsp_cmd = 0, uint32_t syn = 0)
		: len_(o.len_),
		  cmd_(o.cmd_),
		  ver_(o.ver_),
		  syn_(o.syn_),
		  ack_(o.ack_)
	{
		if (rsp_cmd != 0) {	// clone for response
			len_ = sizeof(mstar_proto_head);
			cmd_ = rsp_cmd;
			syn_ = syn;
			ack_ = o.syn_;
		}
	}

	void clone_for_response(const mstar_proto_head& req, uint16_t rsp_cmd = 0, uint32_t syn = 0)
	{
		this->len_ = sizeof(*this);
		this->cmd_ = (rsp_cmd == 0) ? req.cmd_ : rsp_cmd;
		this->ver_ = req.ver_;
		this->syn_ = syn;
		this->ack_ = req.syn_;

		return;
	}

	// convert to stream encoded in network byte order
	void hton(unsigned char* data, size_t size)
	{
		assert(size >= MSTAR_HEAD_SIZE);

		*(uint32_t *)data = htonl(len_); data += 4;
		*(uint16_t *)data = htons(cmd_); data += 2;
		*(uint16_t *)data = htons(ver_); data += 2;
		*(uint32_t *)data = htonl(syn_); data += 4;
		*(uint32_t *)data = htonl(ack_); data += 4;
	}

	// convert from stream
	void ntoh(const unsigned char* data, size_t size)
	{
		assert(size >= MSTAR_HEAD_SIZE);
		len_ = ntohl(*(uint32_t *)data); data += 4;
		cmd_ = ntohs(*(uint16_t *)data); data += 2;
		ver_ = ntohs(*(uint16_t *)data); data += 2;
		syn_ = ntohl(*(uint32_t *)data); data += 4;
		ack_ = ntohl(*(uint32_t *)data); data += 4;
	}
};

// 
// general data block
//
class mstar_data_block {
public:
	mstar_data_block()
		: dbuf_(0), dsize_(0), auto_delete_(false)
	{
		// nothing
	}

	mstar_data_block(unsigned char *dbuf, size_t dsize, bool auto_delete = false)
		: dbuf_(dbuf), dsize_(dsize), auto_delete_(auto_delete)
	{
		// nothing
	}

	mstar_data_block(size_t dsize)
	{
		dbuf_ = new (std::nothrow) unsigned char[dsize];
		if (dbuf_ == NULL) {
			throw std::runtime_error("can not allocate for mstar_data_block");
		}

		dsize_ = dsize;
		auto_delete_ = true;
	}

	~mstar_data_block()
	{
		if (auto_delete_ && dbuf_ && dsize_) {
			delete [] dbuf_;
		}
	}

	//add by yikui 2011.01.14
	mstar_data_block(const mstar_data_block& dblock)
	{
			dbuf_ = dblock.dbuf_; 
			dsize_ = dblock.dsize_;
			auto_delete_ = dblock.auto_delete_;
	}

	//add by yikui 2011.01.14
	mstar_data_block& operator=(const mstar_data_block& rhs)
	{
		if (this != &rhs) {
			
			if (auto_delete_ && dbuf_) {
				delete [] dbuf_;
				dbuf_ = NULL;
				dsize_ = 0;
			}

			dbuf_ = rhs.dbuf_; 
			dsize_ = rhs.dsize_;
			auto_delete_ = rhs.auto_delete_;
			
		}
		return *this; 
	}

	unsigned char *data() const { return dbuf_; }
	size_t size() const { return dsize_; }

	bool auto_delete() const { return auto_delete_; }
	void auto_delete(bool auto_del) { auto_delete_ = auto_del; }

	void setbuf(unsigned char *dbuf, size_t dsize, bool auto_delete = false) {
		if (auto_delete_ && dbuf_) {
			delete [] dbuf_;
		}

		dbuf_ = dbuf;
		dsize_ = dsize;
		auto_delete_ = auto_delete;
	}

	int resize(size_t dsize) {
		unsigned char *nbuf = (unsigned char *)new(std::nothrow) unsigned char[dsize];
		if (nbuf ==  NULL) return -1;

		if (dbuf_ != NULL && dsize_ > 0)
        {
            using namespace std;
			size_t copy = min(dsize_, dsize);
			memcpy(nbuf, dbuf_, copy);
		}

		setbuf(nbuf, dsize, true);
                return 0;
	}
    int newsize(size_t dsize)
    {
        if (dsize == 0)
        {
            setbuf(NULL, 0, true);
            return 0;
        }
          
		unsigned char *nbuf = (unsigned char *)new(std::nothrow) unsigned char[dsize];
		if (nbuf ==  NULL)
        {
            return -1;
        }

		setbuf(nbuf, dsize, true);
        return 0;
	}

private:
	unsigned char *dbuf_;
	size_t dsize_;
	bool auto_delete_;
};

enum enumProtoMethod {
	ENCODE = 0,
	DECODE = 1
};

class mstar_proto_base {
public:
	// init nothing
	mstar_proto_base()
		: dblock_(),
		  mhead_(),
		  rptr_(0),
		  wptr_(0),
		  last_errno_(0)
	{
		// nothing
	}

	// init a response protocol
	mstar_proto_base(size_t size, const mstar_proto_head& rh, uint16_t cmd, uint32_t syn)
		: dblock_(size),
		  mhead_(rh, cmd, syn),
		  rptr_(0),
		  wptr_(0),
		  last_errno_(0)
	{
		if (prepare_encode() < 0) {
			throw std::runtime_error("prepare-encode failed");
		}
	}

	// init for encoding
	mstar_proto_base(size_t size)
		: dblock_(size),
		  mhead_(),
		  rptr_(0),
		  wptr_(0),
		  last_errno_(0)
	{
		if (prepare_encode() < 0) {
			throw std::runtime_error("prepare-encode failed");
		}
	}

	// init for encoding/decoding
	mstar_proto_base(unsigned char *data, size_t size, enumProtoMethod method)
		: dblock_(data, size),
		  mhead_(),
		  rptr_(0),
		  wptr_(0),
		  last_errno_(0)
	{
		if (method == ENCODE) {
			if (prepare_encode() < 0) {
				throw std::runtime_error("prepare-encode failed");
			}
		}
		else {
			if (prepare_decode() < 0) {
				throw std::runtime_error("prepare-decode failed");
			}
		}
	}

	~mstar_proto_base()
	{
		// nothing now
	}
public:

	int construct_res (size_t size, const mstar_proto_head& rh, uint16_t cmd, uint32_t syn)
    {
        if (dblock_.newsize(size) != 0)
        {
            return -1;
        }

		mhead_ = mstar_proto_head(rh, cmd, syn),
		rptr_ = 0,
		wptr_ = 0,
		last_errno_ = 0;

        if (prepare_encode() < 0)
        {
            dblock_.newsize(0);         //we must free memory
            return -1;
        }
        return 0;
    }
	// setup decode env
	int prepare_decode(size_t size = 0) {
		if (dblock_.size() < sizeof(mstar_proto_head))
			return -1;
		mhead_.ntoh(dblock_.data(), sizeof(mstar_proto_head));
		rptr_ = sizeof(mstar_proto_head);
		wptr_ = (size == 0) ? dblock_.size() : size;
		return 0;
	}

	int prepare_decode(unsigned char *data, size_t size, bool auto_delete = false) {
		if (data == NULL || size < sizeof(mstar_proto_head))
			return -1;
		dblock_.setbuf(data, size, auto_delete);
		return prepare_decode();
	}

	int end_decode() {
		if (last_errno_)
			return -1;
		return 0;
	}

	int prepare_encode() {
		if (dblock_.size() < sizeof(mstar_proto_head)) 
			return -1;
		rptr_ = 0;
		wptr_ = MSTAR_HEAD_SIZE;
		return 0;
	}

	int prepare_encode(unsigned char *data, size_t size, bool auto_delete = false) {
		if (data == NULL || size < MSTAR_HEAD_SIZE)
			return -1;
		dblock_.setbuf(data, size, auto_delete);
		return prepare_encode();
	}

	int end_encode() {
		if (last_errno_)
			return -1;
		mhead_.len_ = (uint32_t)size();
		mhead_.hton(dblock_.data(), MSTAR_HEAD_SIZE);
		return 0;
	}

	// set data block
	const mstar_data_block& dblock() const { return dblock_; }
	mstar_data_block& dblock() { return dblock_; }

	// dispatcher for data-block
	unsigned char *data() const { return dblock_.data(); }
	size_t capacity() const { return dblock_.size(); }

	bool auto_delete() const { return dblock_.auto_delete(); }
	void auto_delete(bool auto_del) { dblock_.auto_delete(auto_del); }

	void setbuf(unsigned char *dbuf, size_t dsize, bool auto_delete = false) {
		dblock_.setbuf(dbuf, dsize, auto_delete);
	}

	int resize(size_t nsize) {
		if (nsize < dblock_.size())
			return 0;
		return dblock_.resize(nsize);	
	}

	const mstar_proto_head& head() const { return mhead_; }
	mstar_proto_head& head() { return mhead_; }

	void reset_rptr() { rptr_ = 0; }
	void reset_wptr() { wptr_ = 0; }
	void reset_xptr() { rptr_ = wptr_ = 0; }

	// get the current read pointer
	unsigned char* rptr() const { return dblock_.data() + rptr_; }
	// adjust read pointer by adv(can be negative)
	void rptr(ptrdiff_t adv) { rptr_ += adv; }
	// set the new read pointer
	void rptr(unsigned char* rptr) { rptr_ = rptr - dblock_.data(); }

	// get the current write pointer
	unsigned char* wptr() const { return dblock_.data() + wptr_; }
	// adjust write pointer
	void wptr(ptrdiff_t adv) { wptr_ += adv; }
	// set new write pointer
	void wptr(unsigned char* wptr) { wptr_ = wptr - dblock_.data(); }

	// data size
	size_t size() const { return size_t(wptr_ - rptr_); }
	// space left
	size_t space() const { return dblock_.size() - wptr_; }

	// get last errno
	int last_errno() const { return last_errno_; }
	// reset last errno to 0
	void reset_errno() { last_errno_ = 0; }
protected:
	unsigned char* adjust_wptr(size_t size) {
		if (last_errno_) {
			return NULL;
		}

		if ((wptr_ + size) > dblock_.size()) {
			last_errno_ = ENOMEM;
			return NULL;
		}

		unsigned char *wptr = dblock_.data() + wptr_;
		wptr_ += size;

		return wptr;
	}
	
	int write_1(uint8_t val) {
		unsigned char *wptr = adjust_wptr(sizeof(val));
		if (wptr != NULL) {
			*wptr = val;
			return  0;
		}

		return -1;
	}

	int write_2(uint16_t val) {
		unsigned char *wptr = adjust_wptr(sizeof(val));
		if (wptr != NULL) {
			*(uint16_t *)wptr = htons(val);
			return 0;
		}

		return -1;
	}

	int write_4(uint32_t val) {
		unsigned char *wptr = adjust_wptr(sizeof(val));
		if (wptr != NULL) {
			*(uint32_t  *)wptr = htonl(val);
			return 0;
		}

		return -1;
	}

	int write_8(uint64_t val) {
		unsigned char *wptr = adjust_wptr(sizeof(val));
		if (wptr != NULL) {
			*(uint64_t  *)wptr = hton64(val);
			return 0;
		}

		return -1;
	}

	int write_16(uint128_t val) {
		unsigned char *wptr = adjust_wptr(sizeof(val));
		if (wptr != NULL) {
			*(uint128_t *)wptr = hton128(val);
			return 0;
		}

		return -1;
	}
public:
	//
	// base data type's operation
	//  return 0: OK
	//        -1: error
	//
	//  write: append data to the end of the package in network byte order.
	//  insert: insert data to specific position, and encode into network byte order.
	//  read: read next data from package, decode into local byte order
	//  fetch: read data from spcific position, and decode into local byte order.
	//
	int write(unsigned char val) {
		assert(sizeof(val) == sizeof(uint8_t));
		return write_1((uint8_t)val);
	}
	int write(char val) { 
		return write((unsigned char)val);
	}
	int write(unsigned short val) { 
		assert(sizeof(val) == sizeof(uint16_t)); 
		return write_2((uint16_t)val); 
	}
	int write(short val) { 
		return write((unsigned short)val);
	}
	int write(unsigned int val) { 
		assert(sizeof(val) == sizeof(uint32_t)); 
		return write_4((uint32_t)val); 
	}
	int write(int val) { 
		return write((unsigned int)val);
	}
	int write(unsigned long val) { 
#if __WORDSIZE == 32
		assert(sizeof(val) == sizeof(uint32_t)); 
		return write_4((uint32_t)val); 
#elif __WORDSIZE == 64
		assert(sizeof(val) == sizeof(uint64_t)); 
		return write_8((uint64_t)val); 
#else
#error "Unknow word size!" 
#endif
	}
	int write(long val) { 
		return write((unsigned long)val);
	}
	int write(unsigned long long val) { 
		assert(sizeof(val) == sizeof(uint64_t)); 
		return write_8((uint64_t)val); 
	}
	int write(long long val) { 
		return write((unsigned long long)val);
	}
	int write(float val) { 
		assert(sizeof(val) == sizeof(uint32_t)); 
		uint32_t tmp;
		memcpy(&tmp, &val, sizeof(val));
		return write_4(tmp); 
	}
	int write(double val) { 
		assert(sizeof(val) == sizeof(uint64_t)); 
		uint64_t tmp;
		memcpy(&tmp, &val, sizeof(val));
		return write_8(tmp); 
	}
	int write(uint128_t val) { 
		assert(sizeof(val) == 16);
		return write_16((uint128_t)val); 
	}

	int write_data(const unsigned char* data, size_t len) {
		unsigned char *wptr = adjust_wptr(len);
		if (wptr != NULL) {
			memcpy(wptr, data, len);
			return 0;
		}

		return -1;
	}

	int write(const char* str, size_t len = 0) {
		unsigned char* old_rptr = rptr();
		if (str == NULL)
			len = 0;
		else if (len == 0)
			len = strlen(str) + 1;
		if (write_4((int)len) < 0 || write_data((unsigned char *)str, len) < 0) {
			rptr(old_rptr);	// restore
			return -1;
		}
		return 0;
	}

	int write(const std::string& str) {
		return write(str.c_str(), str.size() + 1);
	}

	int insert(unsigned char* pos, unsigned char val) { return -1; }
	int insert(unsigned char* pos, char val) { return -1; }

protected:
	unsigned char* adjust_rptr(size_t size) {
		if (last_errno_) {
			return NULL;
		}

		if ((rptr_ + (ptrdiff_t)size) > wptr_) {
			last_errno_ = EINVAL;
			return NULL;
		}

		unsigned char *rptr = dblock_.data() + rptr_;
		rptr_ += size;

		return rptr;
	}

	int read_1(uint8_t* val) {
		unsigned char* rptr = adjust_rptr(sizeof(*val));
		if (rptr != NULL) {
			*val = *(uint8_t *)rptr;
			*val = *rptr;
			return 0;
		}

		return -1;
	}
	int read_2(uint16_t* val) {
		unsigned char* rptr = adjust_rptr(sizeof(*val));
		if (rptr != NULL) {
			*val = ntohs(*(uint16_t *)rptr);
			return 0;
		}

		return -1;
	}
	int read_4(uint32_t* val) {
		unsigned char* rptr = adjust_rptr(sizeof(*val));
		if (rptr != NULL) {
			*val = ntohl(*(uint32_t *)rptr);
			return 0;
		}

		return -1;
	}
	int read_8(uint64_t* val) {
		unsigned char* rptr = adjust_rptr(sizeof(*val));
		if (rptr != NULL) {
			*val = ntoh64(*(uint64_t *)rptr);
			return 0;
		}

		return -1;
	}
	int read_16(uint128_t* val) {
		unsigned char* rptr = adjust_rptr(sizeof(*val));
		if (rptr != NULL) {
			*val = ntoh128(*(uint128_t *)rptr);
			return 0;
		}

		return -1;
	}
public:
	int read(unsigned char& val) {
		assert(sizeof(val) == sizeof(uint8_t));
		return read_1((uint8_t *)&val);
	}
	int read(char& val) {
		return read(reinterpret_cast<unsigned char &>(val));
	}
	int read(unsigned short& val) {
		assert(sizeof(val) == sizeof(uint16_t));
		return read_2(reinterpret_cast<uint16_t *>(&val));
	}
	int read(short& val) {
		return read(reinterpret_cast<unsigned short&>(val));
	}
	int read(unsigned int& val) {
		assert(sizeof(val) == sizeof(uint32_t));
		return read_4(reinterpret_cast<uint32_t *>(&val));
	}
	int read(int & val) {
		return read(reinterpret_cast<unsigned int&>(val));
	}
	int read(unsigned long& val) {
#if __WORDSIZE == 32
		assert(sizeof(val) == sizeof(uint32_t));
		return read_4(reinterpret_cast<uint32_t *>(&val));
#elif __WORDSIZE == 64
		assert(sizeof(val) == sizeof(uint64_t));
		return read_8(reinterpret_cast<uint64_t *>(&val));
#else
#error "Unknown __WORDSIZE?"
#endif
	}
	int read(long& val) {
		return read(reinterpret_cast<unsigned long&>(val));
	}
	int read(unsigned long long& val) {
		assert(sizeof(val) == sizeof(uint64_t));
		return read_8(reinterpret_cast<uint64_t *>(&val));
	}
	int read(long long& val) {
		return read(reinterpret_cast<unsigned long long&>(val));
	}
	int read(float& val) {
		assert(sizeof(val) == sizeof(uint32_t));
		return read_4(reinterpret_cast<uint32_t *>(&val));
	}
	int read(double& val) {
		assert(sizeof(val) == sizeof(uint64_t));
		return read_8(reinterpret_cast<uint64_t *>(&val));
	}
	int read(uint128_t& val) {
		assert(sizeof(val) == sizeof(uint128_t));
		return read_16(reinterpret_cast<uint128_t *>(&val));
	}

	int read_data(unsigned char* data, size_t size) {
		unsigned char  *rptr = adjust_rptr(size);
		if  (rptr != NULL) {
			memcpy(data, rptr, size);
			return 0;
		}

		return -1;
	}

	int read_data_ref(unsigned char*& data, size_t size) {
		unsigned char  *rptr = adjust_rptr(size);
		if (rptr != NULL) {
			data = rptr;
			return 0;
		}

		return -1;
	}

	int read(char *str, size_t& size) {
		unsigned char* old_rptr = rptr();
		unsigned char* cur_rptr;
		uint32_t len;

		if (read_4(&len) >= 0 && (cur_rptr = adjust_rptr(len)) != NULL)
		{
			using namespace std;
			size_t copied = min(size, (size_t)len);
			memcpy(str, cur_rptr, copied);
			size = len;
			return 0;
		}

		rptr(old_rptr);
		return -1;
	}

	int read(std::string& str) {
		unsigned char* old_rptr = rptr();
		unsigned char* cur_rptr;
		uint32_t len;

		if (read_4(&len) >= 0 && (cur_rptr = adjust_rptr(len)) != NULL) {
			str.assign((char  *)cur_rptr, len > 1 ? len - 1 : 0);
			return 0;
		}

		rptr(old_rptr);
		return -1;
	}

	int read_string_ref(char*& str, size_t& size) {
		unsigned char* old_rptr = rptr();
		unsigned char* cur_rptr;
		uint32_t len;

		if  (read_4(&len) >= 0 && (cur_rptr = adjust_rptr(len)) != NULL) {
			str = len > 0 ? (char *)cur_rptr : NULL;
			size = len;
			return 0;
		}

		rptr(old_rptr);
		return -1;
	}

	int fetch(unsigned char* pos, unsigned char& val) { return -1; } 
	int fetch(unsigned char* pos, char& val) { return fetch(pos, reinterpret_cast<unsigned char&>(val)); }

	// TODO: more fetch

private:
	mstar_data_block dblock_;	// data¡¯s storage
	mstar_proto_head mhead_;	// message¡¯s header
	ptrdiff_t rptr_;		// next read pointer(offset)
	ptrdiff_t wptr_;		// next write pointer(offset)
	int last_errno_;		// last error
};

#endif /*! __MSTAR_PROTOCOL__H */
