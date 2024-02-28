#include "Token.h"
#include <iomanip>

#pragma region 전역 변수 테이블
// 책에서는 람다함수로 Kind2String을 구현했지만
// 함수호출시 마다 테이블을 재 구성하는 건 용납이 안되어서 변경함
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
#pragma endregion

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