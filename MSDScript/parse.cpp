#include <iostream>
#include <sstream>
#include "parse.hpp"
#include "expr.hpp"
#include "value.hpp"
#include "env.hpp"
#include "step.hpp"
#include "catch.hpp"

static PTR(Expr) parse_expr(std::istream &in);
static PTR(Expr) parse_comparg(std::istream &in);
static PTR(Expr) parse_addend(std::istream &in);
static PTR(Expr) parse_multicand(std::istream &in);
static PTR(Expr) parse_inner(std::istream &in);
static PTR(Expr) parse_number(std::istream &in);
static PTR(Expr) parse_variable(std::istream &in);
static PTR(Expr) parse_let(std::istream &in);
static PTR(Expr) parse_if(std::istream &in);
static PTR(Expr) parse_fun(std::istream &in);
static std::string parse_keyword(std::istream &in);
static std::string parse_alphabetic(std::istream &in, std::string prefix);
static char peek_after_spaces(std::istream &in);

PTR(Expr) parse(std::istream &in) {
    PTR(Expr) expr = parse_expr(in);
    
    char c = peek_after_spaces(in);
    if (!in.eof())
        throw std::runtime_error((std::string)"expected end of file at " + c);
    return expr;
}

static PTR(Expr) parse_expr(std::istream &in) {
    PTR(Expr) expr = parse_comparg(in);
    
    char c = peek_after_spaces(in);
    if (c == '=') {
        c = in.get();
        c = in.get();
        if (c == '=') {
            PTR(Expr) rhs = parse_expr(in);
            expr = NEW(EqualExpr)(expr, rhs);
        }
    }
    return expr;
}

static PTR(Expr) parse_comparg(std::istream &in) {
    PTR(Expr) expr = parse_addend(in);
    
    char c = peek_after_spaces(in);
    if (c == '+') {
        in >> c;
        PTR(Expr) rhs = parse_comparg(in);
        expr = NEW(AddExpr)(expr, rhs);
    }
    return expr;
}

static PTR(Expr) parse_addend(std::istream &in) {
    PTR(Expr) expr = parse_multicand(in);
    
    char c = peek_after_spaces(in);
    if (c == '*') {
        c = in.get();
        PTR(Expr) rhs = parse_addend(in);
        expr = NEW(MultExpr)(expr, rhs);
    }
    return expr;
}

static PTR(Expr) parse_multicand(std::istream &in) {
    PTR(Expr) expr = parse_inner(in);
    
    while (peek_after_spaces(in) == '(') {
        in.get();
        PTR(Expr) actual_arg = parse_expr(in); // try parse inner
        expr = NEW(CallExpr)(expr, actual_arg);
        if(peek_after_spaces(in) == ')'){
            in.get();
        }
        else {
            throw std::runtime_error("expected a close parenthesis");
        }
    }
    return expr;
}

static PTR(Expr) parse_inner(std::istream &in) {
    PTR(Expr) expr;
    
    char c = peek_after_spaces(in);
    if (c == '(') {
        c = in.get();
        expr = parse_expr(in);
        c = peek_after_spaces(in);
        if (c == ')') {
            c = in.get();
        }
        else
            throw std::runtime_error("expected a close parenthesis");
    } else if (c == '-' || isdigit(c)) {
        expr = parse_number(in);
    } else if (isalpha(c)) {
        expr = parse_variable(in);
    } else if (c == '_') {
        std::string keyword = parse_keyword(in);
        if (keyword == "_let") {
            expr = parse_let(in);
        } else if (keyword == "_in") {
            c = peek_after_spaces(in);
            expr = parse_expr(in);
        } else if (keyword == "_true") {
            return NEW(BoolExpr)(true);
        } else if (keyword == "_false") {
            return NEW(BoolExpr)(false);
        } else if (keyword == "_if") {
            expr = parse_if(in);
        } else if (keyword == "_fun" ){
            expr = parse_fun(in);
        } else {
            throw std::runtime_error((std::string)"unexpected keyword " + keyword);
        }
    } else {
        throw std::runtime_error((std::string)"expected a digit or open parenthesis at " + c);
    }
    return expr;
}

static PTR(Expr) parse_let(std::istream &in) {
    char c = peek_after_spaces(in);
    std::string name = "";
    name += parse_alphabetic(in, name);
    c = peek_after_spaces(in);
    c = in.get();
    c = peek_after_spaces(in);
    PTR(Expr) expr = parse_expr(in);
    PTR(Expr) expr2 = parse_expr(in);
    PTR(Expr) let = NEW(LetExpr)(name, expr, expr2);
    return let;
}

static PTR(Expr) parse_number(std::istream &in) {
    char c = peek_after_spaces(in);
    if(c == '-')
        c = in.get();
    int num = 0;
    in >> num;
    if(c == '-')
        num *= -1;
    return NEW(NumExpr)(num);
}

static PTR(Expr) parse_variable(std::istream &in) {
    return NEW(VarExpr)(parse_alphabetic(in, ""));
}

static PTR(Expr) parse_if(std::istream &in) {
    PTR(Expr) test_case = parse_expr(in);
    std::string keyword = parse_keyword(in);
    if (keyword != "_then")
        throw std::runtime_error("expected keyword _then");
    PTR(Expr) then_case = parse_expr(in);
    keyword = parse_keyword(in);
    if (keyword != "_else")
        throw std::runtime_error("expected keyword _else");
    PTR(Expr) else_case = parse_expr(in);
    return NEW(IfExpr)(test_case, then_case, else_case);
}

static PTR(Expr) parse_fun(std::istream &in) {
    char c = peek_after_spaces(in);
    if (c != '(') {
        throw std::runtime_error("expected an open parenthesis");
    }
    c = in.get();
    std::string variable = parse_alphabetic(in, "");
    c = peek_after_spaces(in);
    if (c != ')') {
        throw std::runtime_error("expected a close parenthesis");
    }
    c = in.get();
    PTR(Expr) expr = parse_expr(in);
    return NEW(FunExpr)(variable, expr);
}

static std::string parse_keyword(std::istream &in) {
    in.get(); // consume `_`
    return parse_alphabetic(in, "_");
}

static std::string parse_alphabetic(std::istream &in, std::string prefix) {
    std::string name = prefix;
    while (1) {
        char c = in.peek();
        if (!isalpha(c))
            break;
        name += in.get();
    }
    return name;
}

static char peek_after_spaces(std::istream &in) {
    char c;
    while (1) {
        c = in.peek();
        if (!isspace(c))
            break;
        c = in.get();
    }
    return c;
}

static PTR(Expr) parse_str(std::string s) {
    std::istringstream in(s);
    return parse(in);
}

static std::string parse_str_error(std::string s) {
    std::istringstream in(s);
    try {
        (void)parse(in);
        return "";
    } catch (std::runtime_error exn) {
        return exn.what();
    }
}

TEST_CASE( "Parse Testing" ) {
    PTR(Expr) ten_plus_one = NEW(AddExpr)(NEW(NumExpr)(10), NEW(NumExpr)(1));
    CHECK( parse_str("10")
          ->equals(NEW(NumExpr)(10)));
    CHECK( parse_str("(10)")
          ->equals(NEW(NumExpr)(10)));
    CHECK( parse_str("-10")
          ->equals(NEW(NumExpr)(-10)));
    CHECK( parse_str("(-10)")
          ->equals(NEW(NumExpr)(-10)));
    CHECK( parse_str("10+1")
          ->equals(ten_plus_one));
    CHECK( parse_str("10+-1")
          ->equals(NEW(AddExpr)(NEW(NumExpr)(10), NEW(NumExpr)(-1))));
    CHECK( parse_str("(10+1)")
          ->equals(ten_plus_one));
    CHECK( parse_str("(10)+1")
          ->equals(ten_plus_one));
    CHECK( parse_str("10+(1)")
          ->equals(ten_plus_one));
    CHECK( parse_str("1+2*3")
          ->equals(NEW(AddExpr)(NEW(NumExpr)(1), NEW(MultExpr)(NEW(NumExpr)(2), NEW(NumExpr)(3)))));
    CHECK( parse_str("1*2+3")
          ->equals(NEW(AddExpr)(NEW(MultExpr)(NEW(NumExpr)(1), NEW(NumExpr)(2)), NEW(NumExpr)(3))));
    CHECK( parse_str("4*2*3")
          ->equals(NEW(MultExpr)(NEW(NumExpr)(4), NEW(MultExpr)(NEW(NumExpr)(2), NEW(NumExpr)(3)))));
    CHECK( parse_str("4+2+3")
          ->equals(NEW(AddExpr)(NEW(NumExpr)(4), NEW(AddExpr)(NEW(NumExpr)(2), NEW(NumExpr)(3)))));
    CHECK( parse_str("4*(2+3)")
          ->equals(NEW(MultExpr)(NEW(NumExpr)(4), NEW(AddExpr)(NEW(NumExpr)(2), NEW(NumExpr)(3)))));
    CHECK( parse_str("(2+3)*4")
          ->equals(NEW(MultExpr)(NEW(AddExpr)(NEW(NumExpr)(2), NEW(NumExpr)(3)), NEW(NumExpr)(4))));
    CHECK( parse_str("xyz")
          ->equals(NEW(VarExpr)("xyz")));
    CHECK( parse_str("xyz+1")
          ->equals(NEW(AddExpr)(NEW(VarExpr)("xyz"), NEW(NumExpr)(1))) );
    CHECK( parse_str(" 10 ")
          ->equals(NEW(NumExpr)(10)));
    CHECK( parse_str(" (  10 ) ")
          ->equals(NEW(NumExpr)(10)));
    CHECK( parse_str(" 10  + 1")
          ->equals(ten_plus_one));
    CHECK( parse_str(" ( 10 + 1 ) ")
          ->equals(ten_plus_one));
    CHECK( parse_str(" 11 * ( 10 + 1 ) ")
          ->equals(NEW(MultExpr)(NEW(NumExpr)(11), ten_plus_one)));
    CHECK( parse_str(" ( 11 * 10 ) + 1 ")
          ->equals(NEW(AddExpr)(NEW(MultExpr)(NEW(NumExpr)(11), NEW(NumExpr)(10)), NEW(NumExpr) (1))));
    CHECK( parse_str(" 1 + 2 * 3 ")
          ->equals(NEW(AddExpr)(NEW(NumExpr)(1), NEW(MultExpr)(NEW(NumExpr)(2), NEW(NumExpr)(3)))));
    CHECK( parse_str("_true")
          ->equals(NEW(BoolExpr)(true)));
    CHECK( parse_str("_false")
          ->equals(NEW(BoolExpr)(false)));
    CHECK( parse_str("_let x = 5 _in x + 1")
          ->equals(NEW(LetExpr)("x", NEW(NumExpr)(5), NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(1)))));
    CHECK( parse_str("_let x = 5 _in x + x")
          ->equals(NEW(LetExpr)("x", NEW(NumExpr)(5), NEW(AddExpr)(NEW(VarExpr)("x"), NEW(VarExpr)("x")))));
    CHECK( parse_str("_let x = 5 _in x * 2")
          ->equals(NEW(LetExpr)("x", NEW(NumExpr)(5), NEW(MultExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(2)))));
    CHECK( parse_str("_let x = 5 _in _let x = 3 _in x + 1")
          ->equals(NEW(LetExpr)("x", NEW(NumExpr)(5), NEW(LetExpr)("x", NEW(NumExpr)(3), NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(1))))));
    CHECK( parse_str("_let y = 5 _in _let x = 3 _in x + 1")
          ->equals(NEW(LetExpr)("y", NEW(NumExpr)(5), NEW(LetExpr)("x", NEW(NumExpr)(3), NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(1))))));
    CHECK( parse_str("_let y = 3 _in _let x = 5 _in y + 1")
          ->equals(NEW(LetExpr)("y", NEW(NumExpr)(3), NEW(LetExpr)("x", NEW(NumExpr)(5), NEW(AddExpr)(NEW(VarExpr)("y"), NEW(NumExpr)(1))))));
    CHECK( parse_str("_let x = _true _in x")
          ->equals(NEW(LetExpr)("x", NEW(BoolExpr)(true), NEW(VarExpr)("x"))));
    CHECK( parse_str("1 == 1")
          ->equals(NEW(EqualExpr)(NEW(NumExpr)(1), NEW(NumExpr)(1))));
    CHECK( parse_str("x == 1")
          ->equals(NEW(EqualExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(1))));
    CHECK( parse_str("_if _true _then 3 _else 4")
          ->equals(NEW(IfExpr)(NEW(BoolExpr)(true), NEW(NumExpr)(3), NEW(NumExpr)(4))));
    CHECK( parse_str("_if _false _then 5 _else x")
          ->equals(NEW(IfExpr)(NEW(BoolExpr)(false), NEW(NumExpr)(5), NEW(VarExpr)("x"))));
    CHECK( parse_str("_if 3 == 2 + 1 _then _true _else _false")
          ->equals(NEW(IfExpr)(NEW(EqualExpr)(NEW(NumExpr)(3), NEW(AddExpr)(NEW(NumExpr)(2), NEW(NumExpr)(1))), NEW(BoolExpr)(true), NEW(BoolExpr)(false))));
    CHECK( parse_str("_fun (x) x + 1")
          ->equals(NEW(FunExpr)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(1)))));
    CHECK( parse_str("_fun (x) x * x")
          ->equals(NEW(FunExpr)("x", NEW(MultExpr)(NEW(VarExpr)("x"), NEW(VarExpr)("x")))));
    CHECK( parse_str("(_fun (x) x + 1) (2)")->interp(NEW(EmptyEnv)())
          ->equals(NEW(NumVal)(3)));
    CHECK( parse_str("_let f = _fun (x) x + 1 _in f(3)")->interp(NEW(EmptyEnv)())
          ->equals(NEW(NumVal)(4)));
}

TEST_CASE( "Parse Error Tests" ) {
    CHECK( parse_str_error(" ! ")
          == "expected a digit or open parenthesis at !");
    CHECK( parse_str_error(" ( 1 ")
          == "expected a close parenthesis");
    CHECK( parse_str_error(" 1 )")
          == "expected end of file at )");
    CHECK( parse_str_error("_maybe ")
          == "unexpected keyword _maybe");
    CHECK ( parse_str_error("!")
           == "expected a digit or open parenthesis at !");
    CHECK ( parse_str_error("(1")
           == "expected a close parenthesis");
    CHECK ( parse_str_error(" ( 1 ")
           == "expected a close parenthesis");
    CHECK( parse_str_error("_if _true _else")
          == "expected keyword _then");
    CHECK( parse_str_error("_if _true _then 1 _then")
          == "expected keyword _else");
    CHECK( parse_str_error("_fun x) x + 1")
          == "expected an open parenthesis");
    CHECK( parse_str_error("_fun (x x + 1")
          == "expected a close parenthesis");
    CHECK( parse_str_error("_fun (x) x + 1 (2")
          == "expected a close parenthesis");
}

TEST_CASE( "Extra Complicated Parse Tests" ) {
    CHECK( parse_str("_let z =(_let x = 5 _in x) _in (z+1)")->optimize()->to_string()
          == "6");
    CHECK( parse_str("(_let x = 5 _in x) + 3")->optimize()
          ->equals(NEW(NumExpr)(8)));
    CHECK( parse_str("(_let x = 5 _in x) + 3")->optimize()->to_string()
          == "8");
    CHECK( parse_str("1 == 1")->optimize()
          ->equals(NEW(BoolExpr)(true)));
    CHECK( parse_str("1 == 2")->optimize()
          ->equals(NEW(BoolExpr)(false)));
    CHECK( parse_str("1 == 1")->to_string()
          == "1 == 1");
    CHECK( parse_str("_if 3 == 2 + 1 _then _true _else _false")->optimize()
          ->equals(NEW(BoolExpr)(true)));
    CHECK( parse_str("_if 3 == 2 + 1 _then _true _else _false")->to_string()
          == "(_if 3 == (2 + 1) _then _true _else _false)");
    CHECK( parse_str("_let y = 3 _in _let x = 2 _in x + y")->interp(NEW(EmptyEnv)())
          ->equals(NEW(NumVal)(5)));
    CHECK( parse_str("_let y = 8 _in _let f = _fun (x) x*y _in f(2)")->interp(NEW(EmptyEnv)())
          ->equals(NEW(NumVal)(16)));
    CHECK( (parse_str("_let add = (_fun (x) (_fun (y) (x + y))) _in _let addFive = add(5) _in addFive(10)"))->interp(NEW(EmptyEnv)())->to_string()
          == "15" );
    CHECK( parse_str("_let f = _fun (x) x*8 _in f(2)")->interp(NEW(EmptyEnv)())->to_string()
          == "16" );
    CHECK( parse_str("_let y = 8 _in _let f = _fun (x) x*y _in f(2)")->interp(NEW(EmptyEnv)())->to_string()
          == "16");
    
    
    // Function Stress Test Example - fib(fib)(10) = 89
    PTR(Expr) fib = parse_str("_let fib = _fun (fib) _fun (x) _if x == 0 _then 1 _else _if x == 2 + -1 _then 1 _else fib(fib)(x + -1) + fib(fib)(x + -2) _in fib(fib)(10)");
    CHECK( fib->interp(NEW(EmptyEnv)())
          ->equals(NEW(NumVal)(89)));
    
    PTR(Expr) fact = parse_str("_let factrl = _fun (factrl) _fun (x) _if x == 1 _then 1 _else x * factrl(factrl)(x + -1) _in _let factorial = factrl(factrl) _in factorial(5)");
    CHECK( fact->interp(NEW(EmptyEnv)())->to_string() == "120");
    CHECK(parse_str("f(10)(1)")
          ->equals(NEW(CallExpr)(NEW(CallExpr)(NEW(VarExpr)("f"),NEW(NumExpr)(10)),NEW(NumExpr)(1))));

    PTR(Expr) simple_step = parse_str("_let f = _fun (x) x*8 _in f(2)");
    PTR(Val) simple_step_result = Step::interp_by_steps(simple_step);
    CHECK( simple_step_result->to_string() == "16");
    
    
    PTR(Expr) complex_step = parse_str("_let fib = _fun (fib) _fun (x) _if x == 0 _then 1 _else _if x == 2 + -1 _then 1 _else fib(fib)(x + -1) + fib(fib)(x + -2) _in fib(fib)(28)");
    PTR(Val) complex_result = Step::interp_by_steps(complex_step);
    CHECK( complex_result->to_string() == "514229");
}
