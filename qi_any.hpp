#include <boost/any.hpp>

NAMESPACE_CIRCLE_BEGIN
namespace json {
template <typename Iter>
struct qi_any_grammar: qi::grammar<Iter, boost::any(), ascii::space_type> {
	qi_any_grammar(): qi_any_grammar::base_type(json_text) {
		json_text %= object_ | array_;
		value_ %= array_ | string_ | (qi::int_parser<int64_t>() >> !qi::lit('.')) | qi::double_ | object_ | qi::bool_ | null_;
		null_ = qi::lit("null")[qi::_val = phx::construct<null_t>()];
		ucn_code = "\\u" > qi::uint_parser<uint32_t, 16, 4, 4>()[qi::_val = qi::_1];
		escaped =
			(qi::lit('&') >>
			 (qi::lit("gt;")[qi::_val = '>'] |
			  qi::lit("lt;")[qi::_val = '<'] | 
			  qi::lit("amp;")[qi::_val = '&'] |
			  qi::eps[qi::_val = '&'])) |
			(qi::lit('\\') >>
			 (qi::char_("bfnrt")[qi::_val = qi::_1 - 'a'] |
			  qi::char_("/\\\"")[qi::_val = qi::_1])) |
			(ucn_code[qi::_pass = ucn_to_utf8(qi::_1, qi::_val)] |
			 (ucn_code > ucn_code)[qi::_pass = ucn_to_utf8(((qi::_1-0xD800)<<10)+(qi::_2-0xDC00)+0x10000, qi::_val)]);
		string_ = qi::lexeme['"' > *(escaped[qi::_val += qi::_1] | (ascii::char_-'"')[qi::_val += qi::_1]) > '"'];
		value_pair = (string_ > ':' > value_)[qi::_val = phx::construct<std::pair<std::string,boost::any>>(qi::_1, qi::_2)];
		object_ %= '{' > -(value_pair % ',') > '}';
		array_ %= '[' > -(value_ % ',') > ']';
	}
	qi::rule<Iter, boost::any(), ascii::space_type> json_text;
	qi::rule<Iter, boost::any(), ascii::space_type> value_;
	qi::rule<Iter, std::vector<boost::any>(), ascii::space_type> array_;
	qi::rule<Iter, std::string(), ascii::space_type> string_;
	qi::rule<Iter, std::map<std::string,boost::any>(), ascii::space_type> object_;
	qi::rule<Iter, std::pair<std::string,boost::any>(), ascii::space_type> value_pair;
	qi::rule<Iter, null_t(), ascii::space_type> null_;
	qi::rule<Iter, std::string()> escaped;
	qi::rule<Iter, std::uint32_t()> ucn_code;
	phx::function<ucn_to_utf8_t> ucn_to_utf8;
};
}
NAMESPACE_CIRCLE_END

inline boost::any qi_any_parse(const std::string& s) {
	circle::json::qi_any_grammar<std::string::const_iterator> g;
	boost::any v;
	auto eit = s.end();
	boost::spirit::qi::phrase_parse(s.begin(), eit, g, boost::spirit::ascii::space, v );
	return v;
}
