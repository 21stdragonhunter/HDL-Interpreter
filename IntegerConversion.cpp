#include <iostream>
#include "vector"
#include "string"

vector<int> toBinary(int decimal) {
  
  vector<int> returnVector;
  vector<int> reverseVector;
  
  while(decimal > 0) {
    
    reversedVector.append(decimal % 2);
    decimal /= 2;
    
  }
  
  for(int i = reversedVector.length; i > 0; i--) {
    
    returnVector.append(reversedVector[i-1]);
    
  }
  
  return returnVector;
  
}
