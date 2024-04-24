#pragma once

#include <any>
#include <map>
#include <iomanip>
#include "TypeDefine.h"

using std::any;
using std::string;
using std::ostream;

enum class Instruction {
	Exit,
	Call, Alloca, Return,
	Jump, ConditionJump,
	Print, PrintLine,

	LogicalOr, LogicalAnd,
	Add, Subtract,
	Multiply, Divide, Modulo,
	Equal, NotEqual,
	LessThan, GreaterThan,
	LessOrEqual, GreaterOrEqual,
	Absolute, ReverseSign,

	GetElement, SetElement,
	GetGlobal, SetGlobal,
	GetLocal, SetLocal,

	PushNull, PushBoolean,
	PushNumber, PushString,
	PushArray, PushMap,
	PopOperand,
};

std::string ToString(Instruction _instruction);

struct Code {
	Instruction m_instruction;
	any m_anyOperand;
};

std::string PrintCode(Code& _code);
std::ostream& operator<<(std::ostream& _os, Code& _code);
