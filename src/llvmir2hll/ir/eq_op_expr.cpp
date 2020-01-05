/**
* @file src/llvmir2hll/ir/eq_op_expr.cpp
* @brief Implementation of EqOpExpr.
* @copyright (c) 2017 Avast Software, licensed under the MIT license
*/

#include "retdec/llvmir2hll/ir/eq_op_expr.h"
#include "retdec/llvmir2hll/ir/int_type.h"
#include "retdec/llvmir2hll/support/debug.h"
#include "retdec/llvmir2hll/support/visitor.h"

namespace retdec {
namespace llvmir2hll {

/**
* @brief Constructs an equality operator.
*
* See create() for more information.
*/
EqOpExpr::EqOpExpr(Expression* op1, Expression* op2):
	BinaryOpExpr(op1, op2) {}

bool EqOpExpr::isEqualTo(Value* otherValue) const {
	if (EqOpExpr* otherValueEqOpExpr = cast<EqOpExpr>(otherValue)) {
		return op1->isEqualTo(otherValueEqOpExpr->getFirstOperand()) &&
			op2->isEqualTo(otherValueEqOpExpr->getSecondOperand());
	}
	return false;
}

Value* EqOpExpr::clone() {
	EqOpExpr* eqOpExpr(EqOpExpr::create(
		ucast<Expression>(op1->clone()), ucast<Expression>(op2->clone())));
	eqOpExpr->setMetadata(getMetadata());
	return eqOpExpr;
}

Type* EqOpExpr::getType() const {
	// The type of `x == y` should be bool.
	return IntType::create(1);
}

/**
* @brief Creates a new equality operator.
*
* @param[in] op1 First operand.
* @param[in] op2 Second operand.
*
* @par Preconditions
*  - both operands are non-null
*/
EqOpExpr* EqOpExpr::create(Expression* op1, Expression* op2) {
	PRECONDITION_NON_NULL(op1);
	PRECONDITION_NON_NULL(op2);

	EqOpExpr* expr(new EqOpExpr(op1, op2));

	// Initialization (recall that this cannot be called in a
	// constructor).
	op1->addObserver(expr);
	op2->addObserver(expr);

	return expr;
}

void EqOpExpr::accept(Visitor *v) {
	v->visit(ucast<EqOpExpr>(this));
}

} // namespace llvmir2hll
} // namespace retdec
