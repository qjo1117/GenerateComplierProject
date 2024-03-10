#include "Token.h"
#include <iomanip>

#pragma region ���� ���� ���̺�
// å������ �����Լ��� Kind2String�� ����������
// �Լ�ȣ��� ���� ���̺��� �� �����ϴ� �� �볳�� �ȵǾ ������
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

std::string ToKindString(CodeToken& _token)
{
	return ToString(_token.m_eKind);
}

std::string ToTokenString(CodeToken& _token)
{
	return _token.m_strName;
}