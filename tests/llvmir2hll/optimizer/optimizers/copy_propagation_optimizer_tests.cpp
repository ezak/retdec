/**
* @file tests/llvmir2hll/optimizer/optimizers/copy_propagation_optimizer_tests.cpp
* @brief Tests for the @c copy_propagation_optimizer module.
* @copyright (c) 2017 Avast Software, licensed under the MIT license
*/

#include <gtest/gtest.h>

#include "llvmir2hll/analysis/tests_with_value_analysis.h"
#include "retdec/llvmir2hll/ir/add_op_expr.h"
#include "retdec/llvmir2hll/ir/address_op_expr.h"
#include "retdec/llvmir2hll/ir/assign_stmt.h"
#include "retdec/llvmir2hll/ir/call_expr.h"
#include "retdec/llvmir2hll/ir/call_stmt.h"
#include "retdec/llvmir2hll/ir/const_int.h"
#include "retdec/llvmir2hll/ir/const_null_pointer.h"
#include "retdec/llvmir2hll/ir/deref_op_expr.h"
#include "retdec/llvmir2hll/ir/empty_stmt.h"
#include "retdec/llvmir2hll/ir/function_builder.h"
#include "retdec/llvmir2hll/ir/if_stmt.h"
#include "retdec/llvmir2hll/ir/int_type.h"
#include "retdec/llvmir2hll/ir/pointer_type.h"
#include "retdec/llvmir2hll/ir/return_stmt.h"
#include "llvmir2hll/ir/tests_with_module.h"
#include "retdec/llvmir2hll/ir/var_def_stmt.h"
#include "retdec/llvmir2hll/ir/variable.h"
#include "retdec/llvmir2hll/ir/void_type.h"
#include "retdec/llvmir2hll/obtainer/call_info_obtainers/optim_call_info_obtainer.h"
#include "retdec/llvmir2hll/optimizer/optimizers/copy_propagation_optimizer.h"
#include "retdec/llvmir2hll/support/types.h"

#include "retdec/llvmir2hll/hll/bir_writer.h"

using namespace ::testing;

namespace retdec {
namespace llvmir2hll {
namespace tests {

/**
* @brief Tests for the @c copy_propagation_optimizer module.
*/
class CopyPropagationOptimizerTests: public TestsWithModule {};

TEST_F(CopyPropagationOptimizerTests,
OptimizerHasNonEmptyID) {
	INSTANTIATE_ALIAS_ANALYSIS_AND_VALUE_ANALYSIS(module);

	CopyPropagationOptimizer* optimizer(new CopyPropagationOptimizer(
		module, va, OptimCallInfoObtainer::create()));

	EXPECT_TRUE(!optimizer->getId().empty()) <<
		"the optimizer should have a non-empty ID";
}

TEST_F(CopyPropagationOptimizerTests,
InEmptyBodyThereIsNothingToOptimize) {
	INSTANTIATE_ALIAS_ANALYSIS_AND_VALUE_ANALYSIS(module);

	// Optimize the module.
	Optimizer::optimize<CopyPropagationOptimizer>(module, va,
		OptimCallInfoObtainer::create());

	// Check that the output is correct.
	ASSERT_TRUE(isa<EmptyStmt>(testFunc->getBody())) <<
		"expected EmptyStmt, got " << testFunc->getBody();
	EXPECT_TRUE(!testFunc->getBody()->hasSuccessor()) <<
		"expected no successors of the statement, but got `" <<
		testFunc->getBody()->getSuccessor() << "`";
}

TEST_F(CopyPropagationOptimizerTests,
LocalVariableInVarDefStmtWithNoUsesGetsRemoved) {
	// Set-up the module.
	//
	// void test() {
	//     int a;
	// }
	//
	Variable* varA(Variable::create("a", IntType::create(32)));
	testFunc->addLocalVar(varA);
	VarDefStmt* varDefA(VarDefStmt::create(varA));
	testFunc->setBody(varDefA);

	INSTANTIATE_ALIAS_ANALYSIS_AND_VALUE_ANALYSIS(module);

	// Optimize the module.
	Optimizer::optimize<CopyPropagationOptimizer>(module, va,
		OptimCallInfoObtainer::create());

	// Check that the output is correct.
	ASSERT_TRUE(testFunc->getBody()) <<
		"expected a non-empty body";
	EXPECT_TRUE(isa<EmptyStmt>(testFunc->getBody())) <<
		"expected EmptyStmt, got `" << testFunc->getBody() << "`";
}

TEST_F(CopyPropagationOptimizerTests,
LocalVariableInAssignStmtWithNoUsesGetsRemoved) {
	// Set-up the module.
	//
	// void test() {
	//     a = 1;
	// }
	//
	Variable* varA(Variable::create("a", IntType::create(32)));
	testFunc->addLocalVar(varA);
	AssignStmt* assignA1(AssignStmt::create(varA, ConstInt::create(1, 32)));
	testFunc->setBody(assignA1);

	INSTANTIATE_ALIAS_ANALYSIS_AND_VALUE_ANALYSIS(module);

	// Optimize the module.
	Optimizer::optimize<CopyPropagationOptimizer>(module, va,
		OptimCallInfoObtainer::create());

	// Check that the output is correct.
	ASSERT_TRUE(testFunc->getBody()) <<
		"expected a non-empty body";
	EXPECT_TRUE(isa<EmptyStmt>(testFunc->getBody())) <<
		"expected EmptyStmt, got `" << testFunc->getBody() << "`";
}

TEST_F(CopyPropagationOptimizerTests,
DoNotEliminateVarDefStmtWhenVariableHasNameFromDebugInfo) {
	// Set-up the module.
	//
	// void test() {
	//     int d; (the name is assigned from debug information)
	// }
	//
	Variable* varD(Variable::create("d", IntType::create(32)));
	testFunc->addLocalVar(varD);
	module->addDebugNameForVar(varD, varD->getName());
	VarDefStmt* varDefD(VarDefStmt::create(varD));
	testFunc->setBody(varDefD);

	INSTANTIATE_ALIAS_ANALYSIS_AND_VALUE_ANALYSIS(module);

	// Optimize the module.
	Optimizer::optimize<CopyPropagationOptimizer>(module, va,
		OptimCallInfoObtainer::create());

	// Check that the output hasn't been changed.
	ASSERT_TRUE(testFunc->getBody()) <<
		"expected a non-empty body";
	EXPECT_EQ(varDefD, testFunc->getBody()) <<
		"expected `" << varDefD << "`, "
		"got `" << testFunc->getBody() << "`";
}

TEST_F(CopyPropagationOptimizerTests,
DoNotEliminateAssignStmtWhenVariableHasNameFromDebugInfo) {
	// Set-up the module.
	//
	// void test() {
	//     d = 1; (the name is assigned from debug information)
	// }
	//
	Variable* varD(Variable::create("d", IntType::create(32)));
	testFunc->addLocalVar(varD);
	module->addDebugNameForVar(varD, varD->getName());
	AssignStmt* assignD1(AssignStmt::create(varD, ConstInt::create(1, 32)));
	testFunc->setBody(assignD1);

	INSTANTIATE_ALIAS_ANALYSIS_AND_VALUE_ANALYSIS(module);

	// Optimize the module.
	Optimizer::optimize<CopyPropagationOptimizer>(module, va,
		OptimCallInfoObtainer::create());

	// Check that the output hasn't been changed.
	ASSERT_TRUE(testFunc->getBody()) <<
		"expected a non-empty body";
	EXPECT_EQ(assignD1, testFunc->getBody()) <<
		"expected `" << assignD1 << "`, "
		"got `" << testFunc->getBody() << "`";
}

TEST_F(CopyPropagationOptimizerTests,
DoNotEliminateAssignStmtWhenVariableIsExternal) {
	// Set-up the module.
	//
	// void test() {
	//     d = 1; (d is 'external' and comes from a volatile store)
	// }
	//
	Variable* varD(Variable::create("d", IntType::create(32)));
	varD->markAsExternal();
	testFunc->addLocalVar(varD);
	AssignStmt* assignD1(AssignStmt::create(varD, ConstInt::create(1, 32)));
	testFunc->setBody(assignD1);

	INSTANTIATE_ALIAS_ANALYSIS_AND_VALUE_ANALYSIS(module);

	// Optimize the module.
	Optimizer::optimize<CopyPropagationOptimizer>(module, va,
		OptimCallInfoObtainer::create());

	// Check that the output hasn't been changed.
	ASSERT_TRUE(testFunc->getBody()) <<
		"expected a non-empty body";
	EXPECT_EQ(assignD1, testFunc->getBody()) <<
		"expected `" << assignD1 << "`, "
		"got `" << testFunc->getBody() << "`";
}

TEST_F(CopyPropagationOptimizerTests,
DotNotEliminateAssignIntoGlobalVariableIfThereIsNoSuccessiveAssignIntoIt) {
	// Set-up the module.
	//
	// int g;
	//
	// void test() {
	//     g = 1;
	// }
	//
	Variable* varG(Variable::create("g", IntType::create(32)));
	module->addGlobalVar(varG);
	AssignStmt* assignG1(AssignStmt::create(varG, ConstInt::create(1, 32)));
	testFunc->setBody(assignG1);

	INSTANTIATE_ALIAS_ANALYSIS_AND_VALUE_ANALYSIS(module);

	// Optimize the module.
	Optimizer::optimize<CopyPropagationOptimizer>(module, va,
		OptimCallInfoObtainer::create());

	// Check that the output hasn't been changed.
	ASSERT_TRUE(testFunc->getBody()) <<
		"expected a non-empty body";
	EXPECT_EQ(assignG1, testFunc->getBody()) <<
		"expected `" << assignG1 << "`, "
		"got `" << testFunc->getBody() << "`";
}

TEST_F(CopyPropagationOptimizerTests,
DoNotEliminateAssignToGlobalVarIfItIsUsedInTheNextStatement) {
	// Set-up the module.
	//
	// int g;
	//
	// void test() {
	//     g = 1;
	//     return g;
	// }
	//
	Variable* varG(Variable::create("g", IntType::create(32)));
	module->addGlobalVar(varG);
	ReturnStmt* returnG(ReturnStmt::create(varG));
	AssignStmt* assignG1(AssignStmt::create(varG, ConstInt::create(1, 32), returnG));
	testFunc->setBody(assignG1);

	INSTANTIATE_ALIAS_ANALYSIS_AND_VALUE_ANALYSIS(module);

	// Optimize the module.
	Optimizer::optimize<CopyPropagationOptimizer>(module, va,
		OptimCallInfoObtainer::create());

	// Check that the output hasn't been changed.
	Statement* stmt1(testFunc->getBody());
	ASSERT_TRUE(stmt1) <<
		"expected `" << assignG1 << "`, "
		"got the null pointer";
	EXPECT_EQ(assignG1, stmt1) <<
		"expected `" << assignG1 << "`, "
		"got `" << stmt1 << "`";
	Statement* stmt2(stmt1->getSuccessor());
	ASSERT_TRUE(stmt2) <<
		"expected `" << returnG << "`, "
		"got the null pointer";
	EXPECT_EQ(returnG, stmt2) <<
		"expected `" << returnG << "`, "
		"got `" << stmt2 << "`";
}

TEST_F(CopyPropagationOptimizerTests,
DoNotEliminateAssignToGlobalVarIfThereIsFuncCallBeforeTheNextAssign) {
	// Set-up the module.
	//
	// int g;
	// int h;
	//
	// void readG() {
	//     h = g;
	// }
	//
	// void test() {
	//     g = 1;
	//     readG();
	//     g = 2;
	// }
	//
	Variable* varG(Variable::create("g", IntType::create(32)));
	module->addGlobalVar(varG);
	Variable* varH(Variable::create("h", IntType::create(32)));
	module->addGlobalVar(varH);

	Function* readGFunc = FunctionBuilder("readG")
		.definitionWithBody(AssignStmt::create(varG, varH))
		.build();
	AssignStmt* assignG2(AssignStmt::create(varG, ConstInt::create(2, 32)));
	CallExpr* readGCallExpr(CallExpr::create(readGFunc->getAsVar()));
	CallStmt* readGCall(CallStmt::create(readGCallExpr, assignG2));
	AssignStmt* assignG1(AssignStmt::create(varG, ConstInt::create(1, 32), readGCall));
	testFunc->setBody(assignG1);

	INSTANTIATE_ALIAS_ANALYSIS_AND_VALUE_ANALYSIS(module);

	// Optimize the module.
	Optimizer::optimize<CopyPropagationOptimizer>(module, va,
		OptimCallInfoObtainer::create());

	// Check that the output hasn't been changed.
	Statement* stmt1(testFunc->getBody());
	ASSERT_TRUE(stmt1) <<
		"expected `" << assignG1 << "`, "
		"got the null pointer";
	EXPECT_EQ(assignG1, stmt1) <<
		"expected `" << assignG1 << "`, "
		"got `" << stmt1 << "`";
	Statement* stmt2(stmt1->getSuccessor());
	ASSERT_TRUE(stmt2) <<
		"expected `" << readGCall << "`, "
		"got the null pointer";
	EXPECT_EQ(readGCall, stmt2) <<
		"expected `" << readGCall << "`, "
		"got `" << stmt2 << "`";
	Statement* stmt3(stmt2->getSuccessor());
	ASSERT_TRUE(stmt3) <<
		"expected `" << assignG2 << "`, "
		"got the null pointer";
	EXPECT_EQ(assignG2, stmt3) <<
		"expected `" << assignG2 << "`, "
		"got `" << stmt3 << "`";
}

TEST_F(CopyPropagationOptimizerTests,
DoNotEliminateAssignToGlobalVarIfThereMayNotAlwaysBeAnotherAssignToIt) {
	// Set-up the module.
	//
	// int g;
	// int h;
	//
	// void test() {
	//     g = 1;
	//     if (h) {
	//         g = 2;
	//     }
	// }
	//
	Variable* varG(Variable::create("g", IntType::create(32)));
	module->addGlobalVar(varG);
	Variable* varH(Variable::create("h", IntType::create(32)));
	module->addGlobalVar(varH);

	AssignStmt* assignG2(AssignStmt::create(varG, ConstInt::create(2, 32)));
	IfStmt* ifStmt(IfStmt::create(varH, assignG2));
	AssignStmt* assignG1(AssignStmt::create(varG, ConstInt::create(1, 32), ifStmt));
	testFunc->setBody(assignG1);

	INSTANTIATE_ALIAS_ANALYSIS_AND_VALUE_ANALYSIS(module);

	// Optimize the module.
	Optimizer::optimize<CopyPropagationOptimizer>(module, va,
		OptimCallInfoObtainer::create());

	// Check that the output hasn't been changed.
	Statement* stmt1(testFunc->getBody());
	ASSERT_TRUE(stmt1) <<
		"expected `" << assignG1 << "`, "
		"got the null pointer";
	EXPECT_EQ(assignG1, stmt1) <<
		"expected `" << assignG1 << "`, "
		"got `" << stmt1 << "`";
	Statement* stmt2(stmt1->getSuccessor());
	ASSERT_TRUE(stmt2) <<
		"expected `" << ifStmt << "`, "
		"got the null pointer";
	EXPECT_EQ(ifStmt, stmt2) <<
		"expected `" << ifStmt << "`, "
		"got `" << stmt2 << "`";
	Statement* stmt3(cast<IfStmt>(stmt2)->getFirstIfBody());
	ASSERT_TRUE(stmt3) <<
		"expected `" << assignG2 << "`, "
		"got the null pointer";
	EXPECT_EQ(assignG2, stmt3) <<
		"expected `" << assignG2 << "`, "
		"got `" << stmt3 << "`";
}

TEST_F(CopyPropagationOptimizerTests,
DoNotEliminateAssignToGlobalVarIfItMayBeUsedIndirectly) {
	// Set-up the module.
	//
	// int g;
	//
	// void test() {
	//     int *p = &g;
	//     g = 1;
	//     return *p;
	// }
	//
	Variable* varG(Variable::create("g", IntType::create(32)));
	module->addGlobalVar(varG);
	Variable* varP(Variable::create("p", PointerType::create(IntType::create(32))));
	testFunc->addLocalVar(varP);
	ReturnStmt* returnP(ReturnStmt::create(DerefOpExpr::create(varP)));
	AssignStmt* assignG1(AssignStmt::create(varG, ConstInt::create(1, 32), returnP));
	VarDefStmt* varDefP(VarDefStmt::create(varP, AddressOpExpr::create(varG), assignG1));
	testFunc->setBody(varDefP);

	INSTANTIATE_ALIAS_ANALYSIS_AND_VALUE_ANALYSIS(module);
	VarSet refPPointsTo;
	refPPointsTo.insert(varG);
	ON_CALL(*aliasAnalysisMock, mayPointTo(varP))
		.WillByDefault(ReturnRef(refPPointsTo));
	ON_CALL(*aliasAnalysisMock, mayBePointed(varG))
		.WillByDefault(Return(true));

	// Optimize the module.
	Optimizer::optimize<CopyPropagationOptimizer>(module, va,
		OptimCallInfoObtainer::create());

	// Check that the output hasn't been changed.
	Statement* stmt1(testFunc->getBody());
	ASSERT_TRUE(stmt1) <<
		"expected `" << varDefP << "`, "
		"got the null pointer";
	EXPECT_EQ(varDefP, stmt1) <<
		"expected `" << varDefP << "`, "
		"got `" << stmt1 << "`";
	Statement* stmt2(stmt1->getSuccessor());
	ASSERT_TRUE(stmt2) <<
		"expected `" << assignG1 << "`, "
		"got the null pointer";
	EXPECT_EQ(assignG1, stmt2) <<
		"expected `" << assignG1 << "`, "
		"got `" << stmt2 << "`";
	Statement* stmt3(stmt2->getSuccessor());
	ASSERT_TRUE(stmt3) <<
		"expected `" << returnP << "`, "
		"got the null pointer";
	EXPECT_EQ(returnP, stmt3) <<
		"expected `" << returnP << "`, "
		"got `" << stmt3 << "`";
}

TEST_F(CopyPropagationOptimizerTests,
DoNotEliminateAssignToGlobalVarIfItMustBeUsedIndirectly) {
	// Set-up the module.
	//
	// int g;
	//
	// void test() {
	//     int *p = &g;
	//     g = 1;
	//     return *p;
	// }
	//
	Variable* varG(Variable::create("g", IntType::create(32)));
	module->addGlobalVar(varG);
	Variable* varP(Variable::create("p", PointerType::create(IntType::create(32))));
	testFunc->addLocalVar(varP);
	ReturnStmt* returnP(ReturnStmt::create(DerefOpExpr::create(varP)));
	AssignStmt* assignG1(AssignStmt::create(varG, ConstInt::create(1, 32), returnP));
	VarDefStmt* varDefP(VarDefStmt::create(varP, AddressOpExpr::create(varG), assignG1));
	testFunc->setBody(varDefP);

	INSTANTIATE_ALIAS_ANALYSIS_AND_VALUE_ANALYSIS(module);
	ON_CALL(*aliasAnalysisMock, pointsTo(varP))
		.WillByDefault(Return(varG));
	ON_CALL(*aliasAnalysisMock, mayBePointed(varG))
		.WillByDefault(Return(true));

	// Optimize the module.
	Optimizer::optimize<CopyPropagationOptimizer>(module, va,
		OptimCallInfoObtainer::create());

	// Check that the output hasn't been changed.
	Statement* stmt1(testFunc->getBody());
	ASSERT_TRUE(stmt1) <<
		"expected `" << varDefP << "`, "
		"got the null pointer";
	EXPECT_EQ(varDefP, stmt1) <<
		"expected `" << varDefP << "`, "
		"got `" << stmt1 << "`";
	Statement* stmt2(stmt1->getSuccessor());
	ASSERT_TRUE(stmt2) <<
		"expected `" << assignG1 << "`, "
		"got the null pointer";
	EXPECT_EQ(assignG1, stmt2) <<
		"expected `" << assignG1 << "`, "
		"got `" << stmt2 << "`";
	Statement* stmt3(stmt2->getSuccessor());
	ASSERT_TRUE(stmt3) <<
		"expected `" << returnP << "`, "
		"got the null pointer";
	EXPECT_EQ(returnP, stmt3) <<
		"expected `" << returnP << "`, "
		"got `" << stmt3 << "`";
}

TEST_F(CopyPropagationOptimizerTests,
EliminateConstantInitializerOfVarDefStmtIfNextUseIsWrite) {
	// Set-up the module.
	//
	// void test() {
	//     int a = 0;
	//     a = rand();
	//     return a + a;
	// }
	//
	Variable* varA(Variable::create("a", IntType::create(32)));
	testFunc->addLocalVar(varA);
	ReturnStmt* returnAA(ReturnStmt::create(AddOpExpr::create(varA, varA)));
	Variable* varRand(Variable::create("a", IntType::create(16)));
	CallExpr* randCall(CallExpr::create(varRand));
	AssignStmt* assignArand(AssignStmt::create(varA, randCall, returnAA));
	VarDefStmt* varDefA(VarDefStmt::create(varA, ConstInt::create(0, 32), assignArand));
	testFunc->setBody(varDefA);

	INSTANTIATE_ALIAS_ANALYSIS_AND_VALUE_ANALYSIS(module);

	// Optimize the module.
	Optimizer::optimize<CopyPropagationOptimizer>(module, va,
		OptimCallInfoObtainer::create());

	// Check that the output is correct:
	//
	// void test() {
	//     int a;        // no initializer
	//     a = rand();
	//     return a;
	// }
	//
	Statement* stmt1(testFunc->getBody());
	ASSERT_TRUE(stmt1) <<
		"expected `" << varDefA << "`, "
		"got the null pointer";
	ASSERT_TRUE(isa<VarDefStmt>(stmt1)) <<
		"expected a VarDefStmt, got `" << stmt1 << "`";
	EXPECT_EQ(varDefA, stmt1) <<
		"expected `" << varDefA << "`, "
		"got `" << stmt1 << "`";
	EXPECT_FALSE(cast<VarDefStmt>(stmt1)->getInitializer()) <<
		"expected varDefA to have no initializer, but got `" <<
		cast<VarDefStmt>(stmt1)->getInitializer() << "`";
	Statement* stmt2(stmt1->getSuccessor());
	ASSERT_TRUE(stmt2) <<
		"expected `" << assignArand << "`, "
		"got the null pointer";
	EXPECT_EQ(assignArand, stmt2) <<
		"expected `" << assignArand << "`, "
		"got `" << stmt2 << "`";
	Statement* stmt3(stmt2->getSuccessor());
	ASSERT_TRUE(stmt3) <<
		"expected `" << returnAA << "`, "
		"got the null pointer";
	EXPECT_EQ(returnAA, stmt3) <<
		"expected `" << returnAA << "`, "
		"got `" << stmt3 << "`";
}

TEST_F(CopyPropagationOptimizerTests,
DoNotEliminateInitializerOfVarDefStmtIfItIsNotConstant) {
	// Set-up the module.
	//
	// void test() {
	//     int a = rand();
	//     a = rand();
	//     return a + a;
	// }
	//
	Variable* varA(Variable::create("a", IntType::create(32)));
	testFunc->addLocalVar(varA);
	ReturnStmt* returnAA(ReturnStmt::create(AddOpExpr::create(varA, varA)));
	Variable* varRand(Variable::create("a", IntType::create(16)));
	CallExpr* randCall(CallExpr::create(varRand));
	AssignStmt* assignArand(AssignStmt::create(varA, randCall, returnAA));
	VarDefStmt* varDefA(VarDefStmt::create(varA, randCall, assignArand));
	testFunc->setBody(varDefA);

	INSTANTIATE_ALIAS_ANALYSIS_AND_VALUE_ANALYSIS(module);

	// Optimize the module.
	Optimizer::optimize<CopyPropagationOptimizer>(module, va,
		OptimCallInfoObtainer::create());

	// Check that the output hasn't been changed.
	Statement* stmt1(testFunc->getBody());
	ASSERT_TRUE(stmt1) <<
		"expected `" << varDefA << "`, "
		"got the null pointer";
	ASSERT_TRUE(isa<VarDefStmt>(stmt1)) <<
		"expected a VarDefStmt, got `" << stmt1 << "`";
	EXPECT_EQ(varDefA, stmt1) <<
		"expected `" << varDefA << "`, "
		"got `" << stmt1 << "`";
	EXPECT_TRUE(cast<VarDefStmt>(stmt1)->getInitializer()) <<
		"expected varDefA to have an initializer";
	Statement* stmt2(stmt1->getSuccessor());
	ASSERT_TRUE(stmt2) <<
		"expected `" << assignArand << "`, "
		"got the null pointer";
	EXPECT_EQ(assignArand, stmt2) <<
		"expected `" << assignArand << "`, "
		"got `" << stmt2 << "`";
	Statement* stmt3(stmt2->getSuccessor());
	ASSERT_TRUE(stmt3) <<
		"expected `" << returnAA << "`, "
		"got the null pointer";
	EXPECT_EQ(returnAA, stmt3) <<
		"expected `" << returnAA << "`, "
		"got `" << stmt3 << "`";
}

TEST_F(CopyPropagationOptimizerTests,
DoNotPropagateNullPointersToDereferencesOnLeftHandSidesOfAssignStmts) {
	// Set-up the module.
	//
	// void test() {
	//     int *p;
	//     p = NULL;
	//     *p = 1;
	// }
	//
	PointerType* intPtrType(PointerType::create(IntType::create(32)));
	Variable* varP(Variable::create("p",
		PointerType::create(IntType::create(32))));
	testFunc->addLocalVar(varP);
	DerefOpExpr* derefP(DerefOpExpr::create(varP));
	AssignStmt* assignDerefP1(AssignStmt::create(
		derefP, ConstInt::create(1, 32)));
	AssignStmt* assignPNULL(AssignStmt::create(
		varP, ConstNullPointer::create(intPtrType), assignDerefP1));
	VarDefStmt* varDefP(VarDefStmt::create(
		varP, Expression*(), assignPNULL));
	testFunc->setBody(varDefP);

	INSTANTIATE_ALIAS_ANALYSIS_AND_VALUE_ANALYSIS(module);

	// Optimize the module.
	Optimizer::optimize<CopyPropagationOptimizer>(module, va,
		OptimCallInfoObtainer::create());

	// Check that the output hasn't been changed.
	ASSERT_TRUE(testFunc->getBody()) <<
		"expected a non-empty body";
	EXPECT_EQ(varP, derefP->getOperand()) <<
		"expected `" << varP << "`, "
		"got `" << derefP->getOperand() << "`";
}

TEST_F(CopyPropagationOptimizerTests,
OptimizeIfSingleUseAfterOriginalStatementWithVarDef) {
	// Set-up the module.
	//
	// void test() {
	//     int a;
	//     a = b;
	//     return a;
	// }
	//
	Variable* varA(Variable::create("a", IntType::create(32)));
	testFunc->addLocalVar(varA);
	Variable* varB(Variable::create("b", IntType::create(32)));
	testFunc->addLocalVar(varB);
	ReturnStmt* returnA(ReturnStmt::create(varA));
	AssignStmt* assignAB(AssignStmt::create(varA, varB, returnA));
	VarDefStmt* varDefA(VarDefStmt::create(varA, Expression*(),
		assignAB));
	testFunc->setBody(varDefA);

	INSTANTIATE_ALIAS_ANALYSIS_AND_VALUE_ANALYSIS(module);

	// Optimize the module.
	Optimizer::optimize<CopyPropagationOptimizer>(module, va,
		OptimCallInfoObtainer::create());

	// Check that the output is correct.
	ReturnStmt* returnB(cast<ReturnStmt>(testFunc->getBody()));
	ASSERT_TRUE(returnB) <<
		"expected a return statement, got `" << testFunc << "`";
	EXPECT_EQ(varB, returnB->getRetVal()) <<
		"expected `" << varB << "` as the return value, "
		"got `" << returnB->getRetVal() << "`";
}

TEST_F(CopyPropagationOptimizerTests,
OptimizeIfSingleUseAfterOriginalStatementNoVarDef) {
	// Set-up the module.
	//
	// void test() {
	//     a = b;
	//     return a;
	// }
	//
	Variable* varA(Variable::create("a", IntType::create(32)));
	testFunc->addLocalVar(varA);
	Variable* varB(Variable::create("b", IntType::create(32)));
	testFunc->addLocalVar(varB);
	ReturnStmt* returnA(ReturnStmt::create(varA));
	AssignStmt* assignAB(AssignStmt::create(varA, varB, returnA));
	testFunc->setBody(assignAB);

	INSTANTIATE_ALIAS_ANALYSIS_AND_VALUE_ANALYSIS(module);

	// Optimize the module.
	Optimizer::optimize<CopyPropagationOptimizer>(module, va,
		OptimCallInfoObtainer::create());

	// Check that the output is correct.
	ReturnStmt* returnB(cast<ReturnStmt>(testFunc->getBody()));
	ASSERT_TRUE(returnB) <<
		"expected a return statement, got `" << testFunc->getBody() << "`";
	EXPECT_EQ(varB, returnB->getRetVal()) <<
		"expected `" << varB << "` as the return value, "
		"got `" << returnB->getRetVal() << "`";
}

TEST_F(CopyPropagationOptimizerTests,
OptimizeIfTwoUsesAfterOriginalStatementNoVarDef) {
	// Set-up the module.
	//
	// void test() {
	//     a = b;
	//     c = a;
	//     return a;
	// }
	//
	Variable* varA(Variable::create("a", IntType::create(32)));
	testFunc->addLocalVar(varA);
	Variable* varB(Variable::create("b", IntType::create(32)));
	testFunc->addLocalVar(varB);
	Variable* varC(Variable::create("c", IntType::create(32)));
	testFunc->addLocalVar(varC);
	ReturnStmt* returnA(ReturnStmt::create(varA));
	AssignStmt* assignCA(AssignStmt::create(varC, varA, returnA));
	AssignStmt* assignAB(AssignStmt::create(varA, varB, assignCA));
	testFunc->setBody(assignAB);

	INSTANTIATE_ALIAS_ANALYSIS_AND_VALUE_ANALYSIS(module);

	// Optimize the module.
	Optimizer::optimize<CopyPropagationOptimizer>(module, va,
		OptimCallInfoObtainer::create());

	// Check that the output is correct.
	ReturnStmt* returnB(cast<ReturnStmt>(testFunc->getBody()));
	ASSERT_TRUE(returnB) <<
		"expected a return statement, got `" << testFunc->getBody() << "`";
	EXPECT_EQ(varB, returnB->getRetVal()) <<
		"expected `" << varB << "` as the return value, "
		"got `" << returnB->getRetVal() << "`";
}

TEST_F(CopyPropagationOptimizerTests,
OptimizeIfRhsModifiedAfterTheOnlyUseOfLhsAndFuncReturnsRightAfterThat) {
	// Set-up the module.
	//
	// void test() {
	//     a = b;
	//     c = a;
	//     b = 1;
	// }
	//
	Variable* varA(Variable::create("a", IntType::create(32)));
	testFunc->addLocalVar(varA);
	Variable* varB(Variable::create("b", IntType::create(32)));
	testFunc->addLocalVar(varB);
	Variable* varC(Variable::create("c", IntType::create(32)));
	testFunc->addLocalVar(varC);
	AssignStmt* assignB1(AssignStmt::create(varB, ConstInt::create(1, 32)));
	AssignStmt* assignCA(AssignStmt::create(varC, varA, assignB1));
	AssignStmt* assignAB(AssignStmt::create(varA, varB, assignCA));
	testFunc->setBody(assignAB);

	INSTANTIATE_ALIAS_ANALYSIS_AND_VALUE_ANALYSIS(module);

	// Optimize the module.
	Optimizer::optimize<CopyPropagationOptimizer>(module, va,
		OptimCallInfoObtainer::create());

	// Check that the output is correct.
	EmptyStmt* emptyStmt(cast<EmptyStmt>(testFunc->getBody()));
	ASSERT_TRUE(emptyStmt) <<
		"expected a return statement, got `" << testFunc->getBody() << "`";
	EXPECT_EQ(nullptr, emptyStmt->getSuccessor()) <<
		"expected `nullptr`, got `" << emptyStmt->getSuccessor() << "`";
}

//==============================================================================

TEST_F(CopyPropagationOptimizerTests,
OptimizeNoAssignStmtOneUse) {
	// Add a body to the testing function:
	//
	//   a = 1  (VarDefStmt)
	//   b = a  (VarDefStmt)
	//   return b
	//
	Variable* varA(Variable::create("a", IntType::create(16)));
	Variable* varB(Variable::create("b", IntType::create(16)));
	ConstInt* constInt1(ConstInt::create(llvm::APInt(16, 1)));
	ReturnStmt* returnB(ReturnStmt::create(varB));
	VarDefStmt* varDefB(
		VarDefStmt::create(varB, varA, returnB));
	VarDefStmt* varDefA(
		VarDefStmt::create(varA, constInt1, varDefB));
	testFunc->setBody(varDefA);

	INSTANTIATE_ALIAS_ANALYSIS_AND_VALUE_ANALYSIS(module);

	// Optimize the module.
	Optimizer::optimize<CopyPropagationOptimizer>(module, va,
		OptimCallInfoObtainer::create());

	// Check that the output is correct.
	ASSERT_TRUE(testFunc->getBody()) <<
		"expected a non-empty body";
	// return 1
	ReturnStmt* outReturn(cast<ReturnStmt>(testFunc->getBody()));
	ASSERT_EQ(constInt1, outReturn->getRetVal()) <<
		"expected `" << constInt1 << "`, got `" << outReturn->getRetVal() << "`";
}

TEST_F(CopyPropagationOptimizerTests,
OptimizeNoAssignStmtOneUseEvenIfLhsVarIsExternal) {
	// Add a body to the testing function:
	//
	//   a = 1  (VarDefStmt, where 'a' is an 'external' variable comming from a
	//           volatile load/store)
	//   b = a  (VarDefStmt)
	//   return b
	//
	Variable* varA(Variable::create("a", IntType::create(16)));
	varA->markAsExternal();
	Variable* varB(Variable::create("b", IntType::create(16)));
	ConstInt* constInt1(ConstInt::create(llvm::APInt(16, 1)));
	ReturnStmt* returnB(ReturnStmt::create(varB));
	VarDefStmt* varDefB(
		VarDefStmt::create(varB, varA, returnB));
	VarDefStmt* varDefA(
		VarDefStmt::create(varA, constInt1, varDefB));
	testFunc->setBody(varDefA);

	INSTANTIATE_ALIAS_ANALYSIS_AND_VALUE_ANALYSIS(module);

	// Optimize the module.
	Optimizer::optimize<CopyPropagationOptimizer>(module, va,
		OptimCallInfoObtainer::create());

	// Check that the output is correct.
	ASSERT_TRUE(testFunc->getBody()) <<
		"expected a non-empty body";
	// a = 1
	VarDefStmt* outVarDefA(cast<VarDefStmt>(testFunc->getBody()));
	ASSERT_EQ(varDefA, outVarDefA) <<
		"expected `" << varDefA << "`, got `" << testFunc->getBody() << "`";
	// return a
	ASSERT_TRUE(outVarDefA->hasSuccessor());
	ReturnStmt* outReturnA(cast<ReturnStmt>(outVarDefA->getSuccessor()));
	ASSERT_TRUE(outReturnA) <<
		"expected ReturnStmt, got `" << outVarDefA->getSuccessor() << "`";
	ASSERT_TRUE(outReturnA->getRetVal()) <<
		"expected a return value, got no return value";
	ASSERT_EQ(varA, outReturnA->getRetVal()) <<
		"expected `" << varA->getName() << "`, got `" << outReturnA->getRetVal() << "`";
}

TEST_F(CopyPropagationOptimizerTests,
OptimizeAssignStmtsOneUse) {
	// Add a body to the testing function:
	//
	//   a      (VarDefStmt)
	//   b      (VarDefStmt)
	//   a = 1  (AssignStmt)
	//   b = a  (AssignStmt)
	//   return b
	//
	Variable* varA(Variable::create("a", IntType::create(16)));
	Variable* varB(Variable::create("b", IntType::create(16)));
	ConstInt* constInt1(ConstInt::create(llvm::APInt(16, 1)));
	ReturnStmt* returnB(ReturnStmt::create(varB));
	AssignStmt* assignBA(AssignStmt::create(varB, varA, returnB));
	AssignStmt* assignA1(AssignStmt::create(varA, constInt1, assignBA));
	VarDefStmt* varDefB(VarDefStmt::create(varB, Expression*(), assignA1));
	VarDefStmt* varDefA(VarDefStmt::create(varA, Expression*(), varDefB));
	testFunc->setBody(varDefA);

	INSTANTIATE_ALIAS_ANALYSIS_AND_VALUE_ANALYSIS(module);

	// Optimize the module.
	Optimizer::optimize<CopyPropagationOptimizer>(module, va,
		OptimCallInfoObtainer::create());

	// Check that the output is correct.
	ASSERT_TRUE(testFunc->getBody()) <<
		"expected a non-empty body";
	// return 1
	ReturnStmt* outReturn(cast<ReturnStmt>(testFunc->getBody()));
	ASSERT_EQ(constInt1, outReturn->getRetVal()) <<
		"expected `" << constInt1 << "`, got `" << outReturn->getRetVal() << "`";
}

TEST_F(CopyPropagationOptimizerTests,
OptimizeWhenOriginalValueIsUsedAfter) {
	// Add a body to the testing function:
	//
	//   a = 1  (VarDefStmt)
	//   b = a  (VarDefStmt)
	//   return a
	//
	Variable* varA(Variable::create("a", IntType::create(16)));
	Variable* varB(Variable::create("b", IntType::create(16)));
	ConstInt* constInt1(ConstInt::create(llvm::APInt(16, 1)));
	ReturnStmt* returnA(ReturnStmt::create(varA));
	VarDefStmt* varDefB(
		VarDefStmt::create(varB, varA, returnA));
	VarDefStmt* varDefA(
		VarDefStmt::create(varA, constInt1, varDefB));
	testFunc->setBody(varDefA);

	INSTANTIATE_ALIAS_ANALYSIS_AND_VALUE_ANALYSIS(module);

	// Optimize the module.
	Optimizer::optimize<CopyPropagationOptimizer>(module, va,
		OptimCallInfoObtainer::create());

	// Check that the output is correct.
	ASSERT_TRUE(testFunc->getBody()) <<
		"expected a non-empty body";
	// return 1
	ReturnStmt* outReturn(cast<ReturnStmt>(testFunc->getBody()));
	ASSERT_EQ(constInt1, outReturn->getRetVal()) <<
		"expected `" << constInt1 << "`, got `" << outReturn->getRetVal() << "`";
}

TEST_F(CopyPropagationOptimizerTests,
OptimizeWhenRhsIsComplexExpression) {
	// Add a body to the testing function:
	//
	//   a = 1      (VarDefStmt)
	//   b = a + 3  (VarDefStmt)
	//   return b
	//
	Variable* varA(Variable::create("a", IntType::create(16)));
	Variable* varB(Variable::create("b", IntType::create(16)));
	ConstInt* constInt1(ConstInt::create(llvm::APInt(16, 1)));
	ConstInt* constInt3(ConstInt::create(llvm::APInt(16, 3)));
	ReturnStmt* returnB(ReturnStmt::create(varB));
	VarDefStmt* varDefB(
		VarDefStmt::create(varB, AddOpExpr::create(varA, constInt3), returnB));
	VarDefStmt* varDefA(
		VarDefStmt::create(varA, constInt1, varDefB));
	testFunc->setBody(varDefA);

	INSTANTIATE_ALIAS_ANALYSIS_AND_VALUE_ANALYSIS(module);

	// Optimize the module.
	Optimizer::optimize<CopyPropagationOptimizer>(module, va,
		OptimCallInfoObtainer::create());

	// Check that the output is correct.
	ASSERT_TRUE(testFunc->getBody()) <<
		"expected a non-empty body";
	// return 1 + 3
	ReturnStmt* outReturn(cast<ReturnStmt>(testFunc->getBody()));
	ASSERT_TRUE(outReturn) <<
		"expected `return`, "
		"got the null pointer";
	AddOpExpr* expr(cast<AddOpExpr>(outReturn->getRetVal()));
	ASSERT_TRUE(expr) <<
		"expected `add expr`, "
		"got the null pointer";
	ASSERT_EQ(constInt1, expr->getFirstOperand()) <<
		"expected `" << constInt1 << "`, got `" << expr->getFirstOperand() << "`";
	ASSERT_EQ(constInt3, expr->getSecondOperand()) <<
		"expected `" << constInt3 << "`, got `" << expr->getSecondOperand() << "`";
}

TEST_F(CopyPropagationOptimizerTests,
OptimizeWhenLhsIsGlobalVariable) {
	// Add a body to the testing function:
	//
	//   global b
	//
	//   a = 1  (VarDefStmt)
	//   b = a  (AssignStmt)
	//   return b
	//
	Variable* varA(Variable::create("a", IntType::create(16)));
	Variable* varB(Variable::create("b", IntType::create(16)));
	module->addGlobalVar(varB);
	ConstInt* constInt1(ConstInt::create(llvm::APInt(16, 1)));
	ReturnStmt* returnB(ReturnStmt::create(varB));
	AssignStmt* assignBA(
		AssignStmt::create(varB, varA, returnB));
	VarDefStmt* varDefA(
		VarDefStmt::create(varA, constInt1, assignBA));
	testFunc->setBody(varDefA);

	INSTANTIATE_ALIAS_ANALYSIS_AND_VALUE_ANALYSIS(module);

	// Optimize the module.
	Optimizer::optimize<CopyPropagationOptimizer>(module, va,
		OptimCallInfoObtainer::create());

	// Check that the output is correct.
	// b = 1
	Statement* stmt1(testFunc->getBody());
	ASSERT_EQ(assignBA, stmt1) <<
		"expected `" << assignBA << "`, got `" << stmt1 << "`";
	auto rhs = cast<AssignStmt>(stmt1)->getRhs();
	ASSERT_EQ(constInt1, rhs) <<
		"expected `" << constInt1 << "`, got `" << rhs << "`";
	// return b
	Statement* stmt2(stmt1->getSuccessor());
	ASSERT_EQ(returnB, stmt2) <<
		"expected `" << returnB << "`, got `" << stmt2 << "`";
}

TEST_F(CopyPropagationOptimizerTests,
OptimizeWhenRhsIsGlobalVariable) {
	// Add a body to the testing function:
	//
	//   global a
	//
	//   a = 1  (AssignStmt)
	//   b = a  (VarDefStmt)
	//   return b
	//
	Variable* varA(Variable::create("a", IntType::create(16)));
	module->addGlobalVar(varA);
	Variable* varB(Variable::create("b", IntType::create(16)));
	ConstInt* constInt1(ConstInt::create(llvm::APInt(16, 1)));
	ReturnStmt* returnB(ReturnStmt::create(varB));
	VarDefStmt* varDefB(
		VarDefStmt::create(varB, varA, returnB));
	AssignStmt* assignA1(
		AssignStmt::create(varA, constInt1, varDefB));
	testFunc->setBody(assignA1);

	INSTANTIATE_ALIAS_ANALYSIS_AND_VALUE_ANALYSIS(module);

	// Optimize the module.
	Optimizer::optimize<CopyPropagationOptimizer>(module, va,
		OptimCallInfoObtainer::create());

	// Check that the output is correct.
	// a = 1
	Statement* stmt1(testFunc->getBody());
	ASSERT_EQ(assignA1, stmt1) <<
		"expected `" << assignA1 << "`, got `" << stmt1 << "`";
	// return a
	ReturnStmt* outReturn(cast<ReturnStmt>(stmt1->getSuccessor()));
	ASSERT_TRUE(outReturn) <<
		"expected `return`, "
		"got the null pointer";
	ASSERT_EQ(varA, outReturn->getRetVal()) <<
		"expected `" << varA << "`, got `" << outReturn->getRetVal() << "`";
}

TEST_F(CopyPropagationOptimizerTests,
DoNotOptimizeWhenAuxiliaryVariableIsExternal) {
	// Add a body to the testing function:
	//
	//   a = 1  (VarDefStmt)
	//   b = a  (VarDefStmt, where 'b' is an 'external' variable comming from a
	//           volatile load/store)
	//   return b
	//
	Variable* varA(Variable::create("a", IntType::create(16)));
	Variable* varB(Variable::create("b", IntType::create(16)));
	varB->markAsExternal();
	ConstInt* constInt1(ConstInt::create(llvm::APInt(16, 1)));
	ReturnStmt* returnB(ReturnStmt::create(varB));
	VarDefStmt* varDefB(
		VarDefStmt::create(varB, varA, returnB));
	VarDefStmt* varDefA(
		VarDefStmt::create(varA, constInt1, varDefB));
	testFunc->setBody(varDefA);

	INSTANTIATE_ALIAS_ANALYSIS_AND_VALUE_ANALYSIS(module);

	// Optimize the module.
	Optimizer::optimize<CopyPropagationOptimizer>(module, va,
		OptimCallInfoObtainer::create());

	// Check that the output is correct.
	// b = 1
	Statement* stmt1(testFunc->getBody());
	ASSERT_EQ(varDefB, stmt1) <<
		"expected `" << varDefB << "`, got `" << stmt1 << "`";
	auto rhs = cast<VarDefStmt>(stmt1)->getInitializer();
	ASSERT_EQ(constInt1, rhs) <<
		"expected `" << constInt1 << "`, got `" << rhs << "`";
	// return b
	Statement* stmt2(stmt1->getSuccessor());
	ASSERT_EQ(returnB, stmt2) <<
		"expected `" << returnB << "`, got `" << stmt2 << "`";
}

} // namespace tests
} // namespace llvmir2hll
} // namespace retdec
