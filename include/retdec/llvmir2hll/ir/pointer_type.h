/**
* @file include/retdec/llvmir2hll/ir/pointer_type.h
* @brief A representation of a pointer type.
* @copyright (c) 2017 Avast Software, licensed under the MIT license
*/

#ifndef RETDEC_LLVMIR2HLL_IR_POINTER_TYPE_H
#define RETDEC_LLVMIR2HLL_IR_POINTER_TYPE_H

#include "retdec/llvmir2hll/ir/type.h"
#include "retdec/llvmir2hll/support/smart_ptr.h"

namespace retdec {
namespace llvmir2hll {

class Visitor;

/**
* @brief A representation of a pointer type.
*
* Use create() to create instances. Instances of this class have reference
* object semantics. This class is not meant to be subclassed.
*/
class PointerType final: public Type {
public:
	static PointerType* create(Type* containedType);

	virtual Value* clone() override;

	virtual bool isEqualTo(Value* otherValue) const override;

	void setContainedType(Type* newContainedType);
	Type* getContainedType() const;

	/// @name Visitor Interface
	/// @{
	virtual void accept(Visitor *v) override;
	/// @}

private:
	/// Contained type.
	Type* containedType = nullptr;

private:
	// Since instances are created by calling the static function create(), the
	// constructor can be private.
	PointerType(Type* containedType);
};

} // namespace llvmir2hll
} // namespace retdec

#endif
