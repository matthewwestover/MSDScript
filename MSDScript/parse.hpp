#ifndef parse_hpp
#define parse_hpp

#include <iostream>
#include "env.hpp"

class Expr;
PTR(Expr) parse(std::istream &in);

#endif /* parse_hpp */

