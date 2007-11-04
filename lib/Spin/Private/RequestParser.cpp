#include "RequestParser.h"
#include "RequestGrammar.h"
#include <boost/spirit/tree/parse_tree.hpp>
#include <boost/spirit/tree/tree_to_xml.hpp>
#include <iostream>

namespace Spin
{
	namespace Private
	{
		Details::Request parse(const std::string & request)
		{
			const char * first(request.c_str());
			boost::spirit::tree_parse_info<> info(boost::spirit::pt_parse(first, RequestGrammar()));
			// dump parse tree as XML
			std::map< boost::spirit::parser_id, std::string > rule_names;
			//rule_names[integer.id()] = "integer";
			//rule_names[factor.id()] = "factor";
			//rule_names[term.id()] = "term";
			//rule_names[expression.id()] = "expression";
			tree_to_xml(std::clog, info.trees, first, rule_names);

			return Details::Request();
		}
	}
}

