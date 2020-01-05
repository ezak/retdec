/**
* @file src/llvmir2hll/ir/call_stmt.cpp
* @brief Implementation of CallStmt.
* @copyright (c) 2017 Avast Software, licensed under the MIT license
*/

#include "retdec/llvmir2hll/ir/call_expr.h"
#include "retdec/llvmir2hll/ir/call_stmt.h"
#include "retdec/llvmir2hll/support/debug.h"
#include "retdec/llvmir2hll/support/visitor.h"

namespace retdec {
namespace llvmir2hll {

/**
* @brief Constructs a new call statement.
*
* See create() for more information.
*/
CallStmt::CallStmt(CallExpr* call, Address a):
	Statement(a), call(call) {}

bool CallStmt::isEqualTo(Value* otherValue) const {
	// Both types and values have to be equal.
	if (CallStmt* otherCallStmt = cast<CallStmt>(otherValue)) {
		return call->isEqualTo(otherCallStmt->call);
	}
	return false;
}

Value* CallStmt::clone() {
	CallStmt* callStmt(
		CallStmt::create(ucast<CallExpr>(call->clone()), nullptr, getAddress()));
	callStmt->setMetadata(getMetadata());
	return callStmt;
}

void CallStmt::replace(Expression* oldExpr, Expression* newExpr) {
	if (oldExpr == call && isa<CallExpr>(newExpr)) {
		setCall(cast<CallExpr>(newExpr));
	} else {
		call->replace(oldExpr, newExpr);
	}
}

Expression* CallStmt::asExpression() const {
	return call;
}

/**
* @brief Returns the contained call.
*/
CallExpr* CallStmt::getCall() const {
	return call;
}

/**
* @brief Sets a new contained call.
*
* @par Preconditions
*  - @a newCall is non-null
*/
void CallStmt::setCall(CallExpr* newCall) {
	PRECONDITION_NON_NULL(newCall);

	call->removeObserver(this);
	newCall->addObserver(this);
	call = newCall;
}

/**
* @brief Constructs a new call statement.
*
* @param[in] call Call to be wrapped.
* @param[in] succ Follower of the statement in the program flow.
* @param[in] a Address.
*
* @par Preconditions
*  - @a call is non-null
*/
CallStmt* CallStmt::create(CallExpr* call, Statement* succ,
		Address a) {
	PRECONDITION_NON_NULL(call);

	CallStmt* callStmt(new CallStmt(call, a));
	callStmt->setSuccessor(succ);

	// Initialization (recall that this cannot be called in a
	// constructor).
	call->addObserver(callStmt);

	return callStmt;
}

/**
* @brief Updates the statement according to the changes of @a subject.
*
* @param[in] subject Observable object.
* @param[in] arg Optional argument.
*
* If @a subject is the contained call, this function replaces it with @a arg.
*
* This function does nothing when:
*  - @a subject does not correspond to the contained call
*  - @a arg is not an expression
*
* @par Preconditions
*  - both arguments are non-null
*
* @see Subject::update()
*/
void CallStmt::update(Value* subject, Value* arg) {
	PRECONDITION_NON_NULL(subject);
	PRECONDITION_NON_NULL(arg);

	CallExpr* newCall = cast<CallExpr>(arg);
	if (subject == call && newCall) {
		setCall(newCall);
	}
}

void CallStmt::accept(Visitor *v) {
	v->visit(ucast<CallStmt>(this));
}

} // namespace llvmir2hll
} // namespace retdec
