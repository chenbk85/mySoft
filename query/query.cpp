#include <iostream>
#include <stream>

static void do_match(const query_exp<document>* exp, const std::vector<document>& docs, std::vector<int>& matches)
{
	for (std::vector<document>::const_iterator iter = docs.begin();
		iter != docs.end();
			++iter) {
		if (exp->execute(*iter)) {
			matches.push_back(iter->id());
		}
	}

	return matches.size();
}

int main(int argc, char **argv)
{
	// load into memory

	char line[8192];

	fprintf(stdout, "Query? ");
	while (fgets(line, sizeof line - 1, stdin) != NULL) {
		line[sizeof line - 1] = 0;
		char* eptr = line + strlen(line) - 1;
		while (eptr > line && (*eptr == '\n' || *eptr == '\r'))
			--eptr;
		*eptr = 0;

		query_expr<document>* exp;
		yyFlexLexer lex();
		int ret = yyparse(lex, exp);
		if (ret == 0) {
			std::vector<int> ids;
			do_match(exp, docs, ids);
		}
	}

	return 0;
}

		
		
