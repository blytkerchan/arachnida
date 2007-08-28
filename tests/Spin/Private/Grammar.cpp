#include "../../../lib/Spin/Private/Grammar.h"

int main()
{
	Spin::Private::Grammar grammar;

	char hello[] = "hello";
	boost::spirit::parse(hello, grammar);
}
