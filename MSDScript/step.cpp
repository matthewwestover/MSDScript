#include "step.hpp"
#include "expr.hpp"
#include "cont.hpp"
#include "env.hpp"
#include "value.hpp"
#include "catch.hpp"

Step::mode_t Step::mode;

PTR(Cont) Step::cont;
PTR(Expr) Step::expr;
PTR(Val) Step::val;
PTR(Env) Step::env;

PTR(Val) Step::interp_by_steps(PTR(Expr) e) {
    Step::mode = Step::interp_mode;
    Step::expr = e;
    Step::env = NEW(EmptyEnv)();
    Step::val = nullptr;
    Step::cont = Cont::done;
    while (1) {
        if (Step::mode == Step::interp_mode) {
            Step::expr->step_interp();
        } else {
            if (Step::cont == Cont::done) {
                return Step::val;
            } else {
                Step::cont->step_continue();
            }
            
        }
    }
    
}

TEST_CASE( "Step Interp" ) {
    SECTION( "NumExpr" ) {
        CHECK( (Step::interp_by_steps(NEW(NumExpr)(10)))
              ->equals(NEW(NumVal)(10)));
        CHECK( ! (Step::interp_by_steps(NEW(NumExpr)(77)))
              ->equals(NEW(NumVal)(89)));
    }
    SECTION( "AddExpr" ){
        CHECK( (Step::interp_by_steps(NEW(AddExpr)(NEW(NumExpr)(3), NEW(NumExpr)(2))))
              ->equals(NEW(NumVal)(5)));
        CHECK( ! (Step::interp_by_steps(NEW(AddExpr)(NEW(NumExpr)(2), NEW(NumExpr)(3))))
              ->equals(NEW(NumVal)(6)));
    }
    SECTION( "MultExpr" ) {
        CHECK( (Step::interp_by_steps(NEW(MultExpr)(NEW(NumExpr)(3), NEW(NumExpr)(2))))
              ->equals(NEW(NumVal)(6)));
        CHECK( ! (Step::interp_by_steps(NEW(MultExpr)(NEW(NumExpr)(2), NEW(NumExpr)(3))))
              ->equals(NEW(NumVal)(5)));
    }
    SECTION( "LetExpr" ) {
        CHECK( (Step::interp_by_steps(NEW(LetExpr)("x", NEW(NumExpr)(5), NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(5)))))
              ->equals(NEW(NumVal)(10)));
        CHECK( ! (Step::interp_by_steps(NEW(LetExpr)("x", NEW(NumExpr)(4), NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(5)))))
              ->equals(NEW(NumVal)(5)));
        CHECK( (Step::interp_by_steps(NEW (LetExpr)("x",NEW (NumExpr)(3),NEW (AddExpr)(NEW (VarExpr)("x"),NEW (NumExpr)(3)))))
              ->equals(NEW (NumVal)(6)));
    }
    SECTION( "BoolExpr" ) {
        CHECK( (Step::interp_by_steps(NEW(BoolExpr)(true)))
              ->equals(NEW(BoolVal)(true)));
        CHECK( ! (Step::interp_by_steps(NEW(BoolExpr)(true)))
              ->equals(NEW(BoolVal)(false)));
    }
    SECTION( "EqualExpr" ) {
        CHECK( (Step::interp_by_steps(NEW(EqualExpr)(NEW(NumExpr)(3), NEW(NumExpr)(3))))
              ->equals(NEW(BoolVal)(true)));
        CHECK( (Step::interp_by_steps(NEW(EqualExpr)(NEW(NumExpr)(4), NEW(NumExpr)(3))))
              ->equals(NEW(BoolVal)(false)));
    }
    SECTION( "IfExpr" ) {
        CHECK( (Step::interp_by_steps(NEW(IfExpr)(NEW(BoolExpr)(true), NEW(NumExpr)(1), NEW(NumExpr)(2))))
              ->equals(NEW(NumVal)(1)));
        CHECK( (Step::interp_by_steps(NEW(IfExpr)(NEW(BoolExpr)(false), NEW(NumExpr)(1), NEW(NumExpr)(2))))
              ->equals(NEW(NumVal)(2)));
        CHECK( (Step::interp_by_steps(NEW(IfExpr)(NEW(EqualExpr)(NEW(NumExpr)(1), NEW(NumExpr)(1)), NEW(NumExpr)(1), NEW(NumExpr)(2))))
              ->equals(NEW(NumVal)(1)));
        CHECK( (Step::interp_by_steps(NEW(IfExpr)(NEW(EqualExpr)(NEW(NumExpr)(1), NEW(NumExpr)(2)), NEW(NumExpr)(1), NEW(NumExpr)(2))))
              ->equals(NEW(NumVal)(2)));
    }
    SECTION( "FunExpr" ) {
        CHECK( (Step::interp_by_steps(NEW(FunExpr)("x", NEW(NumExpr)(4))))
              ->equals(NEW(FunVal)("x", NEW(NumExpr)(4), NEW(EmptyEnv)())));
        CHECK( (Step::interp_by_steps(NEW(FunExpr)("x", NEW(AddExpr)(NEW(NumExpr)(4), NEW(VarExpr)("x")))))
              ->equals(NEW(FunVal)("x", NEW(AddExpr)(NEW(NumExpr)(4), NEW(VarExpr)("x")), NEW(EmptyEnv)())));
    }
}
