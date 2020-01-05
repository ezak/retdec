/**
* @file tests/llvmir2hll/semantics/semantics/win_api_semantics_tests.cpp
* @brief Tests for the @c win_api_semantics module.
* @copyright (c) 2017 Avast Software, licensed under the MIT license
*/

#include <gtest/gtest.h>

#include "retdec/llvmir2hll/semantics/semantics/win_api_semantics.h"

using namespace ::testing;

namespace retdec {
namespace llvmir2hll {
namespace tests {

/**
* @brief Tests for the @c win_api_semantics module.
*/
class WinAPISemanticsTests: public Test {
protected:
	Semantics* semantics;

protected:
	virtual void SetUp() override {
		semantics = WinAPISemantics::create();
	}
};

TEST_F(WinAPISemanticsTests,
SemanticsHasNonEmptyID) {
	EXPECT_TRUE(!semantics->getId().empty()) <<
		"the semantics should have a non-empty ID";
}

//
// getCHeaderFileForFunc()
//

TEST_F(WinAPISemanticsTests,
GetCHeaderFileForKnownFunctionsReturnsCorrectAnswer) {
	// ShellAboutA
	std::optional<std::string> headerForShellAboutA(semantics->getCHeaderFileForFunc("ShellAboutA"));
	ASSERT_TRUE(headerForShellAboutA) << "no header file for `print`";
	EXPECT_EQ("windows.h", headerForShellAboutA.value());

	// wsprintfA
	std::optional<std::string> headerForWsprintfA(semantics->getCHeaderFileForFunc("wsprintfA"));
	ASSERT_TRUE(headerForWsprintfA) << "no header file for `wsprintfA`";
	EXPECT_EQ("windows.h", headerForWsprintfA.value());
}

TEST_F(WinAPISemanticsTests,
GetCHeaderFileForUnknownFunctionsReturnsNoAnswer) {
	// foo
	std::optional<std::string> headerForFoo(semantics->getCHeaderFileForFunc("foo"));
	EXPECT_FALSE(headerForFoo);
}

//
// funcNeverReturns()
//

TEST_F(WinAPISemanticsTests,
FuncNeverReturnsForKnownFunctionThatNeverReturnsReturnsTrue) {
	// ExitProcess
	std::optional<bool> exitProcessNeverReturns(semantics->funcNeverReturns("ExitProcess"));
	ASSERT_TRUE(exitProcessNeverReturns) << "no information for `ExitProcess`";
	EXPECT_TRUE(exitProcessNeverReturns.value());

	// ExitThread
	std::optional<bool> exitThreadNeverReturns(semantics->funcNeverReturns("ExitThread"));
	ASSERT_TRUE(exitThreadNeverReturns) << "no information for `ExitThread`";
	EXPECT_TRUE(exitThreadNeverReturns.value());
}

TEST_F(WinAPISemanticsTests,
FuncNeverReturnsForUnknownFunctionsReturnsNoAnswer) {
	// foo
	std::optional<bool> fooNeverReturns(semantics->funcNeverReturns("foo"));
	ASSERT_FALSE(fooNeverReturns) << "there should be no information for `foo`";
}

//
// getNameOfVarStoringResult()
//

TEST_F(WinAPISemanticsTests,
GetNameOfVarStoringResultForKnownFunctionsReturnsCorrectAnswer) {
	// IsValidCodePage
	std::optional<std::string> IsValidCodePageVarName(semantics->getNameOfVarStoringResult("IsValidCodePage"));
	ASSERT_TRUE(IsValidCodePageVarName) << "no name of the variable storing the result of `IsValidCodePage`";
	EXPECT_EQ("validCodePage", IsValidCodePageVarName.value());

	// CreateFile
	std::optional<std::string> CreateFileVarName(semantics->getNameOfVarStoringResult("CreateFile"));
	ASSERT_TRUE(CreateFileVarName) << "no name of the variable storing the result of `CreateFile`";
	EXPECT_EQ("fileHandle", CreateFileVarName.value());
}

TEST_F(WinAPISemanticsTests,
GetNameOfVarStoringResultForUnknownFunctionsReturnsNoAnswer) {
	// foo
	std::optional<std::string> fooVarName(semantics->getNameOfVarStoringResult("foo"));
	EXPECT_FALSE(fooVarName);
}

//
// getNameOfParam()
//

TEST_F(WinAPISemanticsTests,
GetNameOfParamForKnownFunctionsReturnsCorrectAnswer) {
	// OpenFile (first parameter)
	std::optional<std::string> openFileParam1Name(semantics->getNameOfParam("OpenFile", 1));
	ASSERT_TRUE(openFileParam1Name) << "no name of the first parameter of `OpenFile`";
	EXPECT_EQ("lpFileName", openFileParam1Name.value());

	// openFile (second parameter)
	std::optional<std::string> openFileParam2Name(semantics->getNameOfParam("OpenFile", 2));
	ASSERT_TRUE(openFileParam2Name) << "no name of the second parameter of `OpenFile`";
	EXPECT_EQ("lpReOpenBuff", openFileParam2Name.value());

	// openFile (third parameter)
	std::optional<std::string> openFileParam3Name(semantics->getNameOfParam("OpenFile", 3));
	ASSERT_TRUE(openFileParam3Name) << "no name of the third parameter of `OpenFile`";
	EXPECT_EQ("uStyle", openFileParam3Name.value());
}

TEST_F(WinAPISemanticsTests,
GetNameOfParamForUnknownFunctionsReturnsNoAnswer) {
	// foo
	std::optional<std::string> fooParam1Name(semantics->getNameOfParam("foo", 1));
	EXPECT_FALSE(fooParam1Name) << "there should be no information for `foo`";
}

//
// getSymbolicNamesForParam()
//

TEST_F(WinAPISemanticsTests,
GetSymbolicNamesForParamForKnownFunctionsReturnsCorrectAnswer) {
	// regOpenKey
	std::optional<IntStringMap> regOpenKeySymbolicNames(semantics->getSymbolicNamesForParam("RegOpenKey", 1));
	ASSERT_TRUE(regOpenKeySymbolicNames) << "no information for `RegOpenKey`";

	IntStringMap refMap;
	refMap[-2147483647 - 1] = "HKEY_CLASSES_ROOT";
	refMap[-2147483647] = "HKEY_CURRENT_USER";
	refMap[-2147483646] = "HKEY_LOCAL_MACHINE";
	refMap[-2147483645] = "HKEY_USRS";
	refMap[-2147483644] = "HKEY_PERFORMANCE_DATA";
	refMap[-2147483643] = "HKEY_CURRENT_CONFIG";
	refMap[-2147483642] = "HKEY_DYN_DATA";
	EXPECT_EQ(refMap, regOpenKeySymbolicNames.value());
}

TEST_F(WinAPISemanticsTests,
GetSymbolicNamesForParamForUnknownFunctionsReturnsNoAnswer) {
	// foo
	std::optional<IntStringMap> fooSymbolicNames(semantics->getSymbolicNamesForParam("foo", 1));
	EXPECT_FALSE(fooSymbolicNames);
}

} // namespace tests
} // namespace llvmir2hll
} // namespace retdec
