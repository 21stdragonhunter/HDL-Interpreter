#include <iostream>
#include "vector"
#include "string"

vector<int> toBinary(int decimal) {
  
  vector<int> returnVector;
  vector<int> reversedVector;
  
  while(decimal > 0) {
    
    reversedVector.push_back(decimal % 2);
    decimal /= 2;
    
  }
  
  for(int i = reversedVector.size(); i > 0; i--) {
    
    returnVector.push_back(reversedVector[i-1]);
    
  }
  
  return returnVector;
  
}
