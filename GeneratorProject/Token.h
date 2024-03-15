#pragma once

#include <map>
#include <iostream>
#include <string>
#include "TypeDefine.h"

enum class EKind
{
#define X(A, B) B,
#include "TokenDefine.ini"
#undef X
};

struct CodeToken
{
	std::string m_strName;
	int32 m_iRow, m_iCol;
	EKind m_eKind = EKind::Unknown;
};

const EKind ToKind(const std::string& _str) noexcept;
const std::string ToString(EKind _kind) noexcept;

std::string ToKindString(CodeToken& _token);
std::string ToTokenString(CodeToken& _token);