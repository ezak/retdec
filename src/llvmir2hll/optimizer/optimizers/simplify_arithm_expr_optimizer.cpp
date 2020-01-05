/**
* @file src/llvmir2hll/optimizer/optimizers/simplify_arithm_expr_optimizer.cpp
* @brief Implementation of SimplifyArithmExprOptimizer.
* @copyright (c) 2017 Avast Software, licensed under the MIT license
*/

#include "retdec/llvmir2hll/ir/add_op_expr.h"
#include "retdec/llvmir2hll/ir/bit_and_op_expr.h"
#include "retdec/llvmir2hll/ir/bit_or_op_expr.h"
#include "retdec/llvmir2hll/ir/bit_xor_op_expr.h"
#include "retdec/llvmir2hll/ir/constant.h"
#include "retdec/llvmir2hll/ir/div_op_expr.h"
#include "retdec/llvmir2hll/ir/eq_op_expr.h"
#include "retdec/llvmir2hll/ir/function.h"
#include "retdec/llvmir2hll/ir/global_var_def.h"
#include "retdec/llvmir2hll/ir/gt_eq_op_expr.h"
#include "retdec/llvmir2hll/ir/gt_op_expr.h"
#include "retdec/llvmir2hll/ir/if_stmt.h"
#include "retdec/llvmir2hll/ir/lt_eq_op_expr.h"
#include "retdec/llvmir2hll/ir/lt_op_expr.h"
#include "retdec/llvmir2hll/ir/mod_op_expr.h"
#include "retdec/llvmir2hll/ir/module.h"
#include "retdec/llvmir2hll/ir/mul_op_expr.h"
#include "retdec/llvmir2hll/ir/neq_op_expr.h"
#include "retdec/llvmir2hll/ir/not_op_expr.h"
#include "retdec/llvmir2hll/ir/or_op_expr.h"
#include "retdec/llvmir2hll/ir/sub_op_expr.h"
#include "retdec/llvmir2hll/ir/ternary_op_expr.h"
#include "retdec/llvmir2hll/optimizer/optimizers/simplify_arithm_expr_optimizer.h"
#include "retdec/llvmir2hll/support/debug.h"
#include "retdec/llvmir2hll/support/types.h"

namespace retdec {
namespace llvmir2hll {

/**
* @brief Constructs a new optimizer. Also create all sub-optimizers which are
*        using from this optimization.
*
* @param[in] module Module to be optimized.
* @param[in] arithmExprEvaluator @a The used evaluator of arithmetical
*            expressions.
*
* @par Preconditions
*  - @a module is non-null
*  - @a arithmExprEvaluator is non-null
*/
SimplifyArithmExprOptimizer::SimplifyArithmExprOptimizer(Module* module,
		ArithmExprEvaluator* arithmExprEvaluator):
			Optimizer(module) {
	PRECONDITION_NON_NULL(module);
	PRECONDITION_NON_NULL(arithmExprEvaluator);

	createSubOptimizers(arithmExprEvaluator);
}

void SimplifyArithmExprOptimizer::doOptimization() {
	// Visit the initializer of all global variables.
	for (auto i = module->global_var_begin(), e = module->global_var_end();
			i != e; ++i) {
		// Keep optimizing until there are no changes.
		do {
			codeChanged = false;
			if (Expression* init = (*i)->getInitializer()) {
				init->accept(this);
			}
		} while (codeChanged);
	}

	// Visit all functions.
	for (auto i = module->func_definition_begin(),
			e = module->func_definition_end(); i != e; ++i) {
		// Keep optimizing until there are no changes.
		do {
			codeChanged = false;
			restart();
			(*i)->accept(this);
		} while (codeChanged);
	}
}

void SimplifyArithmExprOptimizer::visit(AddOpExpr* expr) {
	tryOptimizeInSubOptimizations(expr);
}

void SimplifyArithmExprOptimizer::visit(SubOpExpr* expr) {
	tryOptimizeInSubOptimizations(expr);
}

void SimplifyArithmExprOptimizer::visit(MulOpExpr* expr) {
	tryOptimizeInSubOptimizations(expr);
}

void SimplifyArithmExprOptimizer::visit(DivOpExpr* expr) {
	tryOptimizeInSubOptimizations(expr);
}

void SimplifyArithmExprOptimizer::visit(ModOpExpr* expr) {
	tryOptimizeInSubOptimizations(expr);
}

void SimplifyArithmExprOptimizer::visit(BitAndOpExpr* expr) {
	tryOptimizeInSubOptimizations(expr);
}

void SimplifyArithmExprOptimizer::visit(BitOrOpExpr* expr) {
	tryOptimizeInSubOptimizations(expr);
}

void SimplifyArithmExprOptimizer::visit(BitXorOpExpr* expr) {
	tryOptimizeInSubOptimizations(expr);
}

void SimplifyArithmExprOptimizer::visit(LtOpExpr* expr) {
	tryOptimizeInSubOptimizations(expr);
}

void SimplifyArithmExprOptimizer::visit(LtEqOpExpr* expr) {
	tryOptimizeInSubOptimizations(expr);
}

void SimplifyArithmExprOptimizer::visit(GtOpExpr* expr) {
	tryOptimizeInSubOptimizations(expr);
}

void SimplifyArithmExprOptimizer::visit(GtEqOpExpr* expr) {
	tryOptimizeInSubOptimizations(expr);
}

void SimplifyArithmExprOptimizer::visit(EqOpExpr* expr) {
	tryOptimizeInSubOptimizations(expr);
}

void SimplifyArithmExprOptimizer::visit(NeqOpExpr* expr) {
	tryOptimizeInSubOptimizations(expr);
}

void SimplifyArithmExprOptimizer::visit(NotOpExpr* expr) {
	tryOptimizeInSubOptimizations(expr);
}

void SimplifyArithmExprOptimizer::visit(OrOpExpr* expr) {
	tryOptimizeInSubOptimizations(expr);
}

void SimplifyArithmExprOptimizer::visit(TernaryOpExpr* expr) {
	tryOptimizeInSubOptimizations(expr);
}

/**
* @brief Iterate through all sub-optimizers and try optimize @a expr.
*
* If something was optimized in sub-optimizations, @c codeChanged is set to
* @c true.
*
* @param[in] expr An expression to optimize.
*/
void SimplifyArithmExprOptimizer::tryOptimizeInSubOptimizations(
		Expression* expr) {
	for (const auto &subOptim : subOptims) {
		codeChanged |= subOptim->tryOptimize(expr);
	}
}

/**
* @brief Create all sub-optimizers and save it into @c subOptims.
*
* @param[in] arithmExprEvaluator @a The used evaluator of arithmetical
*            expressions.
*/
void SimplifyArithmExprOptimizer::createSubOptimizers(ArithmExprEvaluator*
		arithmExprEvaluator) {
	StringVector regObjects = SubOptimizerFactory::getInstance().
		getRegisteredObjects();
	for (const auto &regObject : regObjects) {
		subOptims.push_back(
			SubOptimizerFactory::getInstance().createObject(
				regObject,
				arithmExprEvaluator
			)
		);
	}
}

} // namespace llvmir2hll
} // namespace retdec
