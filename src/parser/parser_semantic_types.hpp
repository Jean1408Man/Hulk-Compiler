#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../ast/abs_nodes/decl.h"
#include "../ast/abs_nodes/expr.h"
#include "../ast/assignments/desctructiveAssign.h"
#include "../ast/assignments/destructiveAssignMember.h"
#include "../ast/binOps/arithmeticBinOp.h"
#include "../ast/binOps/logicBinOp.h"
#include "../ast/binOps/stringBinOp.h"
#include "../ast/conditionals/ifStmt.h"
#include "../ast/domainFunctions/builtinCall.h"
#include "../ast/domainFunctions/print.h"
#include "../ast/functions/functionCall.h"
#include "../ast/functions/functionDecl.h"
#include "../ast/functions/param.h"
#include "../ast/literales/boolean.h"
#include "../ast/literales/number.h"
#include "../ast/literales/string.h"
#include "../ast/loops/for.h"
#include "../ast/loops/while.h"
#include "../ast/others/baseCall.h"
#include "../ast/others/exprBlock.h"
#include "../ast/others/program.h"
#include "../ast/others/selfRef.h"
#include "../ast/protocols/protocolDecl.h"
#include "../ast/protocols/protocolMethodSig.h"
#include "../ast/types/asExpr.h"
#include "../ast/types/isExpr.h"
#include "../ast/types/memberAccess.h"
#include "../ast/types/methodCall.h"
#include "../ast/types/newExpr.h"
#include "../ast/types/typeDecl.h"
#include "../ast/types/typeMemberAttribute.h"
#include "../ast/types/typeMemberMethod.h"
#include "../ast/unaryOps/arithmeticUnaryOp.h"
#include "../ast/unaryOps/logicUnaryOp.h"
#include "../ast/variables/letIn.h"
#include "../ast/variables/variableBinding.h"
#include "../ast/variables/variableReference.h"

namespace hulk::parser {

using ExprPtr = std::unique_ptr<Hulk::Expr>;
using ExprList = std::vector<ExprPtr>;
using DeclPtr = std::unique_ptr<Hulk::Decl>;
using DeclList = std::vector<DeclPtr>;
using ProgramPtr = std::unique_ptr<Hulk::Program>;
using BindingPtr = std::unique_ptr<Hulk::VariableBinding>;
using BindingList = std::vector<BindingPtr>;
using ParamList = std::vector<Hulk::Param>;
using ProtocolMethodList = std::vector<Hulk::ProtocolMethodSig>;
using ElifList = std::vector<Hulk::ElifBranch>;
using TypeMemberList = std::vector<Hulk::TypeMember>;

struct InheritsInfo {
    std::string parentName;
    ExprList parentArgs;
    bool hasParent = false;
};

struct LValueTarget {
    ExprPtr object;
    std::string name;
    bool isMember = false;
};

struct TopLevelItems {
    DeclList decls;
    ExprPtr globalExpr;
    bool hasGlobalExpr = false;
};

}
