#include <iomanip>
#include <iostream>
#include "TypeDefine.h"
#include <cmath>
#include "Object.h"

using std::ostream;
using std::boolalpha;

using std::any;
using std::map;
using std::string;
using std::vector;
using std::function;

map<string, function<any(vector<any>)>> builtinFunctionTable = {
  {"length", [](vector<any> values)->any {
	if (values.size() == 1 && Object::IsArray(values[0])) {
		return static_cast<float64>(Object::ToArray(values[0])->m_vecValue.size());
	}
	if (values.size() == 1 && Object::IsMap(values[0])) {
		return static_cast<float64>(Object::ToMap(values[0])->m_mapValue.size());
	}
	return 0.0;
  }},
  {"push", [](vector<any> values)->any {
	if (values.size() == 2 && Object::IsArray(values[0])) {
	  Object::ToArray(values[0])->m_vecValue.push_back(values[1]);
	  return values[0];
	}
	return nullptr;
  }},
  {"pop", [](vector<any> values)->any {
	if (values.size() == 1 && Object::IsArray(values[0]) && Object::ToArray(values[0])->m_vecValue.size() != 0) {
	  auto result = Object::ToArray(values[0])->m_vecValue.back();
	  Object::ToArray(values[0])->m_vecValue.pop_back();
	  return result;
	}
	return nullptr;
  }},
  {"erase", [](vector<any> values)->any {
	if (values.size() == 2 && Object::IsMap(values[0]) && Object::IsString(values[1]) &&
		Object::ToMap(values[0])->m_mapValue.count(Object::ToString(values[1]))) {
	  auto result = Object::ToMap(values[0])->m_mapValue.at(Object::ToString(values[1]));
	  Object::ToMap(values[0])->m_mapValue.erase(Object::ToString(values[1]));
	  return result;
	}
	return nullptr;
  }},
  {"clock", [](vector<any> values)->any {
	return static_cast<float64>(clock());
  }},
  {"sqrt", [](vector<any> values)->any {
	return sqrt(Object::ToNumber(values[0]));
  }},
};


bool Object::IsSize(std::any _anyValue)
{
	return _anyValue.type() == typeid(size_t);
}

std::size_t Object::ToSize(std::any _anyValue)
{
	return std::any_cast<size_t>(_anyValue);
}

bool Object::IsNull(std::any _anyValue)
{
	return _anyValue.type() == typeid(nullptr_t);
}

bool Object::IsTrue(std::any _anyValue)
{
	return IsBoolean(_anyValue) && ToBoolean(_anyValue);
}

bool Object::IsFalse(std::any _anyValue)
{
	return IsBoolean(_anyValue) && ToBoolean(_anyValue) == false;
}

bool Object::IsBoolean(std::any _anyValue)
{
	return _anyValue.type() == typeid(bool);
}

bool Object::ToBoolean(std::any _anyValue)
{
	return std::any_cast<bool>(_anyValue);
}

bool Object::IsNumber(std::any _anyValue)
{
	return _anyValue.type() == typeid(double);
}

float64 Object::ToNumber(std::any _anyValue)
{
	return std::any_cast<float64>(_anyValue);
}

bool Object::IsString(std::any _anyValue)
{
	return _anyValue.type() == typeid(std::string);
}

std::string Object::ToString(std::any _anyValue)
{
	return std::any_cast<string>(_anyValue);
}

bool Object::IsArray(std::any _anyValue)
{
	return _anyValue.type() == typeid(std::shared_ptr<Array>);
}

std::shared_ptr<Array> Object::ToArray(std::any _anyValue)
{
	return std::any_cast<std::shared_ptr<Array>>(_anyValue);
}

std::any Object::GetValueOfArray(std::any _anyObject, std::any _anyIndex)
{
	std::size_t index = static_cast<std::size_t>(ToNumber(_anyIndex));
	if (index >= 0 && index < ToArray(_anyObject)->m_vecValue.size()) {
		return ToArray(_anyObject)->m_vecValue[index];
	}
	return nullptr;
}

std::any Object::SetValueOfArray(std::any _anyObject, std::any _anyIndex, std::any _anyValue)
{
	std::size_t index = static_cast<std::size_t>(ToNumber(_anyIndex));
	if (index >= 0 && index < ToArray(_anyObject)->m_vecValue.size()) {
		ToArray(_anyObject)->m_vecValue[index] = _anyValue;
	}
	return _anyValue;
}

bool Object::IsMap(std::any _anyValue)
{
	return _anyValue.type() == typeid(std::shared_ptr<Map>);
}

std::shared_ptr<Map> Object::ToMap(std::any _anyValue)
{
	return std::any_cast<std::shared_ptr<Map>>(_anyValue);
}

std::any Object::GetValueOfMap(std::any _anyObject, std::any _anyKey)
{
	if (ToMap(_anyObject)->m_mapValue.count(ToString(_anyKey))) {
		return ToMap(_anyObject)->m_mapValue[ToString(_anyKey)];
	}
	return nullptr;
}

std::any Object::SetValueOfMap(std::any _anyObject, std::any _anyKey, std::any _anyValue)
{
	ToMap(_anyObject)->m_mapValue[ToString(_anyKey)] = _anyValue;
	return _anyValue;
}

bool Object::IsFunction(std::any _anyValue)
{
	return _anyValue.type() == typeid(std::shared_ptr<Function>);
}

std::shared_ptr<Function> Object::ToFunction(std::any _anyValue)
{
	return std::any_cast<std::shared_ptr<Function>>(_anyValue);
}

bool Object::IsBuiltinFunction(std::any _anyValue)
{
	return _anyValue.type() == typeid(std::function<std::any(std::vector<std::any>)>);
}

std::function<std::any(std::vector<std::any>)> Object::ToBuiltinFunction(std::any _anyValue)
{
	return std::any_cast<std::function<std::any(std::vector<std::any>)>>(_anyValue);
}

std::string AnyToString(std::any _anyValue)
{
	std::string strResult;
	if (Object::IsNull(_anyValue)) {
		strResult = "null";
	}
	else if (Object::IsBoolean(_anyValue)) {
		strResult = std::any_cast<bool>(_anyValue) ? "true" : "false";
	}
	else if (Object::IsNumber(_anyValue)) {
		strResult = std::to_string(Object::ToNumber(_anyValue));
	}
	else if (Object::IsString(_anyValue)) {
		strResult = Object::ToString(_anyValue);
	}
	else if (Object::IsArray(_anyValue)) {
		strResult = "[ ";
		for (auto& value : Object::ToArray(_anyValue)->m_vecValue) {
			strResult += AnyToString(value) + " ";
		}
		strResult += "]";
	}
	else if (Object::IsMap(_anyValue)) {
		strResult = "{ ";
		for (auto& [key, value] : Object::ToMap(_anyValue)->m_mapValue) {
			strResult += key + ":" + AnyToString(value) + " ";
		}
		strResult += "}";
	}
	return strResult;
}

std::ostream& operator<<(ostream& _os, any& _anyValue)
{
	if (Object::IsNull(_anyValue)) {
		_os << "null";
	}
	else if (Object::IsBoolean(_anyValue)) {
		_os << std::boolalpha << std::any_cast<bool>(_anyValue);
	}
	else if (Object::IsNumber(_anyValue)) {
		printf("%g", Object::ToNumber(_anyValue));
	}
	else if (Object::IsString(_anyValue)) {
		_os << Object::ToString(_anyValue);
	}
	else if (Object::IsArray(_anyValue)) {
		_os << "[ ";
		for (auto& value : Object::ToArray(_anyValue)->m_vecValue) {
			_os << value << " ";
		}
		_os << "]";
	}
	else if (Object::IsMap(_anyValue)) {
		_os << "{ ";
		for (auto& [key, value] : Object::ToMap(_anyValue)->m_mapValue) {
			_os << key << ":" << value << " ";
		}
		_os << "}";
	}
	return _os;
}
