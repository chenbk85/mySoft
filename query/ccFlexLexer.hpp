#ifndef __CC_FLEXLEXER_HPP
#define __CC_FLEXLEXER_HPP

#include <FlexLexer.h>
#include "yacc.tab.hpp"

extern "C" {
//struct yy_buffer_state;
//struct yy_buffer_state* init_static_buffer_state(struct yy_buffer_state* bs);
};

class ccFlexLexer : public yyFlexLexer {
public:
        ccFlexLexer(char* ibuf, size_t isize)
		: yyFlexLexer(NULL, NULL),
		  use_static_buffer_(false)
	{
		// build our own buffer
		//yy_current_buffer = init_static_state(&static_buffer_state_);
	}

        ccFlexLexer(std::istream* arg_yyin, std::ostream* arg_yyout)
                : yyFlexLexer(arg_yyin, arg_yyout),
		  use_static_buffer_(false)
        {
                // nothing
        }

	virtual ~ccFlexLexer()
	{
		if (use_static_buffer_) {
			// clear it first.
			// otherwise yyFlexLexer will be freed as default way.
			yy_current_buffer = NULL;
		}
	}
public:
	int yylex(yy::query_parser::semantic_type* yylval);
private:
	bool use_static_buffer_;
	//struct yy_buffer_state static_buffer_state_;
};

#endif /*!__CC_FLEXLEXER_HPP */
