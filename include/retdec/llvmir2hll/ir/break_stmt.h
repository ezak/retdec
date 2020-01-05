/**
* @file include/retdec/llvmir2hll/ir/break_stmt.h
* @brief A break statement to exit a loop or a switch case.
* @copyright (c) 2017 Avast Software, licensed under the MIT license
*/

#ifndef RETDEC_LLVMIR2HLL_IR_BREAK_STMT_H
#define RETDEC_LLVMIR2HLL_IR_BREAK_STMT_H

#include "retdec/llvmir2hll/ir/statement.h"

namespace retdec {
namespace llvmir2hll {

class Visitor;

/**
* @brief A break statement to exit a loop or a switch case.
*
* Use create() to create instances. Instances of this class have reference
* object semantics. This class is not meant to be subclassed.
*/
class BreakStmt final: public Statement {
public:
	static BreakStmt* create(Address a = Address::Undefined);

	virtual Value* clone() override;
	virtual bool isEqualTo(Value* otherValue) const override;
	virtual bool isCompound() override { return false; }
	virtual void replace(Expression* oldExpr, Expression* newExpr) override;
	virtual Expression* asExpression() const override;

	/// @name Visitor Interface
	/// @{
	virtual void accept(Visitor *v) override;
	/// @}

private:
	// Since instances are created by calling the static function create(), the
	// constructor can be private.
	BreakStmt(Address a = Address::Undefined);
};

} // namespace llvmir2hll
} // namespace retdec

#endif
