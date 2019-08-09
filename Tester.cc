#include "Tester.hh"
#include "Parser.hh"
#include "Object.hh"

int main (int argc, char** argv) {
  if (1 == argc) {
    Parser::specialCases["map_area_data{"] = "map_area_data={";
    Parser::specialCases["\\\""] = "'";
    Object* testObject = processFile("testfile.txt");
    std::cout << (*testObject) << std::endl;
  }
  else {    
    Object* testObject = processFile(argv[1]); 
    assert(testObject);
    std::cout << "Successfully parsed " << Object::numObjects << " objects."
              << std::endl;
    std::cout << (*testObject) << std::endl; 
  }
  return 0; 
}
