#ifndef OBJECT_HH
#define OBJECT_HH

#include <map> 
#include <vector>
#include <string> 
#include <iostream>

class Object {
  friend std::ostream& operator<< (std::ostream& o, const Object& i);

public:
  Object (std::string k);
  ~Object (); 
  Object (Object* other);

  static bool debug;
  
  void clear () {objects.clear(); tokens.clear(); strVal.clear();} 
  void setValue (Object* val, Object* beforeThis = 0);
  void setValue (std::string val);
  void setValue (std::vector<Object*> val);
  std::string getKey () {return key;}
  void setKey (std::string k) {key = k;} 
  std::vector<std::string> getKeys (); 
  std::vector<Object*> getValue (std::string key) const;
  std::string getLeaf () const {return strVal;}
  std::string getLeaf (std::string leaf) const;
  std::vector<Object*> getLeaves () {return objects;}
  void removeObject (Object* target); 
  void setLeaf (std::string k, std::string value);
  void setLeaf (std::string k, int value);
  void setLeaf (std::string k, unsigned int value);
  void setLeaf (std::string k, double value);
  void resetLeaf (std::string k, std::string value);
  void resetLeaf (std::string k, int value);
  void resetLeaf (std::string k, unsigned int value);
  void resetLeaf (std::string k, double value);
  void unsetValue (std::string val);
  void unsetKeyValue (std::string key, std::string value); 
  void keyCount ();
  void keyCount (std::map<std::string, int>& counter);
  void setObjList (bool l = true) {isObjList = l;}
  std::string getToken (int index);
  int tokenAsInt (int index);
  double tokenAsFloat (int index);
  void remToken (std::string val); 
  void resetToken (unsigned int idx, std::string val);
  int numTokens () const; 
  void addToList (std::string val);
  void addToList (double val);
  void addToList (int val);   
  void printTopLevel ();
  inline bool isLeaf () {return leaf;}
  double safeGetFloat (std::string k, double def = 0);
  std::string safeGetString (std::string k, std::string def = ""); 
  int safeGetInt (std::string k, int def = 0);
  unsigned int safeGetUint (std::string k, unsigned int def = 0); 
  Object* safeGetObject (std::string k, Object* def = 0);
  Object* getNeededObject (std::string k); // Returns the object with key 'k', creating and adding it if necessary. 
  std::string toString () const; 
  void setComment (std::string c) {comment = c;} 
  bool isNumeric () const;
  
private:
  std::vector<Object*> objects;
  std::string strVal;
  std::string key; 
  bool leaf; 
  bool isObjList;
  std::vector<std::string> tokens; // For use in lists.

  std::string comment; 
};

extern std::ostream& operator<< (std::ostream& os, const Object& i);
extern Object* br; 
extern void setVal (std::string name, std::string val, Object* branch = 0);
extern void setInt (std::string name, int val, Object* branch = 0);
extern void setFlt (std::string name, double val, Object* branch = 0);
typedef std::vector<Object*>::iterator objiter;
typedef std::vector<Object*> objvec; 
typedef std::map<std::string, Object*> stobmap;
typedef std::map<std::string, std::string> ststmap;
typedef std::map<Object*, Object*> obobmap;
extern Object* processFile (const char* filename, bool includes = false);
extern Object* processFile (std::string filename, bool includes = false);

#endif
