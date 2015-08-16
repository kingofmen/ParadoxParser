#ifndef PARSER_HH
#define PARSER_HH

//#define BOOST_SPIRIT_DEBUG 1
//#define BOOST_SPIRIT_DEBUG_FLAGS BOOST_SPIRIT_DEBUG_FLAGS_NODES

#define BOOST_SPIRIT_USE_OLD_NAMESPACE 1

#undef BOOST_SPIRIT_DEBUG
#undef BOOST_SPIRIT_DEBUG_FLAGS

#include "boost/spirit/include/classic.hpp"
#include "boost/spirit/include/classic_ast.hpp"
#include <string>
#include "Object.hh"
#include <iostream>
#include <fstream>

using namespace boost::spirit;

void addTag (char const* first, char const* last);
void objectBegins (char /*ch*/);
void objectEnds (char /*ch*/);
void setLHS (char const* first, char const* last);
void setRHSleaf (char const* first, char const* last);

struct Parser : public boost::spirit::grammar<Parser> {
  static Object* topLevel;
  static std::ostream* outstream;
  static const std::string UnkeyedObjectMarker;
  static bool abortOnBadObject;

  static const unsigned int OBJECT  = 1;
  static const unsigned int LEAF    = 2;
  static const unsigned int ASSIGN  = 3;
  static const unsigned int STR     = 4;

  template<typename ScannerT> struct definition {
    boost::spirit::rule<ScannerT, parser_context<>, parser_tag<LEAF> >     leaf;
    boost::spirit::rule<ScannerT, parser_context<>, parser_tag<ASSIGN> >   assign;
    boost::spirit::rule<ScannerT, parser_context<>, parser_tag<OBJECT> >   object;
    boost::spirit::rule<ScannerT, parser_context<>, parser_tag<STR> >      str;

    definition (Parser const& self) {
      str     = ch_p('"') >> *(anychar_p - ch_p('"') ) >> ch_p('"');
      leaf    = ( eps_p( ((anychar_p - ch_p('=')) - ch_p('{') ) - ch_p('}') - ch_p('"') ) >> *(graph_p - ch_p('=') - ch_p('}')));
      object  = *(blank_p) >> ch_p('{')[&objectBegins] >> *(blank_p)
			   >> *(assign | object)
			   >> *(blank_p) >> ch_p('}')[&objectEnds];
      assign  = +((*blank_p >> ( leaf[&setLHS] | str[&setLHS] ) >> *blank_p >>
		   ((ch_p('=') >> *blank_p >> ( leaf[&setRHSleaf] | str[&setRHSleaf] | object ) >> *blank_p) |
		    *((leaf[&addTag] | str[&addTag]) >> *blank_p))
		   >> *blank_p));

      /*
      BOOST_SPIRIT_DEBUG_RULE(leaf);
      //BOOST_SPIRIT_DEBUG_RULE(taglist);
      BOOST_SPIRIT_DEBUG_RULE(assign);
      BOOST_SPIRIT_DEBUG_RULE(object);
      BOOST_SPIRIT_DEBUG_RULE(str);
      */
    }

    boost::spirit::rule<ScannerT, parser_context<>, parser_tag<ASSIGN> > const& start() const { return assign; }
  };

private:

};

void clearStack ();
bool makeObject (std::string& command);
void readFile (std::ifstream& read);
int trim (std::string& str);
void setOutputStream (std::ostream* newos);

#endif
