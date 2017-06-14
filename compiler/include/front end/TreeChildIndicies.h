//------------------------------------------------------------------------------------------
// File: TreeChildIndicies.h
// Desc: Indicies of the different children for each tree node
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#ifndef __TREECHILDINDICIES_H__
#define __TREECHILDINDICIES_H__

//because the child nodes of a given CTreeNode* are stored in
//a std::vector, we use standard notation to access them by index. This
//file stores the position of each child for the different array nodes so when
//we access them in the code we use these identifiers rather than magic numbers

//array decls
#define ARR_DECL_VAR    0
#define ARR_DECL_BOUNDS 1

//array references
#define ARR_REF_INDEX	0

//assignment expressions
#define ASSIGN_EXPR_ID	0
#define ASSIGN_EXPR_RHS	1

//arithmetic expressions
#define ARITH_EXPR_LHS	0
#define ARITH_EXPR_RHS	1

//boolean/conditional expressions
#define BLN_EXPR_LHS	0
#define BLN_EXPR_RHS	1

//function decls
#define FUNC_DECL_PARM_LIST	0

//if statements
#define IF_STMT_COND_EXPR	0
#define IF_STMT_BODY			1
#define IF_STMT_ELSE_BODY 2

//return statements
#define RET_STMT_RET_EXPR	0

//unary expressions
#define UNARY_EXPR_EXPR	0

//while statements
#define WHILE_STMT_COND_EXPR	0

//parameter decls
#define PARM_DECL_ID 0

//literal references
#define LITREF_LITERAL 0

#endif
