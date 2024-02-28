#include "Token.h"
#include <iomanip>

#define STR2KIND(STRING, KIND)	{STRING, EKind::##KIND}
#define KIND2STR(KIND, STRING)	{EKind::##KIND, STRING}

static std::map<std::string, EKind> KindTable =
{
#define X(A, B) { ##A, EKind::##B },
#include "TokenDefine.ini"
#undef X
};

static std::map<EKind, std::string> StringTable =
{
#define X(A, B) { EKind::##B, ##A },
#include "TokenDefine.ini"
#undef X
};

const EKind ToKind(const std::string& _str) noexcept
{
	if (KindTable.count(_str)) {
		return KindTable.at(_str);
	}
	return EKind::Unknown;
}

const std::string ToString(EKind _kind) noexcept
{
	if (StringTable.count(_kind)) {
		return StringTable.at(_kind);
	}
	return "";
}

std::ostream& operator<<(std::ostream& _io, CodeToken& _token)
{
	return _io << std::setw(12) << std::left << ToString(_token.kind) << _token.str;
}