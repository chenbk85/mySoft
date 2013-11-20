option optimize = SPEED | SIZE;
option fixed-width = true | false(default)

include "../basic.bson"
include "../share.bson"

option-lines = option-line | option-lines
option-line = "option" option-setting ";"
option-setting = "optimize" "=" "SPEED" | "SIZE"
		| fixed-width = true | false

include-lines = include-line | include-lines
include-line = "include" "\""path"\""

object = [ struct-obj | message-obj | union-obj ] ";"

struct-obj = "struct" var-name "{" fields "}"
message-obj = "message" var-name "{" fields "}"
union-obj = "union" var-name "{" fields "}"

fields = field | fields
field = man-option field-type var-name field-option-section ";"
	| man-option "array" field-type "[" number "]" var-name field-option-section ";"
	| man-option "var-array" field-type "[" init [inc [max]] "]" var-name field-option-section ";"

man-option = | mandatory | optional(def)
field-option-section = "(" field-options ")"


field-type:
	byte|char|short|int|long|float|double|string
	array type[ number ] // fixed array
	var-array type[ init [inc [ max ] ] ] // vector<?>, or { storage_start/end, iter_end, }

field-options = field-option | field-options
field-option = "default" "=" value
	| "xml-tag" "=" xml-tag
	| "json-name" "=" json-name
	| "encoding" "=" "base64|utf8"

inc = "+" integer | "*" float
max = number | "unlimited"

string == var-array[0 *2 ulimit] char


message xx-request {
	int cmd;
	union bodies; 
}

message xx-pos-len {
	long offset;
	long length;
};

message xx-write {
	string file;
	long offset;
	var-array byte[1024 *2 100M] data;
	array xx-pos-len[32] pls;
	var-array xx-pos-len[1 +2 1000] pls2;
}

<message name="xx-write">
	<field n="file" type="string" v="/tmp/t.data"/>
	<field n="offset" type="long" v="1024"/>
	<field n="data" type="var-array" arr-type="byte" init="1024" inc="*2" max="100M" v="data-encoded"/>
	<field n="pls" type="array xx-pos-len[32]">
	<field n="pls" type="array" arr-type="xx-pos-len" size="32">
		<item i="0">
			<field n="offset" type="long" v="1"/>
			<field n="length" type="long" v="2"/>
		</item>
		<item i="1"/>
		<!-- ... !>
		<item i="31"/>
	</field>
</message>

message xx-read {
	string file;
	long offset;
	long length;
};


service xxx-service {
	int add(struct xxx-req) = 1;
	struct xxx-rsp sub(int a, int b) = 2;
};

==>
class xxx-service-client {
	xxx-service-client(connector& conn);
public:
	struct xxx-rsp sub(struct xxx-req& p1, ...., pn); // sync-call
	{
		message out, in;
		out.encode(p1); ...; out.encode(pn);
		int retval = conn->request(out, &in);
		if (retval == 0) {
			in.decode(rsp);
			return rsp;
		}
	}

	int retval sub(struct xxx-req& p1, ..., pn, void (*cb)(struct xxx-rsp& rsp, void *), data); // async-call
	{
		message out;
		out.encode(p1); ...; out.encode(pn);
		int retval = conn->async_request(out, cb2, cb, data);
		return retval;
	}
private:
	xxx
};

generated-cb2(int retval, message& in, void (*cb)(xxx-rsp& rsp, void *), void* data)
{
	if (retval == 0) { // success
		rsp = in.decode();
		(*cb)(rsp, data);
	}
	else if (retval == ETIMEDOUT) { // time out
	}
	else { // other error
	}	
}

class xxx-service-impl {
	struct xxx-rsp sub(struct xxx-req& p1, ..., pn);
	int sub(struct xxx-req& p1, ..., pn, struct xxx-rsp& rsp); // or the last is return
};

xxx-service-stub {
	int stub_sub(message& in, message& out) {
		p1 = in.decode();
		...
		pn = in.decode();
#if 1
		try {
			struct xxx-rsp rsp = impl->sub(p1, ..., pn);
		}
		catch (std::exception& e) {
			out.retval(e.errno());
			out.errmsg(e.error());
		}
#else
		struct xxx-rsp rsp;
		int retval = impl->sub(p1, ..., pn, &rsp);
		if (retval != 0) {
			out.retval(retval);
			out.errmsg(strerror(retval));
		}
		else {
			out.encode(rsp);
		}
#endif
		return 0;
	}
}


client:
	xxx-service-client xsc(xxx);
	int s = xsc->add(a, b);
	// int retval = xc->add(a, b void cb(int s, data) { sum = s; ... }, data);
	struct xxx-rsp rsp = xsc->sub(a, b);
	// int retval = xc->sub(a, b, void cb(&rsp, data) { rsp = ....; ....}, data);


server:
	xxx-service-impl impl;
	add-stub()
	{
		decode a, b;
		int s = impl->add(a, b);
		encode s;
	}

