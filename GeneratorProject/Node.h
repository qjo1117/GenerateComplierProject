#pragma once

#include <memory>
#include <map>
#include <vector>
#include <string>
#include "TypeDefine.h"
#include "Token.h"

class Function;

class Program 
{
public:
	std::vector<std::shared_ptr<Function>> m_vecFunction;
};

class Statement 
{
public:
	virtual void PrintInfo(int32 _depth) = 0;
};

class Expression 
{
public:
	virtual void PrintInfo(int32 _depth) = 0;
};

class Function : public Statement 
{
public:
	void PrintInfo(int32 _depth);

public:
	std::string m_strName;
	std::vector<std::string> m_vecParameter;
	std::vector<std::shared_ptr<Statement>> m_vecBlock;
};

class Variable : public Statement 
{
public:
	void PrintInfo(int32 _depth);
public:
	std::string m_strName;
	std::shared_ptr<Expression> m_pExpression;
};

class Return : public Statement 
{
public:
	void PrintInfo(int32 _depth);

public:
	std::shared_ptr<Expression> m_pExpression;
};

class For : public Statement 
{
public:
	void PrintInfo(int32 _depth);

public:
	std::shared_ptr<Variable> m_pVariable;
	std::shared_ptr<Expression> m_pCondition;
	std::shared_ptr<Expression> m_pExpression;
	std::vector<std::shared_ptr<Statement>> m_vecBlock;
};

class Break : public Statement 
{
public:
	void PrintInfo(int32 _depth);
};

class Continue : public Statement 
{
public:
	void PrintInfo(int32 _depth);
};

class If : public Statement 
{
public:
	void PrintInfo(int32 _depth);
public:
	std::vector<std::shared_ptr<Expression>> conditions;
	std::vector<std::vector<std::shared_ptr<Statement>>> blocks;
	std::vector<std::shared_ptr<Statement>> m_vecElseBlock;
};

class Print : public Statement 
{
public:
	void PrintInfo(int32 _depth);
public:	
	bool lineFeed = false;
	std::vector<std::shared_ptr<Expression>> m_vecArgument;
};

class ExpressionStatement : public Statement 
{
public:
	void PrintInfo(int32 _depth);
public:
	std::shared_ptr<Expression> m_pExpression;
};

class Or : public Expression
{
public:
	void PrintInfo(int32 _depth);
public:
	std::shared_ptr<Expression> m_pLhs;
	std::shared_ptr<Expression> m_pRhs;
};

class And : public Expression
{
public:
	void PrintInfo(int32 _depth);
public:
	std::shared_ptr<Expression> m_pLhs;
	std::shared_ptr<Expression> m_pRhs;
};

class Relational : public Expression
{
public:
	void PrintInfo(int32 _depth);
public:
	EKind m_eKind = EKind::Unknown;
	std::shared_ptr<Expression> m_pLhs;
	std::shared_ptr<Expression> m_pRhs;
};

class Arithmetic : public Expression
{
public:
	void PrintInfo(int32 _depth);
public:
	EKind m_eKind = EKind::Unknown;
	std::shared_ptr<Expression> m_pLhs;
	std::shared_ptr<Expression> m_pRhs;
};

class Unary : public Expression
{
public:
	void PrintInfo(int32 _depth);
public:
	EKind m_eKind = EKind::Unknown;
	std::shared_ptr<Expression> m_pSub;
};

class Call : public Expression
{
public:
	void PrintInfo(int32 _depth);
public:
	std::shared_ptr<Expression> m_pSub;
	std::vector<std::shared_ptr<Expression>> m_vecArgument;
};

class GetElement : public Expression
{
public:
	void PrintInfo(int32 _depth);
public:
	std::shared_ptr<Expression> m_pSub;
	std::shared_ptr<Expression> m_pIndex;
};

class SetElement : public Expression 
{
public:
	void PrintInfo(int32 _depth);
public:
	std::shared_ptr<Expression> m_pSub;
	std::shared_ptr<Expression> m_pIndex;
	std::shared_ptr<Expression> m_pValue;
};

class GetVariable : public Expression
{
public:
	void PrintInfo(int32 _depth);
public:
	std::string m_strName;
};

class SetVariable : public Expression
{
public:
	void PrintInfo(int32 _depth);
public:
	std::string m_strName;
	std::shared_ptr<Expression> m_pValue;
};

class NullLiteral : public Expression
{
public:
	void PrintInfo(int32 _depth);
};

class BooleanLiteral : public Expression
{
public:
	void PrintInfo(int32 _depth);
public:
	bool m_bValue = false;
};

class NumberLiteral : public Expression
{
public:
	void PrintInfo(int32 _depth);
public:
	float64 m_dValue = 0.0;
};

class StringLiteral : public Expression
{
public:
	void PrintInfo(int32 _depth);
public:
	std::string m_strValue;
};

class ArrayLiteral : public Expression
{
public:
	void PrintInfo(int32 _depth);
public:
	std::vector<std::shared_ptr<Expression>> m_vecValue;
};

class MapLiteral : public Expression
{
public:
	void PrintInfo(int32 _depth);
public:
	std::map<std::string, std::shared_ptr<Expression>> m_vecValue;
};
