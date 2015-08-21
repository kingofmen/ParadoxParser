#include "Parser.hh"
#include "Object.hh"

Object* Parser::topLevel = new Object("topLevel");
std::vector<Object*> stack;
std::vector<std::string> keystack;
std::vector<std::string> taglist;
std::ostream* Parser::outstream = &std::cout;
const std::string Parser::UnkeyedObjectMarker = "u_n_k_e_y_e_d_o_b_j_e_c_t";
bool Parser::abortOnBadObject = false;
std::string Parser::ignoreString("");

void setOutputStream (std::ostream* newos) {Parser::outstream = newos;}

void printStack (std::string source) {
  (*Parser::outstream) << source << " : \n";
  int indent = 1;
  for (objiter s = stack.begin(); s != stack.end(); ++s) {
    for (int i = 0; i < indent; ++i) (*Parser::outstream) << "  ";
    (*Parser::outstream) << (*s)->getKey() << "\n";
    indent++;
  }
}

int trim (std::string& str) {
  // Returns net number of braces opened by str, and trims leading and trailing whitespace.

  const char* s = str.c_str();
  int ret = 0;
  unsigned int strSize = str.size();
  if (0 == strSize) return 0;
  bool isInLiteral = false;
  for (unsigned int i = 0; i < strSize; ++i) {
    if ('"' == s[i]) isInLiteral = !isInLiteral;
    if (isInLiteral) continue;
    if ('{' == s[i]) ++ret;
    else if ('}' == s[i]) --ret;
  }

  unsigned int first = 0;
  for (; first < strSize; ++first) {
    if (s[first] == ' ') continue;
    if (13 == (int) s[first]) continue; // Carriage return
    break;
  }

  unsigned int last = strSize - 1;
  for (; last > first; --last) {
    if (' ' == s[last]) continue;
    if (13 == (int) s[last]) continue;
    break;
  }

  unsigned int fcomment = 0;
  isInLiteral = false;
  for (; fcomment < strSize; ++fcomment) {
    if ('"' == s[fcomment]) isInLiteral = !isInLiteral;
    if (isInLiteral) continue;

    if (s[fcomment] != '#') continue;
    break;
  }
  if (fcomment < last) last = fcomment-1;

  str = str.substr(first, last - first + 1);
  return ret;
}

void readFile (std::ifstream& read) {
  clearStack();
  int count = 0;
  int prevCount = 0;
  int openBraces = 0;
  std::string currObject;
  bool topLevel = true;

  while (!read.eof()) {
    // Until end of file: Slurp complete objects, making them whenever the openBraces count hits zero.
    std::string buffer;
    std::getline(read, buffer);
    count++;
    int currBraces = trim(buffer);
    if ((Parser::ignoreString != "") && (Parser::ignoreString == buffer)) continue;
    openBraces += currBraces;

    if (openBraces < 0) {
      // Apparently some files have a dangling close-brace?
      // Anyway, this indicates some sort of error. Try to
      // continue anyway by discarding the current object.
      (*Parser::outstream) << "Discarding object \n" << currObject << "\n" << buffer << "\ndue to additional close-braces.\n";
      currObject = "";
      continue;
    }

    currObject += " " + buffer;

    if (openBraces > 0) {
      topLevel = false;
      continue;
    }
    if (currObject == "") continue;

    // Don't try to end an object that hasn't started properly;
    // accounts for such constructions as
    // object =
    // {
    // where openBraces is zero after the first line of the object.
    // Not a problem for non-top-level objects since these will have
    // nonzero openBraces anyway.
    if (topLevel) continue;

    openBraces = 0;
    if (count - prevCount > 50000) {
      (*Parser::outstream) << "Parsed to line " << count << "\n";
      prevCount = count;
    }
    makeObject(currObject);
    currObject.clear();
    topLevel = true;
  }

  trim(currObject);
  if ((currObject.size() > 0) && (!makeObject(currObject))) {
    (*Parser::outstream) << "Warning: Unable to parse file; problem is with \"" << currObject << "\"" << std::endl;
    if (Parser::abortOnBadObject) {
      (*Parser::outstream) << "Aborting, unable to parse file" << std::endl;
      abort();
    }
  }
}


void clearStack () {
  if (0 < stack.size()) (*Parser::outstream) << "Warning: Clearing stack size "
				  << stack.size()
				  << ". This should not happen in normal operation."
				  << std::endl;
  for (std::vector<Object*>::iterator i = stack.begin(); i != stack.end(); ++i) {
    //(*Parser::outstream) << (*i)->getKey() << std::endl;
    (*Parser::outstream) << (*(*i)) << std::endl;
  }
  stack.clear();
}

void objectBegins (char /*ch*/) {
  Object* latest = 0;
  if (keystack.empty()) {
    latest = new Object(Parser::UnkeyedObjectMarker);
  }
  else {
    latest = new Object(keystack.back());
    keystack.pop_back();
  }
  if (stack.empty()) Parser::topLevel->setValue(latest);
  stack.push_back(latest);
}

void objectEnds (char /*ch*/) {
  Object* target = stack.back();
  stack.pop_back();
  if (!keystack.empty()) {
    target->setObjList();
    target->addToList(keystack.back());
    keystack.pop_back();
    for (std::vector<std::string>::iterator tag = taglist.begin(); tag != taglist.end(); ++tag) {
      target->addToList(*tag);
    }
    taglist.clear();
  }

  // If it belongs to the top-level object it has already been put there.
  if (!stack.empty()) {
    Object* p = stack.back();
    p->setValue(target);
  }
}

void addTag (char const* first, char const* last) {
  std::string tag(first, last);
  taglist.push_back(tag);
}

void setLHS (char const* first, char const* last) {
  std::string key(first, last);
  keystack.push_back(key);
}

void setRHSleaf (char const* first, char const* last) {
  //printStack("setRHSleaf");
  Object* target = (stack.empty() ? Parser::topLevel : stack.back());
  std::string val(first, last);
  target->setLeaf(keystack.back(), val);
  keystack.pop_back();
}

bool makeObject (std::string& command) {
  //(*Parser::outstream) << "Entering makeObject with " << command << std::endl;
  static Parser p;
  //BOOST_SPIRIT_DEBUG_GRAMMAR(p);
  boost::spirit::parse_info<> result = boost::spirit::parse(command.c_str(), p, boost::spirit::nothing_p);
  if (result.full) {
    //(*Parser::outstream) << "Matched " << command << std::endl;
    // Do stuff with full command

    //command.clear();
    return true;
  }

  //(*Parser::outstream) << "No match in " << command << " stopped at \"" << result.stop << "\"" << std::endl;
  return false;
}




