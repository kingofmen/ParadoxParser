#include "Parser.hh"
#include "Object.hh"

Object* Parser::topLevel = new Object("topLevel"); 
std::vector<Object*> stack; 
std::vector<Object*> objstack; 
std::ostream* Parser::outstream = &std::cout;
const std::string Parser::ObjectListMarker = "o_b_j_l_i_s_t_e_n_t_r_y";
bool Parser::abortOnBadObject = false; 

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
  for (; fcomment < strSize; ++fcomment) {
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

void setLHS (char const* first, char const* last) {
  std::string key(first, last); 
  Object* p = new Object(key); 
  if (0 == stack.size()) {
    Parser::topLevel->setValue(p);
  }
  stack.push_back(p);
}

void setRHSleaf (char const* first, char const* last) {
  //printStack("setRHSleaf");
  
  std::string val(first, last); 
  Object* l = stack.back();
  stack.pop_back(); 
  l->setValue(val);
  if (0 < stack.size()) {
    Object* p = stack.back(); 
    p->setValue(l); 
  }
}

void setRHStaglist (char const* first, char const* last) {
  Object* l = stack.back();
  stack.pop_back(); 

  std::string val(first, last); 
  std::string tag;
  bool stringmode = false; 
  for (unsigned int i = (val[0] == '{') ? 1 : 0; i < val.size(); ++i) {
    if ((val[i] == ' ') && (0 == tag.size())) continue; 
    if (val[i] == '"') {
      tag.push_back(val[i]); 
      if (stringmode) {
	l->addToList(tag); 
	tag.clear(); 
      }
      stringmode = !stringmode;
      continue;
    }
    if (((val[i] == ' ') && (!stringmode)) || (val[i] == '}')) {
      if (tag.size() > 0) l->addToList(tag); 
      tag.clear(); 
      continue;
    }
    tag.push_back(val[i]); 
  }

  if (0 < stack.size()) {
    Object* p = stack.back(); 
    p->setValue(l); 
  }
}

void setRHSobject (char const* first, char const* last) {
  //printStack("setRHSobject");
  // No value is set, a bunch of leaves have been defined. 
  Object* l = stack.back();
  stack.pop_back();
  if (0 < stack.size()) {
    Object* p = stack.back(); 
    p->setValue(l); 
  }
}

void pushObj (char const* first, char const* last) {
  //printStack("pushObj");
  Object* p = new Object(Parser::ObjectListMarker); 
  objstack.push_back(p);

  // The internal (unkeyed) object does not get pushed onto the stack.
  // Therefore the top of the stack is the keyed object that consists
  // of the object list under construction. Any objects that were completed
  // while this one was under construction, ie assigns internal to this
  // object, were mistakenly pushed onto that top-level object. So we
  // should move them into this one. 

  Object* outer = stack.back(); 
  objvec leaves = outer->getLeaves();
  for (objiter l = leaves.begin(); l != leaves.end(); ++l) {
    p->setValue(*l); 
  }
  outer->clear(); 
}

void setRHSobjlist (char const* first, char const* last) {
  //printStack("setRHSobjlist");
  if (0 < stack.size()) {
    Object* p = stack.back();
    p->setObjList(); 
    p->setValue(objstack);
    stack.pop_back();
    if (0 < stack.size()) {
      stack.back()->setValue(p); 
    }
  }
  objstack.clear(); 
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




