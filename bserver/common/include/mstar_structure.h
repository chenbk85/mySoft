#ifndef __MSTAR_STRUCTURE__H
#define __MSTAR_STRUCTURE__H

#include "mstar_protocol.h"
//#include <uuid/uuid.h>
#include "md5.h"

#define MAX_KEY_SIZE	      32
#define MAX_QUERY_NUM         10
#define MAX_FIELD_NUM         10
#define PKEY_LEN              8

//Master Server Error Code Define
enum EnumMasterErrorCode
{
	E_MASTER_CHECK_INSERT_RANGE_TIMEOUT  = 21001,	//insert time out
	E_MASTER_CHECK_QUERY_RANGE_TIMEOUT   = 21002,	//query time out
	E_MASTER_CHECK_DELETE_RANGE_TIMEOUT  = 21003,	//delete time out
	E_MASTER_CHECK_UPDATE_RANGE_TIMEOUT  = 21004,	//update time out
	E_MASTER_QUERY_ALLRANGE_DOWN         = 21005,	//all range down
	E_MASTER_INVALID_CLIENT_PARAM        = 21006,	//Invalid Parameter(s)
	E_MASTER_INVALID_RANGE_PARAM         = 21007,	//Invalid Parameter(s)
};

//Range server Error Code Define
enum EnumRangeErrorCode 
{
    E_RANGE_NOT_IMPL = 22001,
    E_RANGE_INVALID_IN_PROTO,
    E_RANGE_NOT_FOUND,
    E_RANGE_INSERT_FAILED,
    E_RANGE_BUILD_HASH_FAILED,

    E_RANGE_ALLOC_CHUNK,
    E_RANGE_OVERLOAD,
    E_RANGE_KEY_BUSY,
    E_RANGE_KEY_NOT_LOCKED,
    E_RANGE_REALLOC_NO_NEED,

    E_RANGE_NO_SPACE,
    E_RANGE_NO_SERVER,
    E_RANGE_INTERNAL,
    E_RANGE_READ_ONLY,
    E_RANGE_HS_SERVICE_MISMATCH,
    E_RANGE_HS_STATUS_INVALID,
    E_RANGE_HS_FAILED,
};

enum EnumMasterCommand 
{
	CMD_MASTER_INSERT = 10000,
	CMD_MASTER_INSERT_RSP,

	CMD_MASTER_QUERY,
	CMD_MASTER_QUERY_RSP,

	CMD_MASTER_DELETE,
	CMD_MASTER_DELETE_RSP,

	CMD_MASTER_UPDATE,
	CMD_MASTER_UPDATE_RSP,

	CMD_MASTER_MOVE,
	CMD_MASTER_MOVE_RSP,

	CMD_MASTER_HELLO,
	CMD_MASTER_HELLO_RSP,

	CMD_MASTER_BUTT
};

enum EnumRangeCommand 
{
	CMD_RANGE_INSERT = 20000,
	CMD_RANGE_INSERT_RSP,

	CMD_RANGE_QUERY,
	CMD_RANGE_QUERY_RSP,

	CMD_RANGE_DELETE,
	CMD_RANGE_DELETE_RSP,

	CMD_RANGE_UPDATE,
	CMD_RANGE_UPDATE_RSP,

	CMD_RANGE_CHECK,
	CMD_RANGE_CHECK_RSP,

	CMD_RANGE_HELLO,
	CMD_RANGE_HELLO_RSP,

    CMD_RANGE_GET_LASTID,
    CMD_RANGE_GET_LASTID_RSP,

    CMD_RANGE_GET_CDATA,
    CMD_RANGE_GET_CDATA_RSP,    

	CMD_RANGE_BUTT
};

#define FIELD_TYPE_PK 0xFE

enum EnumOperationTypeCommand
{
	OPERATION_TYPE_EQUAL =0,//"=="
	OPERATION_TYPE_LESS,    //"<"
	OPERATION_TYPE_MORE,    //">"
	OPERATION_TYPE_LESSEQ,  //"<="
	OPERATION_TYPE_MOREEQ,  //">="
	OPERATION_TYPE_TOTAL,
};

enum EnumConditionOperationType
{
    OPERATION_CONDITION_NULL = 0,
    OPERATION_CONDITION_AND, 
    OPERATION_CONDITION_OR, 
    OPERATION_CONDITION_BUTT
};


struct mstar_query_condition
{    
    uint8_t		op_type_;  //logic sign, for example,==,EnumOperationTypeCommand
    uint32_t 	value_len_; 
    uint8_t 	value_[MAX_KEY_SIZE]; //accid¡¯s value
};

struct mstar_query_field
{
    uint8_t		field_type_; //for example,accid,EnumFieldTypeCommand
    uint8_t     condition_op_;   //'and', 'or', EnumConditionOperationType
    uint8_t     condition_num_;
    mstar_query_condition condition_[MAX_QUERY_NUM];
};

struct mstar_update_field
{
    uint8_t    field_type_;
    uint32_t   value_len_;
    uint8_t    value_[MAX_KEY_SIZE];
};

struct mstar_atomic_query_condition 
{
	uint8_t		field_type_; //for example,accid,EnumFieldTypeCommand
	uint8_t		op_type_;  //logic sign, for example,==,EnumOperationTypeCommand
	uint32_t 	value_len_; 
	uint8_t 	value_[MAX_KEY_SIZE]; //accid¡¯s value
};

//Between master server and business 
// ********************* CMD_MASTER_INSERT *****************************
struct mstar_master_insert_req 
{
	uint32_t    bskey_len_;
	uint8_t     bskey_[MAX_KEY_SIZE];
	uint32_t 	data_len_;
	uint8_t* 	data_;
};

struct mstar_master_insert_res
{
	mstar_data_block  dblock_;
	uint32_t 	      ret_code_;	//0: means OK
};

// ********************* CMD_MASTER_QUERY *****************************

struct mstar_master_query_req 
{
    uint32_t	limit_num_;     //query number of the result, 0 means no limit
    uint32_t	position_;      //start scan position
    //////////////////////////////////////////////////////////////////////////
    //page access
    uint32_t page_size;
    uint32_t page_no;
    //////////////////////////////////////////////////////////////////////////
    uint32_t 	query_num_;
    mstar_query_field 	query_field_[MAX_FIELD_NUM];
};


struct mstar_master_query_res
{
	mstar_data_block  dblock_;
	uint32_t 	ret_code_;	//0: means OK
	uint32_t 	next_position_; //next position to query, 0 means next position invalid
	uint32_t    count_;
    //////////////////////////////////////////////////////////////////////////
    //page access
    uint32_t total_count_;
    //////////////////////////////////////////////////////////////////////////
	uint32_t 	data_len_;
	uint8_t* 	data_;
};

// ********************* CMD_MASTER_DELETE *****************************
struct mstar_master_delete_req 
{
	uint32_t    bskey_len_;
	uint8_t     bskey_[MAX_KEY_SIZE];
	uint32_t 	query_num_; 
	//mstar_atomic_query_condition 	query_condition_[MAX_QUERY_NUM];
	mstar_query_field 	query_field_[MAX_FIELD_NUM];
};

struct mstar_master_delete_res
{
	mstar_data_block  dblock_;
	uint32_t 	      ret_code_;	//0: means OK
};

// ********************* CMD_MASTER_UPDATE *****************************
struct mstar_master_update_req 
{
	uint32_t    bskey_len_;
	uint8_t     bskey_[MAX_KEY_SIZE];
	uint32_t 	query_num_; 
	mstar_query_field 	query_field_[MAX_FIELD_NUM];
	uint32_t            field_num_;
	mstar_update_field  update_field_[MAX_FIELD_NUM];
	/*uint32_t 	data_len_;
	uint8_t* 	data_;*/
};

struct mstar_master_update_res
{
	mstar_data_block  dblock_;
	uint32_t 	      ret_code_;	//0: means OK
};

// ********************* CMD_MASTER_MOVE *****************************
struct mstar_master_move_req 
{
	uint32_t    ip_old_;
	uint32_t    ip_new_;
	uint32_t 	block_no_; 
};

struct mstar_master_move_res
{
	mstar_data_block  dblock_;
	uint32_t 	      ret_code_;	//0: means OK
};

// ********************* CMD_MASTER_HELLO *****************************
struct mstar_master_hello_req 
{
    uint32_t reserved_;
};

struct mstar_master_hello_res 
{
	mstar_data_block  dblock_;
	uint32_t ret_code_;	//0: means OK
};


//Between master server and range server 
// ********************* CMD_RANGE_QUERY *****************************
struct mstar_range_query_req
{
    //////////////////////////////////////////////////////////////////////////
    //query data in batch
    uint32_t	limit_num_;     // expect to return the number of the result
    uint32_t	position_;      // the position to scan , usefull for date, type field in holding table
    //////////////////////////////////////////////////////////////////////////
    //page access
    uint32_t page_size;
    uint32_t page_no;
    //////////////////////////////////////////////////////////////////////////
	uint32_t 	query_num_; 
    mstar_query_field 	query_field_[MAX_FIELD_NUM];	
};

struct mstar_range_query_res
{
	mstar_data_block dblock_;
	uint32_t    retcode_;
    uint32_t 	next_position_; // next position to scan
    uint32_t    count_;      // the number of the result for this round
    //////////////////////////////////////////////////////////////////////////
    //page access
    uint32_t    total_count_;
    //////////////////////////////////////////////////////////////////////////
	uint32_t 	data_len_;
	uint8_t* 	data_;
};

// ********************* CMD_RANGE_INSERT *****************************
struct mstar_range_insert_req 
{
	uint32_t    bskey_len_;
	uint8_t     bskey_[MAX_KEY_SIZE];
	uint32_t 	data_len_;
	uint8_t* 	data_;
};

struct mstar_range_insert_res
{
	mstar_data_block  dblock_;
	uint32_t 	      ret_code_;	//0: means OK
};

// ********************* CMD_RANGE_DELETE *****************************
struct mstar_range_delete_req 
{
	uint32_t 	        query_num_; 
    mstar_query_field 	query_field_[MAX_FIELD_NUM];
};

struct mstar_range_delete_res
{
	mstar_data_block  dblock_;
	uint32_t 	      ret_code_;	//0: means OK
};

// ********************* CMD_RANGE_UPDATE *****************************
struct mstar_range_update_req 
{
	uint32_t 	        query_num_; 	
    mstar_query_field 	query_field_[MAX_FIELD_NUM];
    uint32_t            field_num_;
    mstar_update_field  update_field_[MAX_FIELD_NUM];
};

struct mstar_range_update_res
{
	mstar_data_block  dblock_;
	uint32_t 	      ret_code_;	//0: means OK
};

// ********************* CMD_RANGE_CHECK *****************************
struct mstar_range_check_req 
{
    uint32_t        bucketNo;
    uint32_t        elemNo;
    uint32_t        totalBlockNum;
	uint32_t        blockNo;
};

struct mstar_range_check_res 
{
    mstar_data_block  dblock_;
    uint32_t        retcode_;
    uint32_t        nextBucketNo;
    uint32_t        nextElemNo;
    uint32_t        count;
    uint32_t        len;
    unsigned char*  data;//the list of mstar_range_key_data
};

// ********************* CMD_RANGE_HELLO *****************************
struct mstar_range_hello_req {
    uint32_t reserved_;
};

struct mstar_range_hello_res {
    uint32_t reserved_;
};

// ********************* CMD_RANGE_GET_LASTID *****************************
struct mstar_range_get_lastid_req
{
};

struct mstar_range_get_lastid_res
{
    uint32_t        lastSid;
};

// ********************* CMD_RANGE_GET_CDATA *****************************
#define MAX_FIX_DATA_SIZE 512
struct mstar_range_fix_elem
{
    uint64_t    sid_;
    //mstar_range_elem_data data_;
    uint32_t    elem_index_;//locate the position in elem file
    uint32_t    data_len_;
    uint8_t     data_[MAX_FIX_DATA_SIZE];
};

struct mstar_range_get_cdata_req
{
    uint64_t        lastSid_;    
    uint64_t        pos_;
    uint32_t        count_;
};

#define MAX_CDATA_COUNT 500
struct mstar_range_get_cdata_res
{
    uint32_t		retcode_;
    uint64_t		nextPos_;
    uint32_t		count_;
    mstar_range_fix_elem cdata_[MAX_CDATA_COUNT];
};

//////////////////////////////////////////////////////////////////////////
//generate pk
struct mstar_pk_data
{
    uint32_t pk_len_;
    uint8_t* pk_data_;
};

#define BSKEY_LEN 16
static void GeneratePK(uint8_t pk[BSKEY_LEN], const mstar_pk_data& pkdata)
{    
    md5_buffer((const char  *)pkdata.pk_data_, pkdata.pk_len_, (void *)pk);
}

#endif /*__MSTAR_STRUCTURE__H */
