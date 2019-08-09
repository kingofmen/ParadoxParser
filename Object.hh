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
  static int numObjects;
  
  void clear () {objects.clear(); tokens.clear(); strVal.clear();} 
  void setValue (Object* val, Object* beforeThis = 0);
  void setValue (std::string val);
  void setValue (std::vector<Object*> val);
  std::string getKey () const {return key;}
  void setKey (std::string k) {key = k;} 
  std::vector<std::string> getKeys () const;
  std::vector<Object*> getValue (std::string key) const;
  std::string getLeaf () const {return strVal;}
  double getLeafAsFloat() const;
  int getLeafAsInt() const;
  std::string getLeaf (std::string leaf) const;
  std::vector<Object*> getLeaves () const {return objects;}
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
  void keyCount () const;
  void keyCount (std::map<std::string, int>& counter) const;
  void setObjList (bool l = true) {isObjList = l;}
  std::string getToken (int index) const;
  int tokenAsInt (int index) const;
  double tokenAsFloat (int index) const;
  void remToken (std::string val); 
  void resetToken (unsigned int idx, std::string val);
  int numTokens () const;
  void addToList (std::string val);
  void addToList (double val);
  void addToList (int val);   
  void printTopLevel ();
  inline bool isLeaf () const {return leaf;}
  double safeGetFloat (std::string k, double def = 0) const;
  std::string safeGetString (std::string k, std::string def = "") const;
  int safeGetInt (std::string k, int def = 0) const;
  unsigned int safeGetUint (std::string k, unsigned int def = 0) const;
  Object* safeGetObject (std::string k, Object* def = 0) const;
  Object* getNeededObject (std::string k); // Returns the object with key 'k', creating and adding it if necessary. 
  std::string toString () const; 
  void setComment (std::string c);
  std::string getComment () const;
  bool isNumeric () const;
  
private:
  std::vector<Object*> objects;
  std::string strVal;
  std::string key; 
  bool leaf; 
  bool isObjList;
  std::vector<std::string> tokens; // For use in lists.

  static std::map<const Object*, std::string> commentMap;
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
