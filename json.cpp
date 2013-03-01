#include <boost/any.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/fusion/include/std_pair.hpp> 

#include <vector>
#include <map>
#include <iostream>
#include <utility>
#include <deque>

namespace json {

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phx = boost::phoenix;

struct nullptr_t_ : qi::symbols< char, void * > {
	nullptr_t_() {
		add( "null", nullptr );
	}
} nullptr_;
	struct ucn_to_utf8_t {
		template <typename, typename>
		struct result { typedef bool type; };
		bool operator ()(uint32_t code, std::string &res) const {
			std::deque<char> seq;
			unsigned char head;
			if (0xD800 <= code && code < 0xE000) return false;
			else if (code >= 0x04000000) head = 0xFC | ((code>>30) & 0x01);
			else if (code >= 0x00200000) head = 0xF8 | ((code>>24) & 0x03);
			else if (code >= 0x00010000) head = 0xF0 | ((code>>18) & 0x07);
			else if (code >= 0x00000800) head = 0xE0 | ((code>>12) & 0x0F);
			else if (code >= 0x00000080) head = 0xC0 | ((code>> 6) & 0x1F);
			else head = code;
			while (code >= 0x80) {
				seq.push_front(0x80 | (code&0x3F));
				code >>= 6;
			}
			seq.push_front(head);
			res.assign(seq.begin(), seq.end());
			return true;
		}
	};

	template <typename Iter>
	struct Grammar: qi::grammar<Iter, boost::any(), ascii::space_type> {
		Grammar(): Grammar::base_type(json_text) {
			json_text %= object_ | array_;
			value_ %= array_ | string_ | (qi::int_parser<int64_t>() >> !qi::lit('.')) | qi::double_ | object_ | qi::bool_ | null_;
			null_ = qi::lit("null")[qi::_val = phx::construct<nullptr_t_>()];
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
		qi::rule<Iter, nullptr_t_(), ascii::space_type> null_;
		qi::rule<Iter, std::string()> escaped;
		qi::rule<Iter, std::uint32_t()> ucn_code;
		phx::function<ucn_to_utf8_t> ucn_to_utf8;
	};


}

inline boost::any test_parse(const std::string& s) {
	json::Grammar< std::string::const_iterator > g;
	boost::any v;
	auto eit = s.end();
	boost::spirit::qi::phrase_parse(s.begin(), eit, g, boost::spirit::ascii::space, v );
	return v;
}

inline void test(const std::stringstream& ss){
	boost::any v = std::move(test_parse(ss.str()));
	auto a = boost::any_cast<std::vector<boost::any>>(boost::any_cast<std::map<std::string,boost::any>>(v)["entries"]);
	int i, l = a.size();
	for(i = 0; i< l; i++) {
		std::cout << boost::any_cast<std::string>(boost::any_cast<std::map<std::string,boost::any>>(a[i])["title"]) << std::endl;
	}
}

#ifdef TTT
#include <fstream>
#include <sstream>
int main(){
  std::stringstream ss;
  std::ifstream f;
  f.open("hoge.json", std::ios::binary);
  ss << f.rdbuf();
  f.close();
  
  test(ss);
}
#endif
