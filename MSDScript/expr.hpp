#ifndef expr_hpp
#define expr_hpp

#include <string>
#include "macros.hpp"

class Env;
class Val;

class Expr ENABLE_THIS(Expr) {
public:
    // Compares to Exprs for equality
    virtual bool equals(PTR(Expr) other_expr) = 0;
    // Returns true if the Expr contains a variable item (ie "x")
    virtual bool has_var() = 0;
    // To compute the number value of an expression,
    virtual PTR(Val) interp(PTR(Env) env) = 0;
    // Prevents stack overflow interpetation
    virtual void step_interp() = 0;
    // To substitute a number in place of a variable
    virtual PTR(Expr) subst(std::string var, PTR(Val) val) = 0;
    // To "simplify" or optimize the input to its fastest version
    virtual PTR(Expr) optimize() = 0;
    // Converts Expr to string
    virtual std::string to_string() = 0;
};

class NumExpr : public Expr {
public:
    int rep;
    PTR(Val) val;
    
    NumExpr(int rep);
    bool equals(PTR(Expr) other_expr);
    bool has_var();
    
    PTR(Val) interp(PTR(Env) env);
    void step_interp();
    PTR(Expr) subst(std::string var, PTR(Val) val);
    PTR(Expr) optimize();
    std::string to_string();
};

class AddExpr : public Expr {
public:
    PTR(Expr) lhs;
    PTR(Expr) rhs;
    
    AddExpr(PTR(Expr) lhs, PTR(Expr) rhs);
    bool equals(PTR(Expr) other_expr);
    bool has_var();
    
    PTR(Val) interp(PTR(Env) env);
    void step_interp();
    PTR(Expr) subst(std::string var, PTR(Val) val);
    PTR(Expr) optimize();
    std::string to_string();
};

class MultExpr : public Expr {
public:
    PTR(Expr) lhs;
    PTR(Expr) rhs;
    
    MultExpr(PTR(Expr) lhs, PTR(Expr) rhs);
    bool equals(PTR(Expr) other_expr);
    bool has_var();
    
    PTR(Val) interp(PTR(Env) env);
    void step_interp();
    PTR(Expr) subst(std::string var, PTR(Val) val);
    PTR(Expr) optimize();
    std::string to_string();
};

class VarExpr : public Expr {
public:
    std::string name;
    
    VarExpr(std::string name);
    bool equals(PTR(Expr) other_expr);
    bool has_var();
    
    
    PTR(Val) interp(PTR(Env) env);
    void step_interp();
    PTR(Expr) subst(std::string var, PTR(Val) val);
    PTR(Expr) optimize();
    std::string to_string();
};

class LetExpr : public Expr {
public:
    std::string name;
    PTR(Expr) rhs;
    PTR(Expr) body;
    
    LetExpr(std::string name, PTR(Expr) rhs, PTR(Expr) body);
    bool equals(PTR(Expr) other_expr);
    bool has_var();
    
    PTR(Val) interp(PTR(Env) env);
    void step_interp();
    PTR(Expr) subst(std::string var, PTR(Val) new_val);
    PTR(Expr) optimize();
    std::string to_string();
};

class BoolExpr : public Expr {
public:
    bool rep;
    
    BoolExpr(bool rep);
    bool equals(PTR(Expr) other_expr);
    bool has_var();
    
    PTR(Val) interp(PTR(Env) env);
    void step_interp();
    PTR(Expr) subst(std::string var, PTR(Val) val);
    PTR(Expr) optimize();
    std::string to_string();
};

class EqualExpr : public Expr {
public:
    PTR(Expr) lhs;
    PTR(Expr) rhs;
    
    EqualExpr(PTR(Expr) lhs, PTR(Expr) rhs);
    bool equals(PTR(Expr) other_expr);
    bool has_var();
    
    PTR(Val) interp(PTR(Env) env);
    void step_interp();
    PTR(Expr) subst(std::string var, PTR(Val) val);
    PTR(Expr) optimize();
    std::string to_string();
};

class IfExpr : public Expr {
public:
    PTR(Expr) test_part;
    PTR(Expr) then_part;
    PTR(Expr) else_part;
    
    IfExpr(PTR(Expr) test_part, PTR(Expr) then_part, PTR(Expr) else_part);
    bool equals(PTR(Expr) other_expr);
    bool has_var();
    
    PTR(Val) interp(PTR(Env) env);
    void step_interp();
    PTR(Expr) subst(std::string var, PTR(Val) val);
    PTR(Expr) optimize();
    std::string to_string();
};

class FunExpr : public Expr {
public:
    std::string formal_arg;
    PTR(Expr) body;
    
    FunExpr(std::string arg, PTR(Expr) body);
    bool equals(PTR(Expr) other_expr);
    bool has_var();
    
    PTR(Val) interp(PTR(Env) env);
    void step_interp();
    PTR(Expr) subst(std::string var, PTR(Val) val);
    PTR(Expr) optimize();
    std::string to_string();
};

class CallExpr : public Expr {
public:
    PTR(Expr) to_be_called;
    PTR(Expr) actual_arg;
    
    CallExpr(PTR(Expr) to_be, PTR(Expr) actual);
    bool equals(PTR(Expr) other_expr);
    bool has_var();
    
    PTR(Val) interp(PTR(Env) env);
    void step_interp();
    PTR(Expr) subst(std::string var, PTR(Val) val);
    PTR(Expr) optimize();
    std::string to_string();
};

#endif /* expr_hpp */
