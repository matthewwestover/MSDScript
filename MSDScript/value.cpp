#include <stdexcept>
#include "catch.hpp"
#include "value.hpp"
#include "expr.hpp"
#include "env.hpp"
#include "step.hpp"

NumVal::NumVal(int rep) {
    this->rep = rep;
}

bool NumVal::equals(PTR(Val) other_val) {
    PTR(NumVal) other_num_val = CAST(NumVal)(other_val);
    if (other_num_val == nullptr)
        return false;
    else
        return rep == other_num_val->rep;
}

bool NumVal::is_true() {
    throw std::runtime_error("numbers cannot be true/false");
}

PTR(Val) NumVal::add_to(PTR(Val) other_val) {
    PTR(NumVal) other_num_val = CAST(NumVal)(other_val);
    if (other_num_val == nullptr)
        throw std::runtime_error("not a number");
    else
        return NEW(NumVal)((unsigned)rep + (unsigned)other_num_val->rep);
}

PTR(Val) NumVal::mult_with(PTR(Val) other_val) {
    PTR(NumVal) other_num_val = CAST(NumVal)(other_val);
    if (other_num_val == nullptr)
        throw std::runtime_error("not a number");
    else
        return NEW(NumVal)((unsigned)rep * (unsigned)other_num_val->rep);
}

// Check if call should do something
PTR(Val) NumVal::call(PTR(Val) actual_arg) {
    throw std::runtime_error("cannot call on a number");
}

void NumVal::call_step(PTR(Val) actual_arg, PTR(Cont) rest) {
    throw std::runtime_error("cannot call on a number");
}

PTR(Expr) NumVal::to_expr() {
    return NEW(NumExpr)(rep);
}

std::string NumVal::to_string() {
    return std::to_string(rep);
}

BoolVal::BoolVal(bool rep) {
    this->rep = rep;
}

bool BoolVal::equals(PTR(Val) other_val) {
    PTR(BoolVal) other_bool_val = CAST(BoolVal)(other_val);
    if (other_bool_val == nullptr)
        return false;
    else
        return rep == other_bool_val->rep;
}

bool BoolVal::is_true() {
    return rep;
}

PTR(Val) BoolVal::add_to(PTR(Val) other_val) {
    throw std::runtime_error("no adding booleans");
}

PTR(Val) BoolVal::mult_with(PTR(Val) other_val) {
    throw std::runtime_error("no multiplying booleans");
}

// Check if call should do something
PTR(Val) BoolVal::call(PTR(Val) actual_arg) {
    throw std::runtime_error("cannot call on a boolean");
}

void BoolVal::call_step(PTR(Val) actual_arg, PTR(Cont) rest) {
    throw std::runtime_error("cannot call on a boolean");
}

PTR(Expr) BoolVal::to_expr() {
    return NEW(BoolExpr)(rep);
}

std::string BoolVal::to_string() {
    if (rep)
        return "_true";
    else
        return "_false";
}

FunVal::FunVal(std::string arg, PTR(Expr) body, PTR(Env) env) {
    this->formal_arg = arg;
    this->body = body;
    this->env = env;
}

bool FunVal::equals(PTR(Val) other_val) {
    PTR(FunVal) other_fun_val = CAST(FunVal)(other_val);
    if (other_fun_val == nullptr)
        return false;
    else
        return (formal_arg == other_fun_val->formal_arg && body->equals(other_fun_val->body));
}

bool FunVal::is_true() {
    throw std::runtime_error("functions cannot be true/false");
}

PTR(Val) FunVal::add_to(PTR(Val) other_val){
    throw std::runtime_error("no adding functions");
}

PTR(Val) FunVal::mult_with(PTR(Val) other_val) {
    throw std::runtime_error("no multiplying functions");
}

PTR(Val) FunVal::call(PTR(Val) actual_arg) {
    return body->interp(NEW(ExtendedEnv)(formal_arg, actual_arg, env));
}

void FunVal::call_step(PTR(Val) actual_arg, PTR(Cont) rest) {
    Step::mode = Step::interp_mode;
    Step::expr = body;
    Step::env = NEW(ExtendedEnv)(formal_arg, actual_arg, env);
    Step::cont = rest;
}

PTR(Expr) FunVal::to_expr() {
    return NEW(FunExpr)(this->formal_arg, this->body);
}

std::string FunVal::to_string() {
    return "_fun (" + this->formal_arg + ") " + this->body->to_string();
}

TEST_CASE( "values equals" ) {
    SECTION( "NumVal" ){
        CHECK( (NEW(NumVal)(5))
              ->equals(NEW(NumVal)(5)) );
        CHECK( ! (NEW(NumVal)(7))
              ->equals(NEW(NumVal)(5)) );
        CHECK( ! (NEW(NumVal)(7))
              ->equals(NEW(BoolVal)(false)) );
    }
    SECTION( "BoolVal" ) {
        CHECK( (NEW(BoolVal)(true))
              ->equals(NEW(BoolVal)(true)) );
        CHECK( ! (NEW(BoolVal)(true))
              ->equals(NEW(BoolVal)(false)) );
        CHECK( ! (NEW(BoolVal)(false))
              ->equals(NEW(BoolVal)(true)) );
        CHECK( ! (NEW(BoolVal)(false))
              ->equals(NEW(NumVal)(8)) );
    }
    SECTION( "FunVal" ) {
        CHECK( (NEW(FunVal)("x", NEW(NumExpr)(5), NEW(EmptyEnv)()))
              ->equals(NEW(FunVal)("x", NEW(NumExpr)(5), NEW(EmptyEnv)())) );
        CHECK( ! (NEW(FunVal)("y", NEW(NumExpr)(5), NEW(EmptyEnv)()))
              ->equals(NEW(FunVal)("x", NEW(NumExpr)(5), NEW(EmptyEnv)())) );
        CHECK( ! (NEW(FunVal)("x", NEW(NumExpr)(5), NEW(EmptyEnv)()))
              ->equals(NEW(FunVal)("x", NEW(NumExpr)(4), NEW(EmptyEnv)())) );
        CHECK( ! (NEW(FunVal)("x", NEW(NumExpr)(5), NEW(EmptyEnv)()))
              ->equals(NEW(NumVal)(5)));
    }
}

TEST_CASE( "add_to" ) {
    SECTION( "NumVal" ){
        CHECK ( (NEW(NumVal)(5))->add_to(NEW(NumVal)(8))
               ->equals(NEW(NumVal)(13)) );
        CHECK_THROWS_WITH ( (NEW(NumVal)(5))->add_to(NEW(BoolVal)(false)),
                           "not a number" );
    }
    SECTION( "BoolVal" ) {
        CHECK_THROWS_WITH ( (NEW(BoolVal)(false))->add_to(NEW(BoolVal)(false)),
                           "no adding booleans" );
    }
    SECTION( "FunVal" ) {
        CHECK_THROWS_WITH ( (NEW(FunVal)("x", NEW(NumExpr)(5),NEW(EmptyEnv)()))->add_to(NEW(NumVal)(5)),
                           "no adding functions");
    }
}

TEST_CASE( "mult_with" ) {
    SECTION( "NumVal" ){
        CHECK ( (NEW(NumVal)(5))->mult_with(NEW(NumVal)(8))
               ->equals(NEW(NumVal)(40)) );
        CHECK_THROWS_WITH ( (NEW(NumVal)(5))->mult_with(NEW(BoolVal)(false)),
                           "not a number" );
    }
    SECTION( "BoolVal" ) {
        CHECK_THROWS_WITH ( (NEW(BoolVal)(false))->mult_with(NEW(BoolVal)(false)),
        "no multiplying booleans" );
    }
    SECTION( "FunVal" ) {
        CHECK_THROWS_WITH( (NEW(FunVal)("x", NEW(NumExpr)(5), NEW(EmptyEnv)()))->mult_with(NEW(NumVal)(5)),
        "no multiplying functions");
    }
}

TEST_CASE( "value to_expr" ) {
    SECTION( "NumVal" ){
        CHECK( (NEW(NumVal)(5))->to_expr()
              ->equals(NEW(NumExpr)(5)) );
    }
    SECTION( "BoolVal" ) {
        CHECK( (NEW(BoolVal)(true))->to_expr()
              ->equals(NEW(BoolExpr)(true)) );
        CHECK( (NEW(BoolVal)(false))->to_expr()
              ->equals(NEW(BoolExpr)(false)) );
    }
    SECTION( "FunVal" ) {
        CHECK( (NEW(FunVal)("x", NEW(NumExpr)(5), NEW(EmptyEnv)()))->to_expr()
              ->equals(NEW(FunExpr)("x", NEW(NumExpr)(5))) );
    }
}

TEST_CASE( "value to_string" ) {
    SECTION( "NumVal" ){
        CHECK( (NEW(NumVal)(5))->to_string()
              == "5" );
    }
    SECTION( "BoolVal" ) {
        CHECK( (NEW(BoolVal)(true))->to_string()
              == "_true" );
        CHECK( (NEW(BoolVal)(false))->to_string()
              == "_false" );
    }
    SECTION( "FunVal" ) {
        CHECK( (NEW(FunVal)("x", NEW(NumExpr)(5), NEW(EmptyEnv)()))->to_string()
              == "_fun (x) 5" );
    }
}

TEST_CASE( "value is_true" ) {
    SECTION( "NumVal" ){
        CHECK_THROWS_WITH( (NEW(NumVal)(5))->is_true(),
                          "numbers cannot be true/false");
    }
    SECTION( "BoolVal" ) {
        CHECK( (NEW(BoolVal)(true))->is_true()
              == true);
        CHECK( (NEW(BoolVal)(false))->is_true()
              == false);
    }
    SECTION( "FunVal" ) {
        CHECK_THROWS_WITH( (NEW(FunVal)("x", NEW(NumExpr)(5), NEW(EmptyEnv)()))->is_true(),
                          "functions cannot be true/false");
    }
}

TEST_CASE ( "call" ) {
    SECTION( "NumVal" ){
        CHECK_THROWS_WITH( (NEW(NumVal)(5))->call(NEW(NumVal)(4)),
                          "cannot call on a number");
        CHECK_THROWS_WITH( (NEW(NumVal)(5))->call(NEW(BoolVal)(true)),
                          "cannot call on a number");
        CHECK_THROWS_WITH( (NEW(NumVal)(5))->call(NEW(FunVal)("x", NEW(NumExpr)(5), NEW(EmptyEnv)())),
                          "cannot call on a number");
    }
    SECTION( "BoolVal" ) {
        CHECK_THROWS_WITH( (NEW(BoolVal)(true))->call(NEW(BoolVal)(true)),
                          "cannot call on a boolean");
        CHECK_THROWS_WITH( (NEW(BoolVal)(true))->call(NEW(NumVal)(5)),
                          "cannot call on a boolean");
        CHECK_THROWS_WITH( (NEW(BoolVal)(true))->call(NEW(FunVal)("x", NEW(NumExpr)(5), NEW(EmptyEnv)())),
                          "cannot call on a boolean");
    }
    SECTION( "FunVal" ) {
        CHECK( (NEW(FunVal)("x", NEW(NumExpr)(5), NEW(EmptyEnv)()))->call(NEW(NumVal)(5))
              ->equals(NEW(NumVal)(5)));
    }
}
