#pragma once

#include <map>
#include <iostream>
#include <string>

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
	EKind kind = EKind::Unknown;
	std::string str;
};

const EKind ToKind(const std::string& _str) noexcept;
const std::string ToString(EKind _kind) noexcept;

std::ostream& operator<<(std::ostream&, CodeToken& _token);