#pragma once

#include <memory>
#include <map>
#include <vector>
#include <string>
#include <any>
#include <list>
#include <functional>
#include "TypeDefine.h"
#include "Token.h"
#include "Code.h"

class Class;
class Function;
class Program;

enum class EMemberAccess
{
	None,
	Private,
	Public,
	Protected,
	Max
};

class Interpreter
{
public:
	using ScriptFunctionType = std::function<std::any(std::vector<std::any>)>;

private:
	Interpreter();
	~Interpreter() { }
public:
	static Interpreter& GetInstance()
	{
		static Interpreter instance;
		return instance;
	}
#define InterpreterMgr	Interpreter::GetInstance()

public:
	// 인터프리터 버전
	void Interpret(std::shared_ptr<Program> _pProgram);
public:
	std::map<std::string, std::any> m_mapGlobal;
	std::list<std::list<std::map<std::string, std::any>>> m_listLocalFrame;
	std::map<std::string, std::shared_ptr<Function>> m_mapFunctionTable;
	std::map<std::string, ScriptFunctionType> m_mapBuiltinFunctionTable;
	std::map<std::string, std::vector<std::tuple<EMemberAccess, std::any>>> m_mapClassDefaultTable;
};

class Generater
{

private:
	Generater() { }
	~Generater() { }
public:
	static Generater& GetInstance()
	{
		static Generater instance;
		return instance;
	}
#define GeneraterMgr		Generater::GetInstance()

	// 코드 생성
	auto Generate(std::shared_ptr<Program> _pProgram) -> std::tuple<std::vector<Code>, std::map<std::string, std::size_t>>;

public:
	void SetLocal(std::string _strLocal);
	uint64 GetLocal(std::string _strLocal);
	void InitBlock();
	void PushBlock();
	void PopBlock();
	uint64 WriteCode(Instruction _instruction);
	uint64 WriteCode(Instruction _instruction, std::any _anyValue);
	void PatchAddress(uint64 _codeIndex);
	void PatchOperand(uint64 _codeIndex, uint64 _operand);

public:
	std::vector<Code> m_vecCodeList;
	std::map<std::string, uint64> m_mapFunctionTable;
	std::list<std::map<std::string, uint64>> m_listSymbolStackTable;
	std::vector<uint64> m_vecOffsetStack;
	uint64 m_iLocalSize = 0;
	std::vector<std::vector<uint64>> m_vecContinueStack;
	std::vector<std::vector<uint64>> m_vecBreakStack;
};

class Program 
{
public:
	std::vector<std::shared_ptr<Function>> m_vecFunction;
	std::vector<std::shared_ptr<Class>> m_vecClass;
};

class Statement 
{
public:
	virtual std::string PrintInfo(int32 _depth) = 0;
	virtual void Interpret() = 0;
	virtual void Generate() = 0;
};

class Expression 
{
public:
	virtual std::string PrintInfo(int32 _depth) = 0;
	virtual std::any Interpret() = 0;
	virtual void Generate() = 0;
};

class Function : public Statement 
{
public:
	std::string PrintInfo(int32 _depth) override;
	void Interpret() override;
	void Generate() override;

public:
	std::string m_strName;
	std::vector<std::string> m_vecParameter;
	std::vector<std::shared_ptr<Statement>> m_vecBlock;
};

class Variable : public Statement 
{
public:
	std::string PrintInfo(int32 _depth) override;
	void Interpret() override;
	void Generate() override;

public:
	std::string m_strName;
	// 책과 사양변경으로 초기화식이 없을수도 있음.
	std::shared_ptr<Expression> m_pExpression;
};

class Return : public Statement 
{
public:
	std::string PrintInfo(int32 _depth) override;
	void Interpret() override;
	void Generate() override;

public:
	std::shared_ptr<Expression> m_pExpression;
};

class For : public Statement 
{
public:
	std::string PrintInfo(int32 _depth) override;
	void Interpret() override;
	void Generate() override;

public:
	std::shared_ptr<Variable> m_pVariable;
	std::shared_ptr<Expression> m_pCondition;
	std::shared_ptr<Expression> m_pExpression;
	std::vector<std::shared_ptr<Statement>> m_vecBlock;
};

class Break : public Statement 
{
public:
	std::string PrintInfo(int32 _depth) override;
	void Interpret() override;
	void Generate() override;
};

class Continue : public Statement 
{
public:
	std::string PrintInfo(int32 _depth) override;
	void Interpret() override;
	void Generate() override;
};

class If : public Statement 
{
public:
	std::string PrintInfo(int32 _depth) override;
	void Interpret() override;
	void Generate() override;

public:
	std::vector<std::shared_ptr<Expression>> m_vecCondition;
	std::vector<std::vector<std::shared_ptr<Statement>>> m_vecBlocks;
	std::vector<std::shared_ptr<Statement>> m_vecElseBlock;
};

class Print : public Statement 
{
public:
	std::string PrintInfo(int32 _depth) override;
	void Interpret() override;
	void Generate() override;

public:	
	bool m_bLineFeed = false;
	std::vector<std::shared_ptr<Expression>> m_vecArgument;
};

class ExpressionStatement : public Statement 
{
public:
	std::string PrintInfo(int32 _depth) override;
	void Interpret() override;
	void Generate() override;

public:
	std::shared_ptr<Expression> m_pExpression;
};

class Or : public Expression
{
public:
	std::string PrintInfo(int32 _depth) override;
	std::any Interpret() override;
	void Generate() override;

public:
	std::shared_ptr<Expression> m_pLhs;
	std::shared_ptr<Expression> m_pRhs;
};

class And : public Expression
{
public:
	std::string PrintInfo(int32 _depth) override;
	std::any Interpret() override;
	void Generate() override;

public:
	std::shared_ptr<Expression> m_pLhs;
	std::shared_ptr<Expression> m_pRhs;
};

class Relational : public Expression
{
public:
	std::string PrintInfo(int32 _depth) override;
	std::any Interpret() override;
	void Generate() override;

public:
	EKind m_eKind = EKind::Unknown;
	std::shared_ptr<Expression> m_pLhs;
	std::shared_ptr<Expression> m_pRhs;
};

class Arithmetic : public Expression
{
public:
	std::string PrintInfo(int32 _depth) override;
	std::any Interpret() override;
	void Generate() override;

public:
	EKind m_eKind = EKind::Unknown;
	std::shared_ptr<Expression> m_pLhs;
	std::shared_ptr<Expression> m_pRhs;
};

class Unary : public Expression
{
public:
	std::string PrintInfo(int32 _depth) override;
	std::any Interpret() override;
	void Generate() override;

public:
	EKind m_eKind = EKind::Unknown;
	std::shared_ptr<Expression> m_pSub;
};

class Call : public Expression
{
public:
	std::string PrintInfo(int32 _depth) override;
	std::any Interpret() override;
	void Generate() override;

public:
	std::shared_ptr<Expression> m_pSub;
	std::vector<std::shared_ptr<Expression>> m_vecArgument;
};

class GetElement : public Expression
{
public:
	std::string PrintInfo(int32 _depth) override;
	std::any Interpret() override;
	void Generate() override;

public:
	std::shared_ptr<Expression> m_pSub;
	std::shared_ptr<Expression> m_pIndex;
};

class SetElement : public Expression 
{
public:
	std::string PrintInfo(int32 _depth) override;
	std::any Interpret() override;
	void Generate() override;

public:
	std::shared_ptr<Expression> m_pSub;
	std::shared_ptr<Expression> m_pIndex;
	std::shared_ptr<Expression> m_pValue;
};

class GetVariable : public Expression
{
public:
	std::string PrintInfo(int32 _depth) override;
	std::any Interpret() override;
	void Generate() override;

public:
	std::string m_strName;
};

class SetVariable : public Expression
{
public:
	std::string PrintInfo(int32 _depth) override;
	std::any Interpret() override;
	void Generate() override;

public:
	std::string m_strName;
	std::shared_ptr<Expression> m_pValue;
};

class NullLiteral : public Expression
{
public:
	std::string PrintInfo(int32 _depth) override;
	std::any Interpret() override;
	void Generate() override;
};

class BooleanLiteral : public Expression
{
public:
	std::string PrintInfo(int32 _depth) override;
	std::any Interpret() override;
	void Generate() override;

public:
	bool m_bValue = false;
};

class NumberLiteral : public Expression
{
public:
	std::string PrintInfo(int32 _depth) override;
	std::any Interpret() override;
	void Generate() override;

public:
	uint64 m_uValue = 0;
};

class FloatLiteral : public Expression
{
public:
	std::string PrintInfo(int32 _depth) override;
	std::any Interpret() override;
	void Generate() override;

public:
	float64 m_dValue = 0.0;
};

class StringLiteral : public Expression
{
public:
	std::string PrintInfo(int32 _depth) override;
	std::any Interpret() override;
	void Generate() override;

public:
	std::string m_strValue;
};

class ArrayLiteral : public Expression
{
public:
	std::string PrintInfo(int32 _depth) override;
	std::any Interpret() override;
	void Generate() override;

public:
	std::vector<std::shared_ptr<Expression>> m_vecValue;
};

class MapLiteral : public Expression
{
public:
	std::string PrintInfo(int32 _depth) override;
	std::any Interpret() override;
	void Generate() override;

public:
	std::map<std::string, std::shared_ptr<Expression>> m_mapValue;
};

struct ClassMemberVariable
{
public:
	std::shared_ptr<Variable> m_pVariable;
	EMemberAccess m_eAccess = EMemberAccess::None;
};

class Class : public Statement
{
public:
	std::string PrintInfo(int32 _depth) override;
	void Interpret() override;
	void Generate() override;

public:
	std::string m_strName;
	std::vector<ClassMemberVariable> m_vecVariable;
};

class SetClassAccess : public Expression
{
public:
	std::string PrintInfo(int32 _depth) override;
	std::any Interpret() override;
	void Generate() override;

public:
	std::shared_ptr<Expression> m_pSub;
	std::shared_ptr<Expression> m_pMember;
	std::shared_ptr<Expression> m_pValue;
	std::string m_strName;
};

class GetClassAccess : public Expression
{
public:
	std::string PrintInfo(int32 _depth) override;
	std::any Interpret() override;
	void Generate() override;
	
public:
	std::shared_ptr<Expression> m_pSub;
	std::shared_ptr<Expression> m_pMember;
	std::string m_strName;
};