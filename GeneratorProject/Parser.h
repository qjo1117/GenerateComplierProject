#pragma once


#include <set>
#include <iostream>
#include "Token.h"
#include "Node.h"

class Parser
{
private:
	Parser() { }
public:
	__forceinline static Parser& GetInstance()
	{
		static Parser instance;
		return instance;
	}
public:
	std::shared_ptr<Program> Parse(std::vector<CodeToken> _vecCodeToken);
	
	std::shared_ptr<Function> ParseFunction();
	std::vector<std::shared_ptr<Statement>> ParseBlock();
	std::shared_ptr<Variable> ParseVariable();
	std::shared_ptr<For> ParseFor();
	std::shared_ptr<If> ParseIf();
	std::shared_ptr<Print> ParsePrint();
	std::shared_ptr<Return> ParseReturn();
	std::shared_ptr<Break> ParseBreak();
	std::shared_ptr<Continue> ParseContinue();
	std::shared_ptr<ExpressionStatement> ParseExpressionStatement();
	std::shared_ptr<Expression> ParseExpression();
	std::shared_ptr<Expression> ParseAssignment();
	std::shared_ptr<Expression> ParseOr();
	std::shared_ptr<Expression> ParseAnd();
	std::shared_ptr<Expression> ParseRelational();
	std::shared_ptr<Expression> ParseArithmetic1();
	std::shared_ptr<Expression> ParseArithmetic2();
	std::shared_ptr<Expression> ParseUnary();
	std::shared_ptr<Expression> ParseOperand();
	std::shared_ptr<Expression> ParseNullLiteral();
	std::shared_ptr<Expression> ParseBooleanLiteral();
	std::shared_ptr<Expression> ParseNumberLiteral();
	std::shared_ptr<Expression> ParseFloatLiteral();
	std::shared_ptr<Expression> ParseStringLiteral();
	std::shared_ptr<Expression> ParseListLiteral();
	std::shared_ptr<Expression> ParseMapLiteral();
	std::shared_ptr<Expression> ParseIdentifier();
	std::shared_ptr<Expression> ParseInnerExpression();
	std::shared_ptr<Expression> ParsePostfix(std::shared_ptr<Expression> _pSub);
	std::shared_ptr<Expression> ParseCall(std::shared_ptr<Expression> _pSub);
	std::shared_ptr<Expression> ParseElement(std::shared_ptr<Expression> _pSub);
	std::shared_ptr<Class> ParseClass();
	std::shared_ptr<GetClassAccess> ParseClassAccess(std::shared_ptr<Expression> _pSub);
	
	void SkipCurrent();
	void SkipCurrent(EKind _eKind);
	bool SkipCurrentIf(EKind _eKind);
};

































