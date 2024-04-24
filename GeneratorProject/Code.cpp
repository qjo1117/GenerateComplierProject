#include "Code.h"
#include "imgui.h"
#include <string>

namespace
{	
	static std::map<string, Instruction> g_stringToIntruction = 
	{
#define _X(Code) { #Code, Instruction::##Code },
#include "CodeDefine.ini"
#undef _X
	};

	static std::map<Instruction, std::string> g_mapInstructionToStringTable
	{
#define _X(Code) { Instruction::##Code, #Code },
#include "CodeDefine.ini"
#undef _X
	};
	std::map<Instruction, std::string> InstructionToString()
	{
		for (auto& [key, value] : g_stringToIntruction) {
			g_mapInstructionToStringTable[value] = key;
		}
		return g_mapInstructionToStringTable;
	}

	// 문자열을 주어진 길이로 왼쪽으로 정렬하는 함수
	std::string LeftAlign(const std::string& _str, size_t _width) {
		if (_str.length() >= _width) {
			return _str;
		}
		ImVec2 currentSize = ImGui::CalcTextSize(_str.c_str());
		ImVec2 spaceSize = ImGui::CalcTextSize(" ");
		uint64 totalWidth = spaceSize.x * _width;
		uint64 count = 0;
		while (totalWidth > (spaceSize.x * count) + currentSize.x) {
			count += 1;
		}
		if (count >= 4) {
			count -= 5;
			return _str + std::string(count, ' ') + "\t";
		}
		else {
			return _str + std::string(count, ' ');
		}
	}
}

std::string ToString(Instruction _instruction)
{
	if (g_mapInstructionToStringTable.count(_instruction)) {
		return g_mapInstructionToStringTable.at(_instruction);
	}
	return "";
}

std::string PrintCode(Code& _code)
{
	std::string strResult;
	auto& type = _code.m_anyOperand.type();
	if (type == typeid(uint64)) {
		strResult += "[" + std::to_string(std::any_cast<uint64>(_code.m_anyOperand)) + "]";
	}
	else if (type == typeid(bool)) {
		strResult += std::any_cast<bool>(_code.m_anyOperand) ? "true" : "false";
	}
	else if (type == typeid(float64)) {
		strResult += std::to_string(std::any_cast<float64>(_code.m_anyOperand));
	}
	else if (type == typeid(std::string)) {
		strResult += "\"" + std::any_cast<std::string>(_code.m_anyOperand) + "\"";
	}
	if (strResult.empty()) {
		return LeftAlign(ToString(_code.m_instruction), 24);
	}
	else {
		return LeftAlign(ToString(_code.m_instruction), 24) + strResult;
	}
}

std::ostream& operator<<(ostream& _stream, Code& _code)
{
	_stream << std::setw(15) << std::left << ToString(_code.m_instruction);
	if (_code.m_anyOperand.type() == typeid(size_t)) {
		_stream << "[" << std::any_cast<size_t>(_code.m_anyOperand) << "]";
	}
	else if (_code.m_anyOperand.type() == typeid(bool)) {
		_stream << std::boolalpha << std::any_cast<bool>(_code.m_anyOperand);
	}
	else if (_code.m_anyOperand.type() == typeid(double)) {
		_stream << std::any_cast<double>(_code.m_anyOperand);
	}
	else if (_code.m_anyOperand.type() == typeid(std::string)) {
		_stream << "\"" << std::any_cast<std::string>(_code.m_anyOperand) << "\"";
	}
	return _stream;
}
