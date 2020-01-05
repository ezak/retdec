/**
* @file tests/llvmir2hll/obtainer/call_info_obtainer_mock.h
* @brief Mocks for the CallInfoObtainer module.
* @copyright (c) 2017 Avast Software, licensed under the MIT license
*/

#ifndef BACKEND_BIR_OBTAINER_TESTS_CALL_INFO_OBTAINER_MOCK_H
#define BACKEND_BIR_OBTAINER_TESTS_CALL_INFO_OBTAINER_MOCK_H

#include <gmock/gmock.h>

#include "retdec/llvmir2hll/obtainer/call_info_obtainer.h"
#include "retdec/llvmir2hll/support/smart_ptr.h"

namespace retdec {
namespace llvmir2hll {

class CallExpr;
class Function;

namespace tests {

/**
* @brief A mock for the CallInfo class.
*/
class CallInfoMock: public CallInfo {
public:
	explicit CallInfoMock(CallExpr* callExpr): CallInfo(callExpr) {}

	MOCK_CONST_METHOD1(isNeverRead, bool (Variable*));
	MOCK_CONST_METHOD1(mayBeRead, bool (Variable*));
	MOCK_CONST_METHOD1(isAlwaysRead, bool (Variable*));
	MOCK_CONST_METHOD1(isNeverModified, bool (Variable*));
	MOCK_CONST_METHOD1(mayBeModified, bool (Variable*));
	MOCK_CONST_METHOD1(isAlwaysModified, bool (Variable*));
	MOCK_CONST_METHOD1(valueIsNeverChanged, bool (Variable*));
	MOCK_CONST_METHOD1(isAlwaysModifiedBeforeRead, bool (Variable*));
};

/**
* @brief A mock for the FuncInfo class.
*/
class FuncInfoMock: public FuncInfo {
public:
	explicit FuncInfoMock(Function* func): FuncInfo(func) {}

	MOCK_CONST_METHOD1(isNeverRead, bool (Variable*));
	MOCK_CONST_METHOD1(mayBeRead, bool (Variable*));
	MOCK_CONST_METHOD1(isAlwaysRead, bool (Variable*));
	MOCK_CONST_METHOD1(isNeverModified, bool (Variable*));
	MOCK_CONST_METHOD1(mayBeModified, bool (Variable*));
	MOCK_CONST_METHOD1(isAlwaysModified, bool (Variable*));
	MOCK_CONST_METHOD1(valueIsNeverChanged, bool (Variable*));
	MOCK_CONST_METHOD1(isAlwaysModifiedBeforeRead, bool (Variable*));
};

/**
* @brief A mock for the CallInfoObtainer class.
*/
class CallInfoObtainerMock: public CallInfoObtainer {
public:
	MOCK_METHOD2(init, void (CG*, ValueAnalysis*));
	MOCK_CONST_METHOD0(isInitialized, bool ());
	MOCK_CONST_METHOD0(getId, std::string ());
	MOCK_METHOD2(getCallInfo, CallInfo* (CallExpr*, Function*));
	MOCK_METHOD1(getFuncInfo, FuncInfo* (Function*));
};

} // namespace tests
} // namespace llvmir2hll
} // namespace retdec

/**
* @brief Instantiates CallInfoObtainerMock.
*
* More specifically, this macro does the following:
*  (1) instantiates @c cioMock, which is of type
*      @code
*      testing::NiceMock<CallInfoObtainerMock> *
*      @endcode
*  (2) instantiates @c cio, which is of type
*      @code
*      CallInfoObtainer*
*      @endcode
*      and delegates to the mock from (1);
*  (3) sets some default actions for the mock from (1).
*
* Example of usage:
* @code
* INSTANTIATE_CALL_INFO_OBTAINER_MOCK();
* SomeClassRequiringCIO* obj(new SomeClassRequiringCIO(cio));
* @endcode
*/
#define INSTANTIATE_CALL_INFO_OBTAINER_MOCK() \
	::testing::NiceMock<CallInfoObtainerMock> *cioMock = \
		new ::testing::NiceMock<CallInfoObtainerMock>(); \
	CallInfoObtainer* cio(cioMock); \
	ON_CALL(*cioMock, init(::testing::_, ::testing::_)) \
		.WillByDefault(::testing::Return()); \
	ON_CALL(*cioMock, isInitialized()) \
		.WillByDefault(::testing::Return(true)); \
	ON_CALL(*cioMock, getId()) \
		.WillByDefault(::testing::Return(std::string("CallInfoObtainerMock")));

#endif
