#ifndef IPRESTRICTION_H_
#define IPRESTRICTION_H_

#include <stdint.h>
#include <vector>

#define RULE_DENY     0x00
#define RULE_ALLOW   0x01

#define RULE_SUBNET  0x00
#define RULE_IP           0x01
#define RULE_ALL         0x02

#define MAX_MASK_BYTES 32

#define white_black 0
#define black_white 1

#define BASE10 10
#define BASE16 16

typedef struct 
{
    unsigned long  s_addr ;
    unsigned short ip_port_start;
    unsigned short ip_port_end;
} ipr_ip_t;

typedef struct 
{
    ipr_ip_t      ip;
    unsigned char mask;
} ipr_subnet_t;

typedef union 
{
    ipr_subnet_t   subnet;
    ipr_ip_t          ip;
} ipr_format_t;

typedef struct 
{
    unsigned char type;
    ipr_format_t   match;
} ipr_rule_t;

class IpRestriction
{
    public:
                IpRestriction();
                ~IpRestriction();
       int    load_config(const char * file);
	bool check_ip(const struct sockaddr_in & clientAddr) const;

    private:
	int set_order(int order);
	int parse_ip (const char * const token, ipr_ip_t * const out);
	int parse_subnet (const char* const token, ipr_subnet_t * const out);
	int parse_rule(const char * const line_buf, const int cline);
	bool match(const std::vector<ipr_rule_t> &v_rules,  const struct sockaddr_in & clientAddr) const;	

	std::vector<ipr_rule_t> v_whitelist_;
	std::vector<ipr_rule_t> v_blacklist_;
	int order_;
};

#endif /* IPRESTRICTION_H_ */

