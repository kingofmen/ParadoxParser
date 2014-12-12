#include "Object.hh"
#include "Parser.hh"
#include <sstream> 

bool Object::debug = false; 

Object::Object (std::string k) :
  objects(),
  strVal(),
  leaf(false),
  isObjList(false)
{
  key = k; 
}

Object::~Object () {
  for (objiter i = objects.begin(); i != objects.end(); ++i) {
    delete (*i); 
  }
  if (br == this) br = 0; 
}

Object::Object (Object* other) :
  objects(),
  strVal(other->strVal),
  leaf(other->leaf), 
  isObjList(other->isObjList)
{
  key = other->key; 
  for (std::vector<Object*>::iterator i = other->objects.begin(); i != other->objects.end(); ++i) {
    objects.push_back(new Object(*i)); 
  }
}

void Object::setValue (std::string val) {
  strVal = val; 
  leaf = true; 
}

void Object::setValue (Object* val, Object* beforeThis) {
  leaf = false;
  if (beforeThis) {
    for (objiter o = objects.begin(); o != objects.end(); ++o) {
      if (beforeThis != (*o)) continue;
      objects.insert(o, val); 
      return;  
    }
  }
  objects.push_back(val); 
}

void Object::unsetValue (std::string val) {
  for (unsigned int i = 0; i < objects.size(); ++i) {
    if (objects[i]->getKey() != val) continue; 
    objects[i] = objects.back(); 
    objects.pop_back();
    --i; 
  }
}

void Object::unsetKeyValue (std::string key, std::string val) {
  for (unsigned int i = 0; i < objects.size(); ++i) {
    if (objects[i]->getKey() != key) continue;
    if (objects[i]->getLeaf() != val) continue; 
    objects[i] = objects.back(); 
    objects.pop_back();
    --i; 
  }
}

void Object::setLeaf (std::string key, std::string val) {
  Object* leaf = new Object(key); 
  leaf->setValue(val); 
  setValue(leaf); 
}

void Object::setLeaf (std::string key, int val) {
  Object* leaf = new Object(key);
  static char strbuffer[1000];
  sprintf(strbuffer, "%i", val); 
  leaf->setValue(strbuffer); 
  setValue(leaf); 
}

void Object::setLeaf (std::string key, double val) {
  Object* leaf = new Object(key);
  static char strbuffer[1000];
  sprintf(strbuffer, "%f", val); 
  leaf->setValue(strbuffer); 
  setValue(leaf); 
}

void Object::resetLeaf (std::string key, std::string val) {
  Object* curr = safeGetObject(key);
  if (curr) curr->setValue(val);
  else setLeaf(key, val); 
}

void Object::resetLeaf (std::string key, int val) {
  Object* curr = safeGetObject(key);
  if (curr) {
    static char strbuffer[1000];
    sprintf(strbuffer, "%i", val); 
    curr->setValue(strbuffer); 
  }
  else setLeaf(key, val); 
}

void Object::resetLeaf (std::string key, double val) {
  Object* curr = safeGetObject(key);
  if (curr) {
    static char strbuffer[1000];
    sprintf(strbuffer, "%f", val);
    curr->setValue(strbuffer);
  }
  else setLeaf(key, val); 
}

void Object::remToken (std::string val) {
  strVal = "";
  std::vector<std::string> oldtokens = tokens;
  tokens.clear(); 
  for (unsigned int i = 0; i < oldtokens.size(); ++i) {
    if (oldtokens[i] == val) continue;
    addToList(oldtokens[i]); 
  }
}

void Object::resetToken (unsigned int idx, std::string val) {
  strVal = "";
  std::vector<std::string> oldtokens = tokens;
  tokens.clear(); 
  for (unsigned int i = 0; i < oldtokens.size(); ++i) {
    if (idx == i) {
      if (val != "") addToList(val);
    }
    else addToList(oldtokens[i]); 
  }
}

void Object::setValue (std::vector<Object*> val) {
  objects = val; 
}

void Object::addToList(std::string val) {
  isObjList = true;
  strVal += " ";
  strVal += val; 
  tokens.push_back(val); 
}

void Object::addToList(double val) {
  static char strbuffer[1000];
  sprintf(strbuffer, "%f", val);
  addToList(strbuffer);  
}

void Object::addToList(int val) {
  static char strbuffer[1000];
  sprintf(strbuffer, "%i", val);
  addToList(strbuffer);
}

std::vector<Object*> Object::getValue (std::string key) const {
  std::vector<Object*> ret; 
  for (std::vector<Object*>::const_iterator i = objects.begin(); i != objects.end(); ++i) {
    if ((*i)->getKey() != key) continue;
    ret.push_back(*i); 
  }
  return ret; 
}

std::string Object::getToken (int index) {
  if (!isObjList) return "";
  if (index >= (int) tokens.size()) return ""; 
  if (index < 0) return ""; 
  return tokens[index]; 
}

int Object::tokenAsInt (int index) {
  if (!isObjList) return 0;
  if (index >= (int) tokens.size()) return 0; 
  if (index < 0) return 0; 
  return atoi(tokens[index].c_str()); 
}

double Object::tokenAsFloat (int index) {
  if (!isObjList) return 0;
  if (index >= (int) tokens.size()) return 0; 
  if (index < 0) return 0; 
  return atof(tokens[index].c_str()); 
}

int Object::numTokens () const {
  if (!isObjList) return 0;
  return tokens.size(); 
}

std::vector<std::string> Object::getKeys () {
  std::vector<std::string> ret;
  for (std::vector<Object*>::iterator i = objects.begin(); i != objects.end(); ++i) {
    std::string curr = (*i)->getKey();
    if (std::find(ret.begin(), ret.end(), curr) != ret.end()) continue;
    ret.push_back(curr);
  }
  return ret; 
}

std::string Object::getLeaf (std::string leaf) const {
  std::vector<Object*> leaves = getValue(leaf); 
  if (0 == leaves.size()) {
    (*Parser::outstream) << "Error: Cannot find leaf " << leaf << " in object " << std::endl << *this;
    assert(leaves.size()); 
  }
  return leaves[0]->getLeaf(); 
}

std::ostream& operator<< (std::ostream& os, const Object& obj) {
  static int indent = 0; 
  for (int i = 0; i < indent; i++) {
    os << "  "; 
  }
  if (obj.leaf) {
    os << obj.key << " = " << obj.strVal;
    if (obj.comment.size()) os << " # " << obj.comment;
    os << "\n"; 
    return os; 
  }

  if (obj.key == Parser::ObjectListMarker) {
    os << "{\n";
    indent++;
    for (std::vector<Object*>::const_iterator i = obj.objects.begin(); i != obj.objects.end(); ++i) {
      os << *(*i); 
    }
    indent--;
    for (int i = 0; i < indent; i++) {
      os << "  "; 
    }
    os << "}";
    if (obj.comment.size()) os << " # " << obj.comment;
    os << "\n";
    return os; 
  }
  
  if (obj.isObjList) {
    os << obj.key << " = { ";
    if (0 < obj.numTokens()) {
      os << obj.strVal << " }";
      if (obj.comment.size()) os << " # " << obj.comment;
      os << "\n";
    } 
    else {
      os << "\n";
      indent++;
      for (std::vector<Object*>::const_iterator i = obj.objects.begin(); i != obj.objects.end(); ++i) {
	os << *(*i); 
      }
      indent--;
      for (int i = 0; i < indent; i++) {
	os << "  "; 
      }
      os << "}";
      if (obj.comment.size()) os << " # " << obj.comment;
      os << "\n";
    }
    return os; 
  }

  if (&obj != Parser::topLevel) {
    os << obj.key << " = { \n";
    indent++;
  }
  for (std::vector<Object*>::const_iterator i = obj.objects.begin(); i != obj.objects.end(); ++i) {
    os << *(*i); 
  }
  if (&obj != Parser::topLevel) {
    indent--; 
    for (int i = 0; i < indent; i++) {
      os << "  "; 
    }
    os << "}";
    if (obj.comment.size()) os << " # " << obj.comment;
    os << "\n";
  }
  return os; 
}

void Object::keyCount () {
  if (leaf) {
    (*Parser::outstream) << key << " : 1\n";
    return;
  }
  
  std::map<std::string, int> refCount;
  keyCount(refCount); 
  std::vector<std::pair<std::string, int> > sortedCount; 
  for (std::map<std::string, int>::iterator i = refCount.begin(); i != refCount.end(); ++i) {
    std::pair<std::string, int> curr((*i).first, (*i).second); 
    if (2 > curr.second) continue; 
    if ((0 == sortedCount.size()) || (curr.second <= sortedCount.back().second)) {
      sortedCount.push_back(curr); 
      continue; 
    }

    for (std::vector<std::pair<std::string, int> >::iterator j = sortedCount.begin(); j != sortedCount.end(); ++j) {
      if (curr.second < (*j).second) continue; 
      sortedCount.insert(j, 1, curr);
      break; 
    }
  }
  
  for (std::vector<std::pair<std::string, int> >::iterator j = sortedCount.begin(); j != sortedCount.end(); ++j) {
    (*Parser::outstream) << (*j).first << " : " << (*j).second << "\n"; 
  }

}

void Object::keyCount (std::map<std::string, int>& counter) {
  for (std::vector<Object*>::iterator i = objects.begin(); i != objects.end(); ++i) {
    counter[(*i)->key]++;
    if ((*i)->leaf) continue; 
    (*i)->keyCount(counter); 
  }
}

void Object::printTopLevel () {
  for (std::vector<Object*>::iterator i = objects.begin(); i != objects.end(); ++i) {
    (*Parser::outstream) << (*i)->key << std::endl; 
  }
}
void Object::removeObject (Object* target) {
  std::vector<Object*>::iterator pos = std::find(objects.begin(), objects.end(), target);
  if (pos == objects.end()) return; 
  objects.erase(pos); 
}

Object* br = 0;
void setVal (std::string name, std::string val, Object* branch) {
  if ((branch) && (br != branch)) br = branch; 
  Object* b = new Object(name);
  b->setValue(val); 
  br->setValue(b); 
}

void setInt (std::string name, int val, Object* branch) {
  if ((branch) && (br != branch)) br = branch; 
  static char strbuffer[1000];
  sprintf(strbuffer, "%i", val); 
  Object* b = new Object(name);
  b->setValue(strbuffer); 
  br->setValue(b); 
}

void setFlt (std::string name, double val, Object* branch) {
  if ((branch) && (br != branch)) br = branch; 
  static char strbuffer[1000];
  sprintf(strbuffer, "%.3f", val); 
  Object* b = new Object(name);
  b->setValue(strbuffer); 
  br->setValue(b); 
}

void processIncludes (Object* ret) {
  objvec empty; 
  objvec includes = ret->getValue("include"); 
  for (objiter i = includes.begin(); i != includes.end(); ++i) {
    Object* temp = processFile((*i)->getLeaf()); 
    objvec nleaves = temp->getLeaves(); 
    for (objiter nl = nleaves.begin(); nl != nleaves.end(); ++nl) {
      ret->setValue(*nl); 
    }
    temp->setValue(empty); 
    delete temp; 
  }
}

Object* processFile (std::string filename, bool includes) {
  Parser::topLevel = new Object("toplevel"); 
  std::ifstream read;
  read.open(filename.c_str());
  readFile(read);
  read.close();
  Object* ret = Parser::topLevel; 
  if (includes) {
    processIncludes(ret); 
    Parser::topLevel = ret; 
  }
  return ret; 
}

Object* processFile (const char* filename, bool includes) {
  Parser::topLevel = new Object("toplevel"); 
  std::ifstream read;
  read.open(filename);
  readFile(read);
  read.close();
  Object* ret = Parser::topLevel; 
  if (includes) {
    processIncludes(ret); 
    Parser::topLevel = ret; 
  }
  return ret;  
}

double Object::safeGetFloat(std::string k, double def) {
  objvec vec = getValue(k); 
  if (0 == vec.size()) return def;
  return atof(vec[0]->getLeaf().c_str());
}

std::string Object::safeGetString(std::string k, std::string def) {
  objvec vec = getValue(k); 
  if (0 == vec.size()) return def;
  return vec[0]->getLeaf(); 
}

int Object::safeGetInt(std::string k, int def) {
  objvec vec = getValue(k); 
  if (0 == vec.size()) return def;
  return atoi(vec[0]->getLeaf().c_str());
}

unsigned int Object::safeGetUint(std::string k, unsigned int def) {
  objvec vec = getValue(k);
  if (0 == vec.size()) return def;
  int ret = atoi(vec[0]->getLeaf().c_str());
  assert(0 <= ret);
  return (unsigned int) ret;
}

Object* Object::safeGetObject(std::string k, Object* def) {
  objvec vec = getValue(k); 
  if (0 == vec.size()) return def; 
  return vec[0]; 
}

Object* Object::getNeededObject (std::string k) {
  Object* ret = safeGetObject(k);
  if (ret) return ret;
  ret = new Object(k);
  setValue(ret);
  return ret; 
}


std::string Object::toString () const {
  std::ostringstream blah;
  blah << *(this); 
  return blah.str(); 
}

bool Object::isNumeric () const {
  if (!leaf) return false;
  if (isObjList) return false; 
  for (std::string::const_iterator i = strVal.begin(); i != strVal.end(); ++i) {
    if ((*i) == ' ') continue;
    if ((*i) == '.') continue;
    if (!isdigit(*i)) return false;
  }
  return true; 
}
