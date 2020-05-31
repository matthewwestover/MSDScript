#include "expr.hpp"
#include "value.hpp"
#include "env.hpp"
#include "cont.hpp"
#include "step.hpp"
#include "catch.hpp"

NumExpr::NumExpr(int rep) {
    this->rep = rep;
    this->val = NEW(NumVal)(rep);
}

bool NumExpr::equals(PTR(Expr) other_expr) {
    PTR(NumExpr) other_num_expr = CAST(NumExpr)(other_expr);
    if (other_num_expr == nullptr)
        return false;
    else
        return rep == other_num_expr->rep;
}

bool NumExpr::has_var() {
    return false;
}

PTR(Val) NumExpr::interp(PTR(Env) env) {
    return val;
}

void NumExpr::step_interp() {
    Step::mode = Step::continue_mode;
    Step::val = NEW(NumVal)(rep);
    Step::cont = Step::cont;
}

PTR(Expr) NumExpr::subst(std::string var, PTR(Val) new_val) {
    return NEW(NumExpr)(rep);
}

PTR(Expr) NumExpr::optimize() {
    return NEW(NumExpr)(rep);
}

std::string NumExpr::to_string() {
    return std::to_string(rep);
}

AddExpr::AddExpr(PTR(Expr) lhs, PTR(Expr) rhs) {
    this->lhs = lhs;
    this->rhs = rhs;
}

bool AddExpr::equals(PTR(Expr) other_expr) {
    PTR(AddExpr) other_add_expr = CAST(AddExpr)(other_expr);
    if (other_add_expr == nullptr)
        return false;
    else
        return (lhs->equals(other_add_expr->lhs)
                && rhs->equals(other_add_expr->rhs));
}

bool AddExpr::has_var() {
    return (lhs->has_var()
            || rhs->has_var());
}

PTR(Val) AddExpr::interp(PTR(Env) env) {
    return lhs->interp(env)->add_to(rhs->interp(env));
}

void AddExpr::step_interp(){
    Step::mode = Step::interp_mode;
    Step::expr = lhs;
    Step::env = Step::env;
    Step::cont = NEW(RightThenAddCont)(rhs, Step::env, Step::cont);
}

PTR(Expr) AddExpr::subst(std::string var, PTR(Val) new_val) {
    return NEW(AddExpr)(lhs->subst(var, new_val),
                        rhs->subst(var, new_val));
}

PTR(Expr) AddExpr::optimize() {
    PTR(Expr) olhs = lhs->optimize();
    PTR(Expr) orhs = rhs->optimize();
    if(olhs->has_var() == false && orhs->has_var() == false){
        return olhs->interp(NEW(EmptyEnv)())->add_to(orhs->interp(NEW(EmptyEnv)()))->to_expr();
    }
    return NEW(AddExpr)(olhs, orhs);
}

std::string AddExpr::to_string() {
    return "(" + lhs->to_string() + " + " + rhs->to_string() + ")";
}

MultExpr::MultExpr(PTR(Expr) lhs, PTR(Expr) rhs) {
    this->lhs = lhs;
    this->rhs = rhs;
}

bool MultExpr::equals(PTR(Expr) other_expr) {
    PTR(MultExpr) other_mult_expr = CAST(MultExpr)(other_expr);
    if (other_mult_expr == nullptr)
        return false;
    else
        return (lhs->equals(other_mult_expr->lhs)
                && rhs->equals(other_mult_expr->rhs));
}

bool MultExpr::has_var() {
    return (lhs->has_var()
            || rhs->has_var());
}

PTR(Val) MultExpr::interp(PTR(Env) env) {
    return lhs->interp(env)->mult_with(rhs->interp(env));
}

void MultExpr::step_interp() {
    Step::mode = Step::interp_mode;
    Step::expr = lhs;
    Step::env = Step::env;
    Step::cont = NEW(RightThenMultCont)(rhs, Step::env, Step::cont);
}

PTR(Expr) MultExpr::subst(std::string var, PTR(Val) new_val)
{
    return NEW(MultExpr)(lhs->subst(var, new_val), rhs->subst(var, new_val));
}

PTR(Expr) MultExpr::optimize() {
    PTR(Expr) olhs = lhs->optimize();
    PTR(Expr) orhs = rhs->optimize();
    if(olhs->has_var() == false && orhs->has_var() == false){
        return olhs->interp(NEW(EmptyEnv)())->mult_with(orhs->interp(NEW(EmptyEnv)()))->to_expr();
    }
    return NEW(MultExpr)(lhs->optimize(),
                         rhs->optimize());
}

std::string MultExpr::to_string() {
    return "(" + lhs->to_string() + " * " + rhs->to_string() + ")";
}

VarExpr::VarExpr(std::string name) {
    this->name = name;
}

bool VarExpr::equals(PTR(Expr) other_expr) {
    PTR(VarExpr) other_var_expr = CAST(VarExpr)(other_expr);
    if (other_var_expr == nullptr)
        return false;
    else
        return name == other_var_expr->name;
}

bool VarExpr::has_var() {
    return true;
}

PTR(Val) VarExpr::interp(PTR(Env) env) {
    return env->lookup(name);
}

void VarExpr::step_interp() {
    Step::mode = Step::continue_mode;
    Step::val = Step::env->lookup(name);
    Step::cont = Step::cont;
}

PTR(Expr) VarExpr::subst(std::string var, PTR(Val) new_val) {
    if (name == var)
        return new_val->to_expr();
    else
        return NEW(VarExpr)(name);
}

PTR(Expr) VarExpr::optimize() {
    return NEW(VarExpr)(name);
}

std::string VarExpr::to_string() {
    return name;
}

LetExpr::LetExpr(std::string name, PTR(Expr) rhs, PTR(Expr) body) {
    this->name = name;
    this->rhs = rhs;
    this->body = body;
}

bool LetExpr::equals(PTR(Expr) other_expr) {
    PTR(LetExpr) other_let_expr = CAST(LetExpr)(other_expr);
    if (other_let_expr == nullptr)
        return false;
    else
        return (name == other_let_expr->name
                && rhs->equals(other_let_expr->rhs)
                && body->equals(other_let_expr->body));
}

bool LetExpr::has_var() {
    return (body->has_var());
}

PTR(Val) LetExpr::interp(PTR(Env) env) {
    PTR(Val) rhs_val = rhs->interp(env);
    PTR(Env) new_env = NEW(ExtendedEnv)(name, rhs_val, env);
    return body->interp(new_env);
}

void LetExpr::step_interp() {
    Step::mode = Step::interp_mode;
    Step::expr = rhs;
    Step::env = Step::env;
    Step::cont = NEW(LetBodyCont)(name, body, Step::env, Step::cont);
}

PTR(Expr) LetExpr::subst(std::string var, PTR(Val) val) {
    return NEW(LetExpr)(name,
                        rhs->subst(var, val),
                        body->subst(var, val));
}

PTR(Expr) LetExpr::optimize() {
    if(body->has_var()){
        rhs = rhs->optimize();
        return (body->subst(name, rhs->interp(NEW(EmptyEnv)()))->optimize());
    }
    else return NEW(LetExpr)(name, rhs->optimize(), body->optimize());
}

std::string LetExpr::to_string() {
    return "(_let " + name + " = " + rhs->to_string() + " _in " + body->to_string() + ")";
}

BoolExpr::BoolExpr(bool rep) {
    this->rep = rep;
}

bool BoolExpr::equals(PTR(Expr) other_expr) {
    PTR(BoolExpr) other_bool_expr = CAST(BoolExpr)(other_expr);
    if (other_bool_expr == nullptr)
        return false;
    else
        return rep == other_bool_expr->rep;
}

bool BoolExpr::has_var() {
    return false;
}

PTR(Val) BoolExpr::interp(PTR(Env) env) {
    return NEW(BoolVal)(rep);
}

void BoolExpr::step_interp() {
    Step::mode = Step::continue_mode;
    Step::val = NEW(BoolVal)(rep);
    Step::cont = Step::cont;
}

PTR(Expr) BoolExpr::subst(std::string var, PTR(Val) new_val) {
    return NEW(BoolExpr)(rep);
}

PTR(Expr) BoolExpr::optimize(){
    return NEW(BoolExpr)(rep);
}

std::string BoolExpr::to_string() {
    if (rep == true)
        return "_true";
    else
        return "_false";
}

EqualExpr::EqualExpr(PTR(Expr) lhs, PTR(Expr) rhs) {
    this->lhs = lhs;
    this->rhs = rhs;
}

bool EqualExpr::equals(PTR(Expr) other_expr) {
    PTR(EqualExpr) other_equals = CAST(EqualExpr)(other_expr);
    if (other_equals == nullptr)
        return false;
    else
        return (lhs->equals(other_equals->lhs)
                && rhs->equals(other_equals->rhs));
}

bool EqualExpr::has_var() {
    return (lhs->has_var()
            || rhs->has_var());
}

PTR(Val) EqualExpr::interp(PTR(Env) env) {
    PTR(Val) olhs = lhs->interp(env);
    PTR(Val) orhs = rhs->interp(env);
    return NEW(BoolVal)(olhs->equals(orhs));
}

void EqualExpr::step_interp() {
    Step::mode = Step::interp_mode;
    Step::expr = lhs;
    Step::env = Step::env;
    Step::cont = NEW(RightThenEqualsCont)(rhs, Step::env, Step::cont);
}

PTR(Expr) EqualExpr::subst(std::string var, PTR(Val) val) {
    return NEW(EqualExpr)(lhs->subst(var, val),
                          rhs->subst(var, val));
}

PTR(Expr) EqualExpr::optimize() {
    PTR(Expr) olhs = lhs->optimize();
    PTR(Expr) orhs = rhs->optimize();
    if(olhs->has_var() || orhs->has_var())
        return NEW(EqualExpr)(olhs, orhs);
    else
        return NEW(BoolExpr)(olhs->interp(NEW(EmptyEnv)())->equals(orhs->interp(NEW(EmptyEnv)())));
}

std::string EqualExpr::to_string() {
    return lhs->to_string() + " == " + rhs->to_string();
}

IfExpr::IfExpr(PTR(Expr) test_part, PTR(Expr) then_part, PTR(Expr) else_part) {
    this->test_part = test_part;
    this->then_part = then_part;
    this->else_part = else_part;
}

bool IfExpr::equals(PTR(Expr) other_expr) {
    PTR(IfExpr) other_if_expr = CAST(IfExpr)(other_expr);
    if (other_if_expr == nullptr)
        return false;
    else
        return (test_part->equals(other_if_expr->test_part)
                && then_part->equals(other_if_expr->then_part)
                && else_part->equals(other_if_expr->else_part));
}

bool IfExpr::has_var() {
    return (test_part->has_var()
            || then_part->has_var()
            || else_part->has_var());
}

PTR(Val) IfExpr::interp(PTR(Env) env) {
    if(test_part->interp(env)->is_true())
        return then_part->interp(env);
    else
        return else_part->interp(env);
}

void IfExpr::step_interp() {
    Step::mode = Step::interp_mode;
    Step::expr = test_part;
    Step::env = Step::env;
    Step::cont = NEW(IfBranchCont)(then_part, else_part, Step::env, Step::cont);
}

PTR(Expr) IfExpr::subst(std::string var, PTR(Val) val) {
    return NEW(IfExpr)(test_part->subst(var, val),
                       then_part->subst(var, val),
                       else_part->subst(var, val));
}

PTR(Expr) IfExpr::optimize() {
    if (! test_part->has_var()) {
        if (test_part->interp(NEW(EmptyEnv)())->is_true()) {
            return then_part->optimize();
        } else {
            return else_part->optimize();
        }
    }
    return NEW(IfExpr)(test_part->optimize(),
                       then_part->optimize(),
                       else_part->optimize());
}

std::string IfExpr::to_string() {
    return "(_if " + test_part->to_string() +
    " _then " + then_part->to_string() +
    " _else " + else_part->to_string() + ")";
}

FunExpr::FunExpr(std::string arg, PTR(Expr) body) {
    this->formal_arg = arg;
    this->body = body;
}

bool FunExpr::equals(PTR(Expr) other_expr) {
    PTR(FunExpr) other_fun_expr = CAST(FunExpr)(other_expr);
    if (other_fun_expr == nullptr)
        return false;
    else
        return (formal_arg == other_fun_expr->formal_arg
                && body->equals(other_fun_expr->body));
}

bool FunExpr::has_var() {
    return true;
}

PTR(Val) FunExpr::interp(PTR(Env) env) {
    return NEW(FunVal)(formal_arg, body, env);
}

void FunExpr::step_interp() {
    Step::mode = Step::continue_mode;
    Step::val = NEW(FunVal)(formal_arg, body, Step::env);
    Step::cont = Step::cont;
}

PTR(Expr) FunExpr::subst(std::string var, PTR(Val) val) {
    if(var == formal_arg){
        return NEW(FunExpr)(formal_arg, body);
    }
    return NEW(FunExpr)(formal_arg, body->subst(var, val));
}

PTR(Expr) FunExpr::optimize() {
    return NEW(FunExpr)(formal_arg, body->optimize());
}

std::string FunExpr::to_string() {
    return "(_fun (" + formal_arg + ") " + body->to_string() + ")";
}

CallExpr::CallExpr(PTR(Expr) to_be, PTR(Expr) actual) {
    this->to_be_called = to_be;
    this->actual_arg = actual;
}

bool CallExpr::equals(PTR(Expr) other_expr) {
    PTR(CallExpr) other_call_expr = CAST(CallExpr)(other_expr);
    if (other_call_expr == nullptr)
        return false;
    else
        return(to_be_called->equals(other_call_expr->to_be_called)
               && actual_arg->equals(other_call_expr->actual_arg));
}

bool CallExpr::has_var() {
    return true;
}

PTR(Val) CallExpr::interp(PTR(Env)env) {
    return to_be_called->interp(env)->call(actual_arg->interp(env));
}

void CallExpr::step_interp() {
    Step::mode = Step::interp_mode;
    Step::expr = to_be_called;
    Step::cont = NEW(ArgThenCallCont)(actual_arg, Step::env, Step::cont);
}

PTR(Expr) CallExpr::subst(std::string var, PTR(Val) val) {
    return NEW(CallExpr)(to_be_called->subst(var, val), actual_arg->subst(var, val));
}

PTR(Expr) CallExpr::optimize() {
    return NEW(CallExpr)(to_be_called->optimize(), actual_arg->optimize());
}

std::string CallExpr::to_string() {
    return ", (" + to_be_called->to_string() + "(" + actual_arg->to_string() + "))";
}

TEST_CASE( "Equals" ) {
    SECTION( "NumExpr" ) {
        CHECK( (NEW(NumExpr)(1))
              ->equals(NEW(NumExpr)(1)) );
        CHECK( ! (NEW(NumExpr)(1))
              ->equals(NEW(NumExpr)(2)) );
        CHECK( ! (NEW(NumExpr)(1))
              ->equals(NEW(MultExpr)(NEW(NumExpr)(2), NEW(NumExpr)(4))) );
    }
    SECTION( "VarExpr" ){
        CHECK( (NEW(VarExpr)("x"))
              ->equals(NEW(VarExpr)("x")) );
        CHECK( ! (NEW(VarExpr)("x"))
              ->equals(NEW(VarExpr)("y")) );
        CHECK( ! (NEW(VarExpr)("x"))
              ->equals(NEW(NumExpr)(5)) );
    }
    SECTION( "AddExpr" ){
        CHECK( (NEW(AddExpr)(NEW(NumExpr)(8), NEW(NumExpr)(9)))
              ->equals(NEW(AddExpr)(NEW(NumExpr)(8), NEW(NumExpr)(9))) );
        CHECK( ! (NEW(AddExpr)(NEW(NumExpr)(8), NEW(NumExpr)(9)))
              ->equals(NEW(AddExpr)(NEW(NumExpr)(8), NEW(NumExpr)(10))) );
        CHECK( ! (NEW(AddExpr)(NEW(NumExpr)(8), NEW(NumExpr)(9)))
              ->equals(NEW(AddExpr)(NEW(NumExpr)(10), NEW(NumExpr)(9))) );
        CHECK( ! (NEW(AddExpr)(NEW(NumExpr)(8), NEW(NumExpr)(9)))
              ->equals(NEW(NumExpr)(8)) );
    }
    SECTION( "MultExpr" ) {
        CHECK( (NEW(MultExpr)(NEW(NumExpr)(8), NEW(NumExpr)(9)))
              ->equals(NEW(MultExpr)(NEW(NumExpr)(8), NEW(NumExpr)(9))));
        CHECK( ! (NEW(MultExpr)(NEW(NumExpr)(8), NEW(NumExpr)(9)))
              ->equals(NEW(MultExpr)(NEW(NumExpr)(8), NEW(NumExpr)(10))) );
        CHECK( ! (NEW(MultExpr)(NEW(NumExpr)(8), NEW(NumExpr)(9)))
              ->equals(NEW(MultExpr)(NEW(NumExpr)(10), NEW(NumExpr)(9))) );
        CHECK( ! (NEW(MultExpr)(NEW(NumExpr)(8), NEW(NumExpr)(9)))
              ->equals(NEW(NumExpr)(8)) );
    }
    SECTION( "LetExpr" ) {
        CHECK( (NEW(LetExpr)("x", NEW(NumExpr)(5), NEW(NumExpr)(5)))
              ->equals(NEW(LetExpr)("x", NEW(NumExpr)(5), NEW(NumExpr)(5))));
        CHECK( ! (NEW(LetExpr)("x", NEW(NumExpr)(5), NEW(NumExpr)(5)))
              ->equals(NEW(LetExpr)("y", NEW(NumExpr)(5), NEW(NumExpr)(5))));
        CHECK( ! (NEW(LetExpr)("x", NEW(NumExpr)(5), NEW(NumExpr)(5)))
              ->equals(NULL));
    }
    SECTION( "BoolExpr" ) {
        CHECK( (NEW(BoolExpr)(true))
              ->equals(NEW(BoolExpr)(true)));
        CHECK( ! (NEW(BoolExpr)(true))
              ->equals(NEW(BoolExpr)(false)));
        CHECK( ! (NEW(BoolExpr)(true))
              ->equals(NULL));
    }
    SECTION( "EqualExpr" ) {
        CHECK( (NEW(EqualExpr)(NEW(NumExpr)(8), NEW(NumExpr)(9)))
              ->equals(NEW(EqualExpr)(NEW(NumExpr)(8), NEW(NumExpr)(9))) );
        CHECK( ! (NEW(EqualExpr)(NEW(NumExpr)(8), NEW(NumExpr)(9)))
              ->equals(NEW(EqualExpr)(NEW(NumExpr)(8), NEW(NumExpr)(10))) );
        CHECK( ! (NEW(EqualExpr)(NEW(NumExpr)(8), NEW(NumExpr)(9)))
              ->equals(NEW(EqualExpr)(NEW(NumExpr)(10), NEW(NumExpr)(9))) );
        CHECK( ! (NEW(EqualExpr)(NEW(NumExpr)(8), NEW(NumExpr)(9)))
              ->equals(NEW(NumExpr)(8)) );
    }
    SECTION( "IfExpr" ) {
        CHECK( (NEW(IfExpr)(NEW(BoolExpr)(true), NEW(NumExpr)(3), NEW(NumExpr)(4)))
              ->equals(NEW(IfExpr)(NEW(BoolExpr)(true), NEW(NumExpr)(3), NEW(NumExpr)(4))));
        CHECK( ! (NEW(IfExpr)(NEW(BoolExpr)(true), NEW(NumExpr)(3), NEW(NumExpr)(4)))
              ->equals(NEW(IfExpr)(NEW(BoolExpr)(false), NEW(NumExpr)(3), NEW(NumExpr)(4))));
        CHECK( ! (NEW(IfExpr)(NEW(BoolExpr)(true), NEW(NumExpr)(3), NEW(NumExpr)(4)))
              ->equals(NEW(IfExpr)(NEW(BoolExpr)(true), NEW(NumExpr)(4), NEW(NumExpr)(4))));
        CHECK( ! (NEW(IfExpr)(NEW(BoolExpr)(true), NEW(NumExpr)(3), NEW(NumExpr)(4)))
              ->equals(NEW(IfExpr)(NEW(BoolExpr)(true), NEW(NumExpr)(3), NEW(NumExpr)(6))));
        CHECK( ! (NEW(IfExpr)(NEW(BoolExpr)(true), NEW(NumExpr)(3), NEW(NumExpr)(4)))
              ->equals(NULL));
    }
    SECTION( "FunExpr" ) {
        CHECK( (NEW(FunExpr)("x", NEW(VarExpr)("x")))
              ->equals(NEW(FunExpr)("x", NEW(VarExpr)("x"))));
        CHECK( (NEW(FunExpr)("x", NEW(NumExpr)(1)))
              ->equals(NEW(FunExpr)("x", NEW(NumExpr)(1))));
        CHECK( ! (NEW(FunExpr)("x", NEW(NumExpr)(3)))
              ->equals(NEW(FunExpr)("y", NEW(NumExpr)(3))));
        CHECK( ! (NEW(FunExpr)("x", NEW(NumExpr)(3)))
              ->equals(NEW(FunExpr)("x", NEW(NumExpr)(4))));
        CHECK( ! (NEW(FunExpr)("x", NEW(NumExpr)(3)))
              ->equals(NULL));
    }
    
    SECTION( "CallExpr" ) {
        CHECK( (NEW(CallExpr)(NEW(NumExpr)(3), NEW(NumExpr)(3)))
              ->equals(NEW(CallExpr)(NEW(NumExpr)(3), NEW(NumExpr)(3))));
        CHECK( ! (NEW(CallExpr)(NEW(NumExpr)(3), NEW(NumExpr)(3)))
              ->equals(NEW(CallExpr)(NEW(NumExpr)(4), NEW(NumExpr)(3))));
        CHECK( ! (NEW(CallExpr)(NEW(NumExpr)(3), NEW(NumExpr)(3)))
              ->equals(NEW(CallExpr)(NEW(NumExpr)(3), NEW(VarExpr)("x"))));
        CHECK( ! (NEW(CallExpr)(NEW(NumExpr)(3), NEW(NumExpr)(3)))
              ->equals(NULL));
    }
}

TEST_CASE( "Interp" ) {
    SECTION( "NumExpr" ) {
        CHECK( (NEW(NumExpr)(10))->interp(NEW(EmptyEnv)())
              ->equals(NEW(NumVal)(10)) );
    }
    SECTION( "VarExpr" ){
        CHECK_THROWS( (NEW(VarExpr)("fish"))->interp(NEW(EmptyEnv)()));
        CHECK_THROWS( (NEW(AddExpr)(NEW(NumExpr)(3), NEW(VarExpr)("taco")))->interp(NEW(EmptyEnv)()));
    }
    SECTION( "AddExpr" ){
        CHECK( (NEW(AddExpr)(NEW(NumExpr)(3), NEW(NumExpr)(2)))->interp(NEW(EmptyEnv)())
              ->equals(NEW(NumVal)(5)) );
        CHECK( (NEW(AddExpr)(NEW(VarExpr)("x"), NEW(VarExpr)("y")))->interp( NEW(ExtendedEnv)("x", NEW(NumVal)(4), NEW(ExtendedEnv)("y", NEW(NumVal)(3), NEW(EmptyEnv)())))
              ->equals(NEW(NumVal)(7)));
    }
    SECTION( "MultExpr" ) {
        CHECK( (NEW(MultExpr)(NEW(NumExpr)(3), NEW(NumExpr)(2)))->interp(NEW(EmptyEnv)())
              ->equals(NEW(NumVal)(6)) );
    }
    SECTION( "LetExpr" ) {
        CHECK( (NEW(LetExpr)("x", NEW(NumExpr)(5), NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(5))))->interp(NEW(EmptyEnv)())
              ->equals(NEW(NumVal)(10)));
    }
    SECTION( "BoolExpr" ) {
        CHECK( (NEW(BoolExpr)(true))->interp(NEW(EmptyEnv)())
              ->equals(NEW(BoolVal)(true)));
        CHECK( (NEW(BoolExpr)(false))->interp(NEW(EmptyEnv)())
              ->equals(NEW(BoolVal)(false)));
    }
    SECTION( "EqualExpr" ) {
        CHECK( (NEW(EqualExpr)(NEW(NumExpr)(3), NEW(NumExpr)(3)))->interp(NEW(EmptyEnv)())
              ->equals(NEW(BoolVal)(true)) );
        CHECK( (NEW(EqualExpr)(NEW(NumExpr)(3), NEW(NumExpr)(4)))->interp(NEW(EmptyEnv)())
              ->equals(NEW(BoolVal)(false)) );
    }
    SECTION( "IfExpr" ) {
        CHECK( (NEW(IfExpr)(NEW(BoolExpr)(true), NEW(NumExpr)(1), NEW(NumExpr)(2)))->interp(NEW(EmptyEnv)())
              ->equals(NEW(NumVal)(1)));
        CHECK( (NEW(IfExpr)(NEW(BoolExpr)(false), NEW(NumExpr)(1), NEW(NumExpr)(2)))->interp(NEW(EmptyEnv)())
              ->equals(NEW(NumVal)(2)));
        CHECK( (NEW(IfExpr)(NEW(EqualExpr)(NEW(NumExpr)(1), NEW(NumExpr)(1)), NEW(NumExpr)(1), NEW(NumExpr)(2)))->interp(NEW(EmptyEnv)())
              ->equals(NEW(NumVal)(1)));
        CHECK( (NEW(IfExpr)(NEW(EqualExpr)(NEW(NumExpr)(1), NEW(NumExpr)(2)), NEW(NumExpr)(1), NEW(NumExpr)(2)))->interp(NEW(EmptyEnv)())
              ->equals(NEW(NumVal)(2)));
    }
    SECTION( "FunExpr" ) {
        CHECK( (NEW(FunExpr)("x", NEW(NumExpr)(4)))->interp(NEW(EmptyEnv)())
              ->equals(NEW(FunVal)("x", NEW(NumExpr)(4), NEW(EmptyEnv)())));
        CHECK( (NEW(FunExpr)("x", NEW(AddExpr)(NEW(NumExpr)(4), NEW(VarExpr)("x"))))->interp(NEW(EmptyEnv)())
              ->equals(NEW(FunVal)("x", NEW(AddExpr)(NEW(NumExpr)(4), NEW(VarExpr)("x")), NEW(EmptyEnv)())));
    }
}

TEST_CASE( "Subst" ) {
    SECTION( "NumExpr" ) {
        CHECK( (NEW(NumExpr)(10))->subst("dog", NEW(NumVal)(3))
              ->equals(NEW(NumExpr)(10)) );
    }
    SECTION( "VarExpr" ){
        CHECK( (NEW(VarExpr)("fish"))->subst("dog", NEW(NumVal)(3))
              ->equals(NEW(VarExpr)("fish")) );
        CHECK( (NEW(VarExpr)("dog"))->subst("dog", NEW(NumVal)(3) )
              ->equals(NEW(NumExpr)(3)) );
    }
    SECTION( "AddExpr" ){
        CHECK( (NEW(AddExpr)(NEW(NumExpr)(2), NEW(VarExpr)("dog")))->subst("dog", NEW(NumVal)(3))
              ->equals(NEW(AddExpr)(NEW(NumExpr)(2), NEW(NumExpr)(3))) );
    }
    SECTION( "MultExpr" ) {
        CHECK( (NEW(MultExpr)(NEW(NumExpr)(2), NEW(VarExpr)("dog")))->subst("dog", NEW(NumVal)(3))
              ->equals(NEW(MultExpr)(NEW(NumExpr)(2), NEW(NumExpr)(3))) );
    }
    SECTION( "LetExpr" ) {
        CHECK( (NEW(LetExpr)("x", NEW(NumExpr)(3), NEW(AddExpr)(NEW(VarExpr)("y"), NEW(NumExpr)(2))))->subst("y", NEW(NumVal)(5))
              ->equals(NEW(LetExpr)("x", NEW(NumExpr)(3), NEW(AddExpr)(NEW(NumExpr)(5), NEW(NumExpr)(2)))));
    }
    SECTION( "BoolExpr" ) {
        CHECK( (NEW(BoolExpr)(true))->subst("dog", NEW(NumVal)(3))
              ->equals(NEW(BoolExpr)(true)));
    }
    SECTION( "EqualExpr" ) {
        CHECK( (NEW(EqualExpr)(NEW(NumExpr)(2), NEW(VarExpr)("dog")))->subst("dog", NEW(NumVal)(3))
              ->equals(NEW(EqualExpr)(NEW(NumExpr)(2), NEW(NumExpr)(3))) );
    }
    SECTION( "IfExpr" ) {
        CHECK( (NEW(IfExpr)(NEW(BoolExpr)(true), NEW(VarExpr)("x"), NEW(NumExpr)(2)))->subst("x", NEW(NumVal)(5))
              ->equals(NEW(IfExpr)(NEW(BoolExpr)(true), NEW(NumExpr)(5), NEW(NumExpr)(2))));
        CHECK( (NEW(IfExpr)(NEW(BoolExpr)(true), NEW(NumExpr)(5), NEW(VarExpr)("x")))->subst("x", NEW(NumVal)(2))
              ->equals(NEW(IfExpr)(NEW(BoolExpr)(true), NEW(NumExpr)(5), NEW(NumExpr)(2))));
        CHECK( (NEW(IfExpr)(NEW(BoolExpr)(false), NEW(NumExpr)(5), NEW(VarExpr)("x")))->subst("x", NEW(NumVal)(2))
              ->equals(NEW(IfExpr)(NEW(BoolExpr)(false), NEW(NumExpr)(5), NEW(NumExpr)(2))));
    }
}

TEST_CASE( "Has_Var" ) {
    SECTION( "NumExpr" ) {
        CHECK( (NEW(NumExpr)(10))->has_var()
              == false );
    }
    SECTION( "VarExpr" ){
        CHECK( (NEW(VarExpr)("x"))->has_var()
              == true );
    }
    SECTION( "AddExpr" ){
        CHECK( (NEW(AddExpr)(NEW(NumExpr)(1), NEW(NumExpr)(1)))->has_var()
              == false );
        CHECK( (NEW(AddExpr)(NEW(NumExpr)(1), NEW(VarExpr)("x")))->has_var()
              == true );
    }
    SECTION( "MultExpr" ) {
        CHECK( (NEW(MultExpr)(NEW(NumExpr)(1), NEW(NumExpr)(1)))->has_var()
              == false );
        CHECK( (NEW(MultExpr)(NEW(NumExpr)(1), NEW(VarExpr)("x")))->has_var()
              == true );
    }
    SECTION( "LetExpr" ) {
        CHECK( (NEW(LetExpr)("x", NEW(NumExpr)(1), NEW(NumExpr)(1)))->has_var()
              == false );
        CHECK( (NEW(LetExpr)("x", NEW(NumExpr)(1), NEW(VarExpr)("x")))->has_var()
              == true );
    }
    SECTION( "BoolExpr" ) {
        CHECK( (NEW(BoolExpr)(true))->has_var()
              == false );
    }
    SECTION( "EqualExpr" ) {
        CHECK( (NEW(EqualExpr)(NEW(NumExpr)(1), NEW(NumExpr)(1)))->has_var()
              == false );
        CHECK( (NEW(EqualExpr)(NEW(NumExpr)(1), NEW(VarExpr)("x")))->has_var()
              == true );
    }
    SECTION( "IfExpr" ) {
        CHECK( (NEW(IfExpr)(NEW(BoolExpr)(true), NEW(NumExpr)(5), NEW(NumExpr)(3)))->has_var()
              == false );
        CHECK( (NEW(IfExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(5), NEW(NumExpr)(3)))->has_var()
              == true );
        CHECK( (NEW(IfExpr)(NEW(BoolExpr)(true), NEW(VarExpr)("x"), NEW(NumExpr)(3)))->has_var()
              == true );
        CHECK( (NEW(IfExpr)(NEW(BoolExpr)(true), NEW(NumExpr)(5), NEW(VarExpr)("x")))->has_var()
              == true );
    }
    SECTION( "FunExpr" ) {
        CHECK( (NEW(FunExpr)("x", NEW(NumExpr)(4)))->has_var()
              == true);
    }
    
    SECTION( "CallExpr" ) {
        CHECK( (NEW(CallExpr)(NEW(NumExpr)(4), NEW(NumExpr)(4)))->has_var()
              == true);
    }
}

TEST_CASE( "Optimize" ) {
    SECTION( "NumExpr" ) {
        CHECK( (NEW(NumExpr)(10))->optimize()
              ->equals(NEW(NumExpr)(10)));
    }
    SECTION( "VarExpr" ){
        CHECK( (NEW(VarExpr)("x"))->optimize()
              ->equals(NEW(VarExpr)("x")));
    }
    SECTION( "AddExpr" ){
        CHECK( (NEW(AddExpr)(NEW(NumExpr)(2), NEW(NumExpr)(3)))->optimize()
              ->equals(NEW(NumExpr)(5)));
        CHECK( (NEW(AddExpr)(NEW(NumExpr)(2), NEW(VarExpr)("x")))->optimize()
              ->equals(NEW(AddExpr)(NEW(NumExpr)(2), NEW(VarExpr)("x"))));
    }
    SECTION( "MultExpr" ) {
        CHECK( (NEW(MultExpr)(NEW(NumExpr)(2), NEW(NumExpr)(3)))->optimize()
              ->equals(NEW(NumExpr)(6)));
        CHECK( (NEW(MultExpr)(NEW(NumExpr)(2), NEW(VarExpr)("x")))->optimize()
              ->equals(NEW(MultExpr)(NEW(NumExpr)(2), NEW(VarExpr)("x"))));
    }
    SECTION( "LetExpr" ) {
        CHECK( (NEW(LetExpr)("x", NEW(NumExpr)(5), NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(5))))->optimize()
              ->equals(NEW(NumExpr)(10)));
        CHECK( (NEW(LetExpr)("x", NEW(NumExpr)(5), NEW(AddExpr)(NEW(NumExpr)(5), NEW(NumExpr)(5))))->optimize()
              ->equals(NEW(LetExpr)("x", NEW(NumExpr)(5), NEW(NumExpr)(10))));
    }
    SECTION( "BoolExpr" ) {
        CHECK( (NEW(BoolExpr)(false))->optimize()
              ->equals(NEW(BoolExpr)(false)));
    }
    SECTION( "EqualExpr" ) {
        CHECK( (NEW(EqualExpr)(NEW(NumExpr)(2), NEW(NumExpr)(3)))->optimize()
              ->equals(NEW(BoolExpr)(false)));
        CHECK( (NEW(EqualExpr)(NEW(NumExpr)(2), NEW(NumExpr)(2)))->optimize()
              ->equals(NEW(BoolExpr)(true)));
        CHECK( (NEW(EqualExpr)(NEW(NumExpr)(2), NEW(VarExpr)("x")))->optimize()
              ->equals(NEW(EqualExpr)(NEW(NumExpr)(2), NEW(VarExpr)("x"))));
    }
    SECTION( "IfExpr" ) {
        CHECK( (NEW(IfExpr)(NEW(BoolExpr)(true), NEW(NumExpr)(1), NEW(NumExpr)(2)))->optimize()
              ->equals(NEW(NumExpr)(1)));
        CHECK( (NEW(IfExpr)(NEW(BoolExpr)(false), NEW(NumExpr)(1), NEW(NumExpr)(2)))->optimize()
              ->equals(NEW(NumExpr)(2)));
        CHECK( (NEW(IfExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(1), NEW(NumExpr)(2)))->optimize()
              ->equals(NEW(IfExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(1), NEW(NumExpr)(2))));
    }
    SECTION( "FunExpr" ) {
        CHECK( (NEW(FunExpr)("x", NEW(NumExpr)(5)))->optimize()
              ->equals(NEW(FunExpr)("x", NEW(NumExpr)(5))));
    }
    SECTION( "CallExpr" ) {
        CHECK( (NEW(CallExpr)(NEW(NumExpr)(4), NEW(NumExpr)(4)))->optimize()
              ->equals(NEW(CallExpr)(NEW(NumExpr)(4), NEW(NumExpr)(4))));
    }
}

TEST_CASE( "to_string" ) {
    SECTION( "NumExpr" ) {
        CHECK( (NEW(NumExpr)(5))->to_string()
              == "5");
    }
    SECTION( "VarExpr" ){
        CHECK( (NEW(VarExpr)("x"))->to_string()
              == "x");
    }
    SECTION( "AddExpr" ){
        CHECK( (NEW(AddExpr)(NEW(NumExpr)(2), NEW(NumExpr)(3)))->to_string()
              == "(2 + 3)");
    }
    SECTION( "MultExpr" ) {
        CHECK( (NEW(MultExpr)(NEW(NumExpr)(2), NEW(NumExpr)(3)))->to_string()
              == "(2 * 3)");
    }
    SECTION( "LetExpr" ) {
        CHECK( (NEW(LetExpr)("x", NEW(NumExpr)(3), NEW(AddExpr)(NEW(NumExpr)(2), NEW(VarExpr)("x"))))->to_string()
              == "(_let x = 3 _in (2 + x))");
    }
    SECTION( "BoolExpr" ) {
        CHECK( (NEW(BoolExpr)(true))->to_string()
              == "_true");
        CHECK( (NEW(BoolExpr)(false))->to_string()
              == "_false");
    }
    SECTION( "EqualExpr" ) {
        CHECK( (NEW(EqualExpr)(NEW(NumExpr)(2), NEW(NumExpr)(3)))->to_string()
              == "2 == 3");
    }
    SECTION( "IfExpr" ) {
        CHECK( (NEW(IfExpr)(NEW(BoolExpr)(true), NEW(NumExpr)(1), NEW(NumExpr)(2)))->to_string()
              == "(_if _true _then 1 _else 2)" );
    }
    SECTION( "FunExpr" ) {
        CHECK( (NEW(FunExpr)("x", NEW(NumExpr)(5)))->to_string()
              == "(_fun (x) 5)");
    }
    SECTION( "CallExpr" ) {
        CHECK( (NEW(CallExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(3)))->to_string()
              == ", (x(3))");
    }
}
