#ifndef env_hpp
#define env_hpp

#include "macros.hpp"
#include <string>

class Val;

// Creates a "dictionary" of bound variables for quicker calculation
// lookup stores values of bound variables, errors out when unbound is foun
class Env ENABLE_THIS(Env) {
public:
    virtual PTR(Val) lookup(std::string find_name) = 0;
};

class EmptyEnv : public Env {
public:
    EmptyEnv();
    PTR(Val) lookup(std::string find_name);
};

class ExtendedEnv : public Env {
public:
    std::string name;
    PTR(Val) val;
    PTR(Env) rest;
    
    ExtendedEnv(std::string name, PTR(Val) val, PTR(Env) rest);
    PTR(Val) lookup(std::string find_name);
};

#endif /* env_hpp */
