#pragma once

#include <map>
#include <iostream>
#include <string>
#include "TypeDefine.h"

enum class EKind
{
	Unknown,
	EndOfToken,
	NullLiteral,
	TrueLiteral,
	FalseLiteral,
	NumberLiteral,
	StringLiteral,
	Identifier,

	Function,
	Return,
	Variable,
	For,
	Break,
	Continue,
	If,
	ElIf,
	Else,
	
	Print,
	PrintLine,

	LogicalAnd,
	LogicalOr,
	Assignment,
	Add,
	Subtract,
	Multiply,
	Divide,
	Modulo,
	Equal,
	NotEqual,

	LessThan,
	GreaterThan,
	LessOrEqual,
	GreaterOrEqual,

	Comma,
	Colon,
	Semicolon,
	LeftParen,
	RightParen,
	LeftBrace,
	RightBrace,
	LeftBraket,
	RightBraket,
};

struct CodeToken
{
	std::string name;
	int32 row, col;
	EKind kind = EKind::Unknown;
};

const EKind ToKind(const std::string& _str) noexcept;
const std::string ToString(EKind _kind) noexcept;

std::ostream& operator<<(std::ostream&, CodeToken& _token);