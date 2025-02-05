#include <stdio.h>
#include <stdlib.h>

#include "ast.h"
#include "y.tab.h"
#include "utils.h"

const char *type_name(enum value_type t) {
  switch (t) {
    case INTEGER: return "int";
    case BOOLEAN: return "bool";
    case ERROR: return "error";
    default: return "not-a-type";
  }
}

struct expr* bool_lit(int v) {
  struct expr* r = malloc(sizeof(struct expr));
  r->type = BOOL_LIT;
  r->value = v;
  return r;
}

struct expr* literal(int v) {
  struct expr* r = malloc(sizeof(struct expr));
  r->type = LITERAL;
  r->value = v;
  return r;
}

struct expr* variable(size_t id) {
  struct expr* r = malloc(sizeof(struct expr));
  r->type = VARIABLE;
  r->id = id;
  return r;
}

struct expr* binop(struct expr *lhs, int op, struct expr *rhs) {
  struct expr* r = malloc(sizeof(struct expr));
  r->type = BIN_OP;
  r->binop.lhs = lhs;
  r->binop.op = op;
  r->binop.rhs = rhs;
  return r;
}

void print_expr(struct expr *expr) {
  switch (expr->type) {
    case BOOL_LIT:
      printf("%s", expr->value ? "true" : "false");
      break;

    case LITERAL:
      printf("%d", expr->value);
      break;

    case VARIABLE:
      printf("%s", string_int_rev(&global_ids, expr->id));
      break;

    case BIN_OP:
      printf("(");
      print_expr(expr->binop.lhs);
      switch (expr->binop.op) {
        case EQ: printf(" == "); break;
        case NE: printf(" != "); break;
        case GE: printf(" >= "); break;
        case LE: printf(" <= "); break;
        //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        case AND: printf(" && "); break;
        case OR: printf(" || "); break;
        case XOR: printf(" ^ "); break;
        //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
       
        default: printf(" %c ", expr->binop.op); break;
      }
      print_expr(expr->binop.rhs);
      printf(")");
      break;  
  }
}

static void print_indent(int indent) {
  while (indent--) {
    printf("  ");
  }
}

void print_stmt(struct stmt *stmt, int indent) {
  switch (stmt->type) {
    case STMT_SEQ:
      print_stmt(stmt->seq.fst, indent);
      print_stmt(stmt->seq.snd, indent);
      break;

    case STMT_ASSIGN:
      print_indent(indent);
      printf("%s = ", string_int_rev(&global_ids, stmt->assign.id));
      print_expr(stmt->assign.expr);
      printf(";\n");
      break;

    case STMT_PRINT:
      print_indent(indent);
      printf("print ");
      print_expr(stmt->print.expr);
      printf(";\n");
      break;

    case STMT_WHILE:
      print_indent(indent);
      printf("while (");
      print_expr(stmt->while_.cond);
      printf(") {\n");
      print_stmt(stmt->while_.body, indent + 1);
      print_indent(indent);
      printf("}\n");
      break;

    //&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
    case STMT_LOOP: 
      print_indent(indent);
      printf("loop (");
      printf("%s = ", string_int_rev(&global_ids, stmt->loop_.num));
      printf(") {\n");
      print_stmt(stmt->loop_.body, indent + 1);
      print_indent(indent);
      printf("}\n");
      break;
    //&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&

    //@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    case STMT_TERNARY:
      print_indent(indent);
      printf("%s = ", string_int_rev(&global_ids, stmt->ternary.id));
      printf("(");
      print_expr(stmt->ternary.cond);
      printf(") ? ");
      print_expr(stmt->ternary.if_true);
      printf(" : ");
      print_expr(stmt->ternary.if_false);      
      printf("\n");
      break;

    case STMT_TER_ASSIGN:
      print_indent(indent);
      printf("(");
      print_expr(stmt->ter_assign.cond);
      printf(") ? ");
      printf("%s = ", string_int_rev(&global_ids, stmt->ter_assign.id1));
      printf(" : ");
      printf("%s = ", string_int_rev(&global_ids, stmt->ter_assign.id2));
      printf(" = ");
      print_expr(stmt->ter_assign.e);
      break;
    //@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

    case STMT_IF:
      print_indent(indent);
      printf("if (");
      print_expr(stmt->ifelse.cond);
      printf(") {\n");
      print_stmt(stmt->ifelse.if_body, indent + 1);
      if (stmt->ifelse.else_body) {
        print_indent(indent);
        printf("} else {\n");
        print_stmt(stmt->ifelse.else_body, indent + 1);
      }
      print_indent(indent);
      printf("}\n");
      break;
  }
}

void emit_stack_machine(struct expr *expr) {
  switch (expr->type) {
    case BOOL_LIT:
      printf(expr->value ? "load_true\n" : "load_false\n");
      break;

    case LITERAL:
      printf("load_imm %d\n", expr->value);
      break;

    case VARIABLE:
      printf("load_mem %zu # %s\n", expr->id, string_int_rev(&global_ids, expr->id));
      break;

    case BIN_OP:
      emit_stack_machine(expr->binop.lhs);
      emit_stack_machine(expr->binop.rhs);
      switch (expr->binop.op) {
        case '+': printf("add\n"); break;
        case '-': printf("sub\n"); break;
        case '*': printf("mul\n"); break;
        case '/': printf("div\n"); break;

        //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        case AND: printf("and\n"); break;
        case OR: printf("or\n"); break;
        case XOR: printf("xor\n"); break;
        //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        
        case EQ: printf("eq\n"); break;
        case NE: printf("ne\n"); break;

        case GE: printf("ge\n"); break;
        case LE: printf("le\n"); break;
        case '>': printf("gt\n"); break;
        case '<': printf("lt\n"); break;
      }
      break;  
  }
}

static int next_reg = 0;

static int gen_reg() {
  return next_reg++;
}

int emit_reg_machine(struct expr *expr) {
  int result_reg = gen_reg();
  switch (expr->type) {
    case BOOL_LIT:
      printf("r%d = %d\n", result_reg, expr->value);
      break;

    case LITERAL:
      printf("r%d = %d\n", result_reg, expr->value);
      break;

    case VARIABLE:
      printf("r%d = load %zu # %s\n", result_reg, expr->id, string_int_rev(&global_ids, expr->id));
      break;

    case BIN_OP: {
      int lhs = emit_reg_machine(expr->binop.lhs);
      int rhs = emit_reg_machine(expr->binop.rhs);
      switch (expr->binop.op) {
        case '+': printf("r%d = add r%d, r%d\n", result_reg, lhs, rhs); break;
        case '-': printf("r%d = sub r%d, r%d\n", result_reg, lhs, rhs); break;
        case '*': printf("r%d = mul r%d, r%d\n", result_reg, lhs, rhs); break;
        case '/': printf("r%d = div r%d, r%d\n", result_reg, lhs, rhs); break;
        
        //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        case AND: printf("r%d = and r%d, r%d\n", result_reg, lhs, rhs); break;
        case OR: printf("r%d = or r%d, r%d\n", result_reg, lhs, rhs); break;
        case XOR: printf("r%d = xor r%d, r%d\n", result_reg, lhs, rhs); break;
        //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        
        case EQ: printf("r%d = eq r%d, r%d\n", result_reg, lhs, rhs); break;
        case NE: printf("r%d = ne r%d, r%d\n", result_reg, lhs, rhs); break;

        case GE: printf("r%d = ge r%d, r%d\n", result_reg, lhs, rhs); break;
        case LE: printf("r%d = le r%d, r%d\n", result_reg, lhs, rhs); break;
        case '>': printf("r%d = gt r%d, r%d\n", result_reg, lhs, rhs); break;
        case '<': printf("r%d = lt r%d, r%d\n", result_reg, lhs, rhs); break;
      }
      break;
    }  
  }
  return result_reg;
}

enum value_type check_types(struct expr *expr) {
  switch (expr->type) {
    case BOOL_LIT:
      return BOOLEAN;

    case LITERAL:
      return INTEGER;

    case VARIABLE: {
      LLVMValueRef ptr = vector_get(&global_types, expr->id);
      LLVMTypeRef t = LLVMGetElementType(LLVMTypeOf(ptr));
      return LLVMGetIntTypeWidth(t) == 1 ? BOOLEAN : INTEGER;
    }

    case BIN_OP: {
      enum value_type lhs = check_types(expr->binop.lhs);
      enum value_type rhs = check_types(expr->binop.rhs);
      switch (expr->binop.op) {
        case '+':
        case '-':
        case '*':
        case '/':
          if (lhs == INTEGER && rhs == INTEGER)
            return INTEGER;
          else
            return ERROR;

        case EQ:
        case NE:
          if (lhs == rhs && lhs != ERROR)
            return BOOLEAN;
          else
            return ERROR;

        case GE:
        case LE:
        case '>':
        case '<':
          if (lhs == INTEGER && rhs == INTEGER)
            return BOOLEAN;
          else
            return ERROR;
        
        //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        case AND:
        case OR:
        case XOR:
          if (lhs == BOOLEAN && rhs == BOOLEAN)
            return BOOLEAN;
          else if (lhs == INTEGER && rhs == INTEGER)
            return INTEGER;
          else
            return ERROR;
        //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        

      }

      default:
        return ERROR;
    }
  }
}

void free_expr(struct expr *expr) {
  switch (expr->type) {
    case BOOL_LIT:
    case LITERAL:
    case VARIABLE:
      free(expr);
      break;

    case BIN_OP:
      free_expr(expr->binop.lhs);
      free_expr(expr->binop.rhs);
      free(expr);
      break;
  }
}

struct stmt* make_seq(struct stmt *fst, struct stmt *snd) {
  struct stmt* r = malloc(sizeof(struct stmt));
  r->type = STMT_SEQ;
  r->seq.fst = fst;
  r->seq.snd = snd;
  return r;
}

struct stmt* make_assign(size_t id, struct expr *e) {
  struct stmt* r = malloc(sizeof(struct stmt));
  r->type = STMT_ASSIGN;
  r->assign.id = id;
  r->assign.expr = e;
  return r;
}

struct stmt* make_while(struct expr *e, struct stmt *body) {
  struct stmt* r = malloc(sizeof(struct stmt));
  r->type = STMT_WHILE;
  r->while_.cond = e;
  r->while_.body = body;
  return r;
}

struct stmt* make_ifelse(struct expr *e, struct stmt *if_body, struct stmt *else_body) {
  struct stmt* r = malloc(sizeof(struct stmt));
  r->type = STMT_IF;
  r->ifelse.cond = e;
  r->ifelse.if_body = if_body;
  r->ifelse.else_body = else_body;
  return r;
}

//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
struct stmt* make_loop(size_t num, struct stmt *body) {
  struct stmt* r = malloc(sizeof(struct stmt));
  r->type = STMT_LOOP;
  r->loop_.num = num;
  r->loop_.body = body;
  return r;
}
//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
struct stmt* make_ternary(size_t id, struct expr *cond, struct expr *if_true, struct expr *if_false){
  struct stmt* r = malloc(sizeof(struct stmt));
  r->type = STMT_TERNARY;
  r->ternary.id = id;
  r->ternary.cond = cond;
  r->ternary.if_true = if_true;
  r->ternary.if_false = if_false;
  return r;   
}
struct stmt* make_ter_assign(struct expr *cond, size_t id1, size_t id2, struct expr *e){
  struct stmt* r = malloc(sizeof(struct stmt));
  r->type = STMT_TERNARY;
  r->ter_assign.cond = cond;
  r->ter_assign.id1 = id1;
  r->ter_assign.id2 = id2;
  r->ter_assign.e = e;
  return r;
}
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

struct stmt* make_if(struct expr *e, struct stmt *body) {
  return make_ifelse(e, body, NULL);
}

struct stmt* make_print(struct expr *e) {
  struct stmt* r = malloc(sizeof(struct stmt));
  r->type = STMT_PRINT;
  r->print.expr = e;
  return r;
}

void free_stmt(struct stmt *stmt) {
  switch (stmt->type) {
    case STMT_SEQ:
      free_stmt(stmt->seq.fst);
      free_stmt(stmt->seq.snd);
      break;

    case STMT_ASSIGN:
      free_expr(stmt->assign.expr);
      break;

    case STMT_PRINT:
      free_expr(stmt->print.expr);
      break;

    case STMT_WHILE:
      free_expr(stmt->while_.cond);
      free_stmt(stmt->while_.body);
      break;
    
    //&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
    case STMT_LOOP:
      free_stmt(stmt->loop_.body);
      break;
    //&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&

    //@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    case STMT_TERNARY:
      free_expr(stmt->ternary.cond);
      free_expr(stmt->ternary.if_true);
      free_expr(stmt->ternary.if_false);
      break;

    case STMT_TER_ASSIGN:
      free_expr(stmt->ter_assign.cond);
      free_expr(stmt->ter_assign.e);
      break;
    //@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
  
    case STMT_IF:
      free_expr(stmt->ifelse.cond);
      free_stmt(stmt->ifelse.if_body);
      if (stmt->ifelse.else_body)
        free_stmt(stmt->ifelse.else_body);
      break;
  
  }

  free(stmt);
}

int valid_stmt(struct stmt *stmt) {
  switch (stmt->type) {
    case STMT_SEQ:
      return valid_stmt(stmt->seq.fst) && valid_stmt(stmt->seq.snd);

    case STMT_ASSIGN:
      // should the language/compiler forbid accessing uninitialized variables?
      // maybe also warn about dead assignments?
      return check_types(stmt->assign.expr) != ERROR;

    case STMT_PRINT:
      return check_types(stmt->print.expr) != ERROR;

    case STMT_WHILE:
      return 
        check_types(stmt->while_.cond) == BOOLEAN && 
        valid_stmt(stmt->while_.body);
    
    case STMT_IF:
      return
        check_types(stmt->ifelse.cond) == BOOLEAN &&
        valid_stmt(stmt->ifelse.if_body) &&
        (stmt->ifelse.else_body == NULL || valid_stmt(stmt->ifelse.else_body));

    //&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
    case STMT_LOOP:
      return 
        valid_stmt(stmt->loop_.body);
    //&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&

    //@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    case STMT_TERNARY:
      return
        (check_types(stmt->ternary.cond) == BOOLEAN &&
        check_types(stmt->ternary.if_true) != ERROR &&
        check_types(stmt->ternary.if_false) != ERROR);
    case STMT_TER_ASSIGN:
      return
        check_types(stmt->ter_assign.cond) == BOOLEAN &&
        check_types(stmt->ter_assign.e) != ERROR ;
    //@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
  }
  return 0;
}

LLVMValueRef codegen_expr(struct expr *expr, LLVMModuleRef module, LLVMBuilderRef builder) {
  switch (expr->type) {
    case BOOL_LIT:
      return LLVMConstInt(LLVMInt1Type(), expr->value, 0);

    case LITERAL:
      return LLVMConstInt(LLVMInt32Type(), expr->value, 0);

    case VARIABLE:
      return LLVMBuildLoad(builder, vector_get(&global_types, expr->id), "loadtmp");

    case BIN_OP: {
      LLVMValueRef lhs = codegen_expr(expr->binop.lhs, module, builder);
      LLVMValueRef rhs = codegen_expr(expr->binop.rhs, module, builder);
      switch (expr->binop.op) {
        case '+': return LLVMBuildAdd(builder, lhs, rhs, "addtmp");
        case '-': return LLVMBuildSub(builder, lhs, rhs, "subtmp");
        case '*': return LLVMBuildMul(builder, lhs, rhs, "multmp");
        case '/': return LLVMBuildSDiv(builder, lhs, rhs, "divtmp");

        //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        case AND: return LLVMBuildAnd(builder, lhs, rhs, "andtmp");
        case OR: return LLVMBuildOr(builder, lhs, rhs, "ortmp");
        case XOR: return LLVMBuildXor(builder, lhs, rhs, "xortmp");
        //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
              
        case EQ: return LLVMBuildICmp(builder, LLVMIntEQ, lhs, rhs, "eqtmp");
        case NE: return LLVMBuildICmp(builder, LLVMIntNE, lhs, rhs, "netmp");

        case GE: return LLVMBuildICmp(builder, LLVMIntSGE, lhs, rhs, "getmp");
        case LE: return LLVMBuildICmp(builder, LLVMIntSLE, lhs, rhs, "letmp");
        case '>': return LLVMBuildICmp(builder, LLVMIntSGT, lhs, rhs, "gttmp");
        case '<': return LLVMBuildICmp(builder, LLVMIntSLT, lhs, rhs, "lttmp");
      }
    }
  }
  return NULL;
}

void codegen_stmt(struct stmt *stmt, LLVMModuleRef module, LLVMBuilderRef builder) {
  switch (stmt->type) {
    case STMT_SEQ: {
      codegen_stmt(stmt->seq.fst, module, builder);
      codegen_stmt(stmt->seq.snd, module, builder);
      break;
    }

    case STMT_ASSIGN: {
      LLVMValueRef expr = codegen_expr(stmt->assign.expr, module, builder);
      LLVMBuildStore(builder, expr, vector_get(&global_types, stmt->assign.id));
      break;
    }

    case STMT_PRINT: {
      enum value_type arg_type = check_types(stmt->print.expr);
      LLVMValueRef print_fn = LLVMGetNamedFunction(module, arg_type == BOOLEAN ? "print_i1" : "print_i32");
      LLVMValueRef args[] = { codegen_expr(stmt->print.expr, module, builder) };
      LLVMBuildCall(builder, print_fn, args, 1, "");
      break;
    }

    case STMT_WHILE: {
      LLVMValueRef func = LLVMGetBasicBlockParent(LLVMGetInsertBlock(builder));
      LLVMBasicBlockRef cond_bb = LLVMAppendBasicBlock(func, "cond");
      LLVMBasicBlockRef body_bb = LLVMAppendBasicBlock(func, "body");
      LLVMBasicBlockRef cont_bb = LLVMAppendBasicBlock(func, "cont");

      LLVMBuildBr(builder, cond_bb);

      LLVMPositionBuilderAtEnd(builder, cond_bb);
      LLVMValueRef cond = codegen_expr(stmt->while_.cond, module, builder);
      LLVMBuildCondBr(builder, cond, body_bb, cont_bb);

      LLVMPositionBuilderAtEnd(builder, body_bb);
      codegen_stmt(stmt->while_.body, module, builder);
      LLVMBuildBr(builder, cond_bb);

      LLVMPositionBuilderAtEnd(builder, cont_bb);
      break;
    }   

     //&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
    case STMT_LOOP: {
      LLVMValueRef func = LLVMGetBasicBlockParent(LLVMGetInsertBlock(builder));
      LLVMBasicBlockRef cond_bb = LLVMAppendBasicBlock(func, "cond");
      LLVMBasicBlockRef body_bb = LLVMAppendBasicBlock(func, "body");
      LLVMBasicBlockRef cont_bb = LLVMAppendBasicBlock(func, "cont");

      LLVMBuildBr(builder, cond_bb);   

      LLVMPositionBuilderAtEnd(builder, cond_bb);
      struct expr *cond_tmp = binop(variable(stmt->loop_.num), '>', literal(1));
      LLVMValueRef cond = codegen_expr(cond_tmp, module, builder);
      LLVMBuildCondBr(builder, cond, body_bb, cont_bb);

      LLVMPositionBuilderAtEnd(builder, body_bb);
      codegen_stmt(stmt->loop_.body, module, builder);
      struct stmt * decr_tmp = make_assign(stmt->loop_.num, binop(variable(stmt->loop_.num), '-', literal(1)));
      codegen_stmt(decr_tmp, module, builder);
      LLVMBuildBr(builder, cond_bb);

      LLVMPositionBuilderAtEnd(builder, cont_bb);
      break;
    }
    //&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&

    //@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    case STMT_TERNARY: {
      LLVMValueRef func = LLVMGetBasicBlockParent(LLVMGetInsertBlock(builder));
      LLVMBasicBlockRef body_bb = LLVMAppendBasicBlock(func, "body");
      LLVMBasicBlockRef else_bb = LLVMAppendBasicBlock(func, "else");
      LLVMBasicBlockRef cont_bb = LLVMAppendBasicBlock(func, "cont");

      LLVMValueRef cond = codegen_expr(stmt->ternary.cond, module, builder);
      LLVMBuildCondBr(builder, cond, body_bb, else_bb);

      LLVMPositionBuilderAtEnd(builder, body_bb);
      struct stmt *body_tmp = make_assign(stmt->ternary.id, stmt->ternary.if_true);
      codegen_stmt(body_tmp, module, builder);
      LLVMBuildBr(builder, cont_bb);

      LLVMPositionBuilderAtEnd(builder, else_bb);
      struct stmt *else_tmp = make_assign(stmt->ternary.id, stmt->ternary.if_false);
      codegen_stmt(else_tmp, module, builder);      
      LLVMBuildBr(builder, cont_bb);

      LLVMPositionBuilderAtEnd(builder, cont_bb);
      break;
    } 

    case STMT_TER_ASSIGN: {
      LLVMValueRef func = LLVMGetBasicBlockParent(LLVMGetInsertBlock(builder));
      LLVMBasicBlockRef body_bb = LLVMAppendBasicBlock(func, "body");
      LLVMBasicBlockRef else_bb = LLVMAppendBasicBlock(func, "else");
      LLVMBasicBlockRef cont_bb = LLVMAppendBasicBlock(func, "cont");

      LLVMValueRef cond = codegen_expr(stmt->ter_assign.cond, module, builder);
      LLVMBuildCondBr(builder, cond, body_bb, else_bb);


      LLVMPositionBuilderAtEnd(builder, body_bb);
      struct stmt *body_tmp = make_assign(stmt->ter_assign.id1, stmt->ter_assign.e);
      codegen_stmt(body_tmp, module, builder);
      LLVMBuildBr(builder, cont_bb);

      LLVMPositionBuilderAtEnd(builder, else_bb);
      struct stmt *else_tmp = make_assign(stmt->ter_assign.id2, stmt->ter_assign.e);
      codegen_stmt(else_tmp, module, builder);      
      LLVMBuildBr(builder, cont_bb);

      LLVMPositionBuilderAtEnd(builder, cont_bb);
      break;
    }
    //@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

    case STMT_IF: {
      LLVMValueRef func = LLVMGetBasicBlockParent(LLVMGetInsertBlock(builder));
      LLVMBasicBlockRef body_bb = LLVMAppendBasicBlock(func, "body");
      LLVMBasicBlockRef else_bb = LLVMAppendBasicBlock(func, "else");
      LLVMBasicBlockRef cont_bb = LLVMAppendBasicBlock(func, "cont");

      LLVMValueRef cond = codegen_expr(stmt->ifelse.cond, module, builder);
      LLVMBuildCondBr(builder, cond, body_bb, else_bb);

      LLVMPositionBuilderAtEnd(builder, body_bb);
      codegen_stmt(stmt->ifelse.if_body, module, builder);
      LLVMBuildBr(builder, cont_bb);

      LLVMPositionBuilderAtEnd(builder, else_bb);
      if (stmt->ifelse.else_body) {
        codegen_stmt(stmt->ifelse.else_body, module, builder);
      }
      LLVMBuildBr(builder, cont_bb);

      LLVMPositionBuilderAtEnd(builder, cont_bb);
      break;
    }
  }
}
