/**
* @file include/retdec/llvmir2hll/optimizer/optimizers/simplify_arithm_expr/sub_optimizer.h
* @brief A base class for all simplify arithmetical expression optimizations.
* @copyright (c) 2017 Avast Software, licensed under the MIT license
*/

#ifndef RETDEC_LLVMIR2HLL_OPTIMIZER_OPTIMIZERS_SIMPLIFY_ARITHM_EXPR_SUB_OPTIMIZER_H
#define RETDEC_LLVMIR2HLL_OPTIMIZER_OPTIMIZERS_SIMPLIFY_ARITHM_EXPR_SUB_OPTIMIZER_H

#include <string>

#include "retdec/llvmir2hll/evaluator/arithm_expr_evaluator.h"
#include "retdec/llvmir2hll/optimizer/optimizers/simplify_arithm_expr/sub_optimizer_factory.h"
#include "retdec/llvmir2hll/support/types.h"
#include "retdec/llvmir2hll/support/visitors/ordered_all_visitor.h"
#include "retdec/utils/non_copyable.h"

namespace retdec {
namespace llvmir2hll {

/**
* @brief A base class for all simplify arithmetical expression optimizations.
*/
class SubOptimizer: public OrderedAllVisitor, private retdec::utils::NonCopyable {
public:
	/**
	* @brief Returns the ID of the optimizer.
	*/
	virtual std::string getId() const = 0;
	virtual bool tryOptimize(Expression* expr);

protected:
	SubOptimizer(ArithmExprEvaluator* arithmExprEvaluator);

	bool isConstFloatOrConstInt(Expression* expr) const;
	void optimizeExpr(Expression* oldExpr, Expression* newExpr);
	bool tryOptimizeAndReturnIfCodeChanged(Expression* expr);

protected:
	/// The used evaluator of arithmetical expressions.
	ArithmExprEvaluator* arithmExprEvaluator = nullptr;

private:
	bool codeChanged;
};

} // namespace llvmir2hll
} // namespace retdec

#endif
