#include <stdlib.h>
#include <llvm-c/Core.h>

#include "y.tab.h"

const char *type_name(enum value_type t);

enum expr_type {
  BOOL_LIT,
  LITERAL,
  VARIABLE,
  BIN_OP,
  EXPR_TERNARY,
};

struct expr {
  enum expr_type type;
  union {
    int value; // for type == LITERAL || type == BOOL_LIT
    size_t id; // for type == VARIABLE
    struct {
      struct expr *lhs;
      struct expr *rhs;
      int op;
    } binop; // for type == BIN_OP

    //#####################################################################################################################
    struct {
      struct expr *cond;
      struct expr *if_true;
      struct expr *if_false;
    }expr_ternary;
    //#####################################################################################################################
  };
};

struct expr* bool_lit(int v);
struct expr* literal(int v);
struct expr* variable(size_t id);
struct expr* binop(struct expr *lhs, int op, struct expr *rhs);
//#####################################################################################################################
struct expr* make_expr_ternary(struct expr *cond, struct expr *if_true, struct expr *if_false);
//#####################################################################################################################

void print_expr(struct expr *expr);
void emit_stack_machine(struct expr *expr);
int emit_reg_machine(struct expr *expr);

enum value_type check_types(struct expr *expr);

void free_expr(struct expr *expr);

enum stmt_type {
  STMT_SEQ,
  STMT_ASSIGN,
  STMT_IF,
  STMT_WHILE,
  STMT_PRINT,
  STMT_TERNARY,
  STMT_LOOP,
};

struct stmt {
  enum stmt_type type;
  union {
    struct {
      size_t id;
      struct expr *expr;
    } assign; // for type == STMT_ASSIGN
    struct {
      struct stmt *fst, *snd;
    } seq; // for type == STMT_SEQ
    struct {
      struct expr *cond;
      struct stmt *if_body, *else_body;
    } ifelse; // for type == STMT_IF
    struct {
      struct expr *cond;
      struct stmt *body;
    } while_; // for type == STMT_WHILE
    struct {
      struct expr *expr;
    } print; // for type == STMT_PRINT
    //@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    struct {
      struct expr *cond;
      struct expr *if_true;
      struct expr *if_false;
    } ternary; //for type == STMT_TERNARY 
    //@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    //&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
    struct {
      struct expr *num;
      struct stmt *body;
    } loop; // for type == STMT_LOOP
    //&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
  };
};

struct stmt* make_seq(struct stmt *fst, struct stmt *snd);
struct stmt* make_assign(size_t id, struct expr *e);
struct stmt* make_while(struct expr *e, struct stmt *body);
struct stmt* make_ifelse(struct expr *e, struct stmt *if_body, struct stmt *else_body);
struct stmt* make_if(struct expr *e, struct stmt *body);
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
struct stmt* make_ternary(struct expr *cond, struct expr *if_true, struct expr *if_false);
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
struct stmt* make_loop(struct expr *num, struct stmt *body);
//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&

struct stmt* make_print(struct expr *e);
void free_stmt(struct stmt *stmt);
void print_stmt(struct stmt *stmt, int indent);
int valid_stmt(struct stmt *stmt);

LLVMValueRef codegen_expr(struct expr *expr, LLVMModuleRef module, LLVMBuilderRef builder);
void codegen_stmt(struct stmt *stmt, LLVMModuleRef module, LLVMBuilderRef builder);
