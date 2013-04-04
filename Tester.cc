#include "Tester.hh"
#include "Parser.hh"
#include "Object.hh"

int main (int argc, char** argv) {
  if (1 == argc) {
    Object* testObject = processFile("testfile.txt"); 
    std::cout << (*testObject) << std::endl; 
  }
  else {    
    Object* testObject = processFile(argv[1]); 
    assert(testObject); 
    std::cout << "Successfully parsed" << std::endl;
    std::cout << (*testObject) << std::endl; 
  }
  return 0; 
}
