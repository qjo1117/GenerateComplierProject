#pragma once
#include <any>
#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <functional>
#include "Node.h"

using std::any;
using std::map;
using std::vector;
using std::string;
using std::ostream;
using std::function;

struct Array;
struct Map;

struct Object 
{
	bool m_bMarked = false;
	virtual ~Object() {}

	static bool IsSize(std::any _anyValue);
	static std::size_t ToSize(std::any _anyValue);
	static bool IsNull(std::any _anyValue);
	static bool IsTrue(std::any _anyValue);
	static bool IsFalse(std::any _anyValue);
	static bool IsBoolean(std::any _anyValue);
	static bool ToBoolean(std::any _anyValue);
	static bool IsNumber(std::any _anyValue);
	static bool IsFloat(std::any _anyValue);
	static uint64 ToNumber(std::any _anyValue);
	static float64 ToFloat(std::any _anyValue);
	static bool IsString(std::any _anyValue);
	static std::string ToString(std::any _anyValue);
	static bool IsArray(std::any _anyValue);
	static std::shared_ptr<Array> ToArray(std::any _anyValue);
	static std::any GetValueOfArray(std::any _anyObject, std::any _anyIndex);
	static std::any SetValueOfArray(std::any _anyObject, std::any _anyIndex, std::any _anyValue);
	static bool IsMap(std::any _anyValue);
	static std::shared_ptr<Map> ToMap(std::any _anyValue);
	static std::any GetValueOfMap(std::any _anyObject, std::any _anyKey);
	static std::any SetValueOfMap(std::any _anyObject, std::any _anyKey, std::any _anyValue);
	static bool IsFunction(std::any _anyValue);
	static std::shared_ptr<Function> ToFunction(std::any _anyValue);
	static bool IsBuiltinFunction(std::any _anyValue);
	static std::function<std::any(std::vector<std::any>)> ToBuiltinFunction(std::any _anyValue);
};

struct Array : Object 
{
	std::vector<std::any> m_vecValue;
};

struct Map : Object 
{
	std::map<std::string, std::any> m_mapValue;
};

std::string AnyToString(std::any _anyValue);
std::ostream& operator<<(ostream& _os, any& _anyValue);
