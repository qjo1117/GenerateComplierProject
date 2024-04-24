#pragma once

#include <memory>
#include <vector>
#include <list>
#include <any>
#include <tuple>
#include <map>
#include <string>
#include "Token.h"
#include "Node.h"

class Object;
class Code;

struct StackFrame
{
public:
	std::vector<std::any> m_vecVariable;
	std::vector<std::any> m_vecOperandStack;
	std::size_t m_instructionPointer = 0;
};

class Machine
{
private:
	Machine() { }
public:
	__forceinline static Machine& GetInstance()
	{
		static Machine instance;
		return instance;
	}

public:
	void Execute(std::tuple<std::vector<Code>, std::map<std::string, std::size_t>> _objectCode);

private:
	void PushOperand(std::any _value);
	std::any PeekOperand();
	std::any PopOperand();
	void CollectGarbage();
	void MarkObject(std::any _object);
	void SweepObject();

private:
	using GenerateFunction = std::function<std::any(std::vector<std::any>)>;

	std::list<std::shared_ptr<Object>> m_vecObject;
	std::map<std::string, std::any> m_mapGlobal;
	std::vector<StackFrame> m_vecCallStack;
	std::map<std::string, GenerateFunction> m_mapBuiltinFunctionTable;
};

