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
#define GET_INTERPRETER()	Interpreter::GetInstance()
#define GLOBAL_TABLE		GET_INTERPRETER().m_mapGlobal
#define LOCAL_TABLE			GET_INTERPRETER().m_listLocalFrame
#define FUNCTION_TABLE		GET_INTERPRETER().m_mapFunctionTable
#define BUILTIN_TABLE		GET_INTERPRETER().m_mapBuiltinFunctionTable
#define CLASS_TABLE			GET_INTERPRETER().m_mapClassDefaultTable

public:
	void Interpret(std::shared_ptr<Program> _pProgram);

public:
	inline static std::map<std::string, std::any> m_mapGlobal;
	inline static std::list<std::list<std::map<std::string, std::any>>> m_listLocalFrame;
	inline static std::map<std::string, std::shared_ptr<Function>> m_mapFunctionTable;
	inline static std::map<std::string, ScriptFunctionType> m_mapBuiltinFunctionTable;
	inline static std::map<std::string, std::vector<std::tuple<EMemberAccess, std::any>>> m_mapClassDefaultTable;
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
};

class Expression 
{
public:
	virtual std::string PrintInfo(int32 _depth) = 0;
	virtual std::any Interpret() = 0;
};

class Function : public Statement 
{
public:
	std::string PrintInfo(int32 _depth) override;
	void Interpret() override;

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

public:
	std::shared_ptr<Expression> m_pExpression;
};

class For : public Statement 
{
public:
	std::string PrintInfo(int32 _depth) override;
	void Interpret() override;

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
};

class Continue : public Statement 
{
public:
	std::string PrintInfo(int32 _depth) override;
	void Interpret() override;
};

class If : public Statement 
{
public:
	std::string PrintInfo(int32 _depth) override;
	void Interpret() override;

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

public:	
	bool m_bLineFeed = false;
	std::vector<std::shared_ptr<Expression>> m_vecArgument;
};

class ExpressionStatement : public Statement 
{
public:
	std::string PrintInfo(int32 _depth) override;
	void Interpret() override;

public:
	std::shared_ptr<Expression> m_pExpression;
};

class Or : public Expression
{
public:
	std::string PrintInfo(int32 _depth) override;
	std::any Interpret() override;

public:
	std::shared_ptr<Expression> m_pLhs;
	std::shared_ptr<Expression> m_pRhs;
};

class And : public Expression
{
public:
	std::string PrintInfo(int32 _depth) override;
	std::any Interpret() override;

public:
	std::shared_ptr<Expression> m_pLhs;
	std::shared_ptr<Expression> m_pRhs;
};

class Relational : public Expression
{
public:
	std::string PrintInfo(int32 _depth) override;
	std::any Interpret() override;

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

public:
	EKind m_eKind = EKind::Unknown;
	std::shared_ptr<Expression> m_pSub;
};

class Call : public Expression
{
public:
	std::string PrintInfo(int32 _depth) override;
	std::any Interpret() override;

public:
	std::shared_ptr<Expression> m_pSub;
	std::vector<std::shared_ptr<Expression>> m_vecArgument;
};

class GetElement : public Expression
{
public:
	std::string PrintInfo(int32 _depth) override;
	std::any Interpret() override;

public:
	std::shared_ptr<Expression> m_pSub;
	std::shared_ptr<Expression> m_pIndex;
};

class SetElement : public Expression 
{
public:
	std::string PrintInfo(int32 _depth) override;
	std::any Interpret() override;

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

public:
	std::string m_strName;
};

class SetVariable : public Expression
{
public:
	std::string PrintInfo(int32 _depth) override;
	std::any Interpret() override;

public:
	std::string m_strName;
	std::shared_ptr<Expression> m_pValue;
};

class NullLiteral : public Expression
{
public:
	std::string PrintInfo(int32 _depth) override;
	std::any Interpret() override;
};

class BooleanLiteral : public Expression
{
public:
	std::string PrintInfo(int32 _depth) override;
	std::any Interpret() override;

public:
	bool m_bValue = false;
};

class NumberLiteral : public Expression
{
public:
	std::string PrintInfo(int32 _depth) override;
	std::any Interpret() override;

public:
	uint64 m_uValue = 0;
};

class FloatLiteral : public Expression
{
public:
	std::string PrintInfo(int32 _depth) override;
	std::any Interpret() override;

public:
	float64 m_dValue = 0.0;
};

class StringLiteral : public Expression
{
public:
	std::string PrintInfo(int32 _depth) override;
	std::any Interpret() override;

public:
	std::string m_strValue;
};

class ArrayLiteral : public Expression
{
public:
	std::string PrintInfo(int32 _depth) override;
	std::any Interpret() override;

public:
	std::vector<std::shared_ptr<Expression>> m_vecValue;
};

class MapLiteral : public Expression
{
public:
	std::string PrintInfo(int32 _depth) override;
	std::any Interpret() override;

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

public:
	std::string m_strName;
	std::vector<ClassMemberVariable> m_vecVariable;
};

class SetClassAccess : public Expression
{
public:
	std::string PrintInfo(int32 _depth) override;
	std::any Interpret() override;

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
	
public:
	std::shared_ptr<Expression> m_pSub;
	std::shared_ptr<Expression> m_pMember;
	std::string m_strName;
};