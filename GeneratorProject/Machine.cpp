#include "Machine.h"
#include "Object.h"
#include "Application.h"

void Machine::Execute(std::tuple<std::vector<Code>, std::map<std::string, std::size_t>> _objectCode)
{
    m_mapGlobal.clear();
    m_vecObject.clear();
    m_vecCallStack.emplace_back();
    auto vecCodeList = std::get<0>(_objectCode);
    auto functionTable = std::get<1>(_objectCode);
    while (true) 
    {
        auto code = vecCodeList[m_vecCallStack.back().m_instructionPointer];
        switch (code.m_instruction) 
        {
        case Instruction::Exit: 
            {
                m_vecCallStack.pop_back();
            }
            return;
        case Instruction::Call: 
            {
                auto operand = PopOperand();
                if (Object::IsSize(operand)) 
                {
                    StackFrame stackFrame;
                    stackFrame.m_instructionPointer = Object::ToSize(operand);
                    for (size_t i = 0; i < Object::ToSize(code.m_anyOperand); i++) {
                        stackFrame.m_vecVariable.push_back(m_vecCallStack.back().m_vecOperandStack.back());
                        m_vecCallStack.back().m_vecOperandStack.pop_back();
                    }
                    m_vecCallStack.push_back(stackFrame);
                    continue;
                }
                if (Object::IsBuiltinFunction(operand)) {
                    vector<any> arguments;
                    for (size_t i = 0; i < Object::ToSize(code.m_anyOperand); i++)
                        arguments.push_back(PopOperand());
                    PushOperand(Object::ToBuiltinFunction(operand)(arguments));
                    break;
                }
                PushOperand(nullptr);
            }
            break;
        case Instruction::Alloca: 
            {
                auto extraSize = Object::ToSize(code.m_anyOperand);
                auto currentSize = m_vecCallStack.back().m_vecVariable.size();
                m_vecCallStack.back().m_vecVariable.resize(currentSize + extraSize);
            }
            break;
        case Instruction::Return: 
            {
                any result = nullptr;
                if (m_vecCallStack.back().m_vecOperandStack.empty() == false) {
                    result = m_vecCallStack.back().m_vecOperandStack.back();
                }
                m_vecCallStack.pop_back();
                m_vecCallStack.back().m_vecOperandStack.push_back(result);
                CollectGarbage();
            }
            break;
        case Instruction::Jump: 
            {
                m_vecCallStack.back().m_instructionPointer = Object::ToSize(code.m_anyOperand);
            }
            continue;
        case Instruction::ConditionJump: 
            {
                auto condition = PopOperand();
                if (Object::IsTrue(condition)) {
                    break;
                }
                m_vecCallStack.back().m_instructionPointer = Object::ToSize(code.m_anyOperand);
            }
            continue;
        case Instruction::Print: 
            {
#ifdef USE_APPLICATION_IMGUI
                for (size_t i = 0; i < Object::ToSize(code.m_anyOperand); i++) {
                    auto value = PopOperand();
                    ImGui::Text(AnyToString(value).c_str());
                }
#else                
                for (size_t i = 0; i < Object::ToSize(code.m_anyOperand); i++) {
                    auto value = PopOperand();
                    std::cout << AnyToString(value).c_str());
                }
#endif
            }
            break;
        case Instruction::PrintLine: 
            {
#ifdef USE_APPLICATION_IMGUI
                ImGui::Text("\n");
#else      
                std::cout << '\n';
#endif
            }
            break;
        case Instruction::LogicalOr: 
            {
                auto value = PopOperand();
                if (Object::IsTrue(value)) {
                    PushOperand(value);
                    m_vecCallStack.back().m_instructionPointer = Object::ToSize(code.m_anyOperand);
                    continue;
                }
            }
            break;
        case Instruction::LogicalAnd: 
            {
                auto value = PopOperand();
                if (Object::IsFalse(value)) {
                    PushOperand(value);
                    m_vecCallStack.back().m_instructionPointer = Object::ToSize(code.m_anyOperand);
                    continue;
                }
            }
            break;
        case Instruction::Equal: 
            {
                auto rValue = PopOperand();
                auto lValue = PopOperand();
                if (Object::IsNull(lValue) && Object::IsNull(rValue)) {
                    PushOperand(true);
                }
                else if (Object::IsBoolean(lValue) && Object::IsBoolean(rValue)) {
                    PushOperand(Object::ToBoolean(lValue) == Object::ToBoolean(rValue));
                }
                else if (Object::IsNumber(lValue) && Object::IsNumber(rValue)) {
                    PushOperand(Object::ToNumber(lValue) == Object::ToNumber(rValue));
                }
                else if (Object::IsString(lValue) && Object::IsString(rValue)) {
                    PushOperand(Object::ToString(lValue) == Object::ToString(rValue));
                }
                else {
                    PushOperand(false);
                }
            }
            break;
        case Instruction::NotEqual: 
            {
                auto rValue = PopOperand();
                auto lValue = PopOperand();
                if (Object::IsNull(lValue) && Object::IsNull(rValue)) {
                    PushOperand(false);
                }
                else if (Object::IsNull(lValue) || Object::IsNull(rValue)) {
                    PushOperand(true);
                }

                if (Object::IsBoolean(lValue) && Object::IsBoolean(rValue)) {
                    PushOperand(Object::ToBoolean(lValue) != Object::ToBoolean(rValue));
                }
                else if (Object::IsNumber(lValue) && Object::IsNumber(rValue)) {
                    PushOperand(Object::ToNumber(lValue) != Object::ToNumber(rValue));
                }
                else if (Object::IsString(lValue) && Object::IsString(rValue)) {
                    PushOperand(Object::ToString(lValue) != Object::ToString(rValue));
                }
                else {
                    PushOperand(false);
                }
            }
            break;
        case Instruction::LessThan: 
            {
                auto rValue = PopOperand();
                auto lValue = PopOperand();
                if (Object::IsNumber(lValue) && Object::IsNumber(rValue)) {
                    PushOperand(Object::ToNumber(lValue) < Object::ToNumber(rValue));
                }
                else {
                    PushOperand(false);
                }
            }
            break;
        case Instruction::GreaterThan: 
            {
                auto rValue = PopOperand();
                auto lValue = PopOperand();
                if (Object::IsNumber(lValue) && Object::IsNumber(rValue)) {
                    PushOperand(Object::ToNumber(lValue) > Object::ToNumber(rValue));
                }
                else {
                    PushOperand(false);
                }
            }
            break;
        case Instruction::LessOrEqual: 
             {
                 auto rValue = PopOperand();
                 auto lValue = PopOperand();
                 if (Object::IsNumber(lValue) && Object::IsNumber(rValue)) {
                     PushOperand(Object::ToNumber(lValue) <= Object::ToNumber(rValue));
                 }
                 else {
                     PushOperand(false);
                 }
             }
             break;
        case Instruction::GreaterOrEqual: 
             {
                auto rValue = PopOperand();
                auto lValue = PopOperand();
                if (Object::IsNumber(lValue) && Object::IsNumber(rValue)) {
                    PushOperand(Object::ToNumber(lValue) >= Object::ToNumber(rValue));
                }
                else {
                    PushOperand(false);
                }
            }
            break;
        case Instruction::Add: 
            {
                auto rValue = PopOperand();
                auto lValue = PopOperand();
                if (Object::IsNumber(lValue) && Object::IsNumber(rValue)) {
                    PushOperand(Object::ToNumber(lValue) + Object::ToNumber(rValue));
                }
                else if (Object::IsString(lValue) && Object::IsString(rValue)) {
                    PushOperand(Object::ToString(lValue) + Object::ToString(rValue));
                }
                else {
                    PushOperand(0.0);
                }
            }
            break;
        case Instruction::Subtract: 
            {
                auto rValue = PopOperand();
                auto lValue = PopOperand();
                if (Object::IsNumber(lValue) && Object::IsNumber(rValue)) {
                    PushOperand(Object::ToNumber(lValue) - Object::ToNumber(rValue));
                }
                else {
                    PushOperand(0.0);
                }
            }
            break;
        case Instruction::Multiply: 
            {
                auto rValue = PopOperand();
                auto lValue = PopOperand();
                if (Object::IsNumber(lValue) && Object::IsNumber(rValue)) {
                    PushOperand(Object::ToNumber(lValue)* Object::ToNumber(rValue));
                }
                else {
                    PushOperand(0.0);
                }
            }
            break;
        case Instruction::Divide: 
            {
                auto rValue = PopOperand();
                auto lValue = PopOperand();
                if (Object::IsNumber(lValue) && Object::IsNumber(rValue) && Object::ToNumber(rValue) == 0) {
                    PushOperand(0.0);
                }
                else if (Object::IsNumber(lValue) && Object::IsNumber(rValue)) {
                    PushOperand(Object::ToNumber(lValue) / Object::ToNumber(rValue));
                }
                else {
                    PushOperand(0.0);
                }
            }
            break;
        case Instruction::Modulo: 
            {
                auto rValue = PopOperand();
                auto lValue = PopOperand();
                if (Object::IsNumber(lValue) && Object::IsNumber(rValue) && Object::ToNumber(rValue) == 0) {
                    PushOperand(0.0);
                }
                else if (Object::IsNumber(lValue) && Object::IsNumber(rValue)) {
                    PushOperand(fmod(Object::ToNumber(lValue), Object::ToNumber(rValue)));
                }
                else {
                    PushOperand(0.0);
                }
            }
            break;
        case Instruction::Absolute: 
            {
                auto value = PopOperand();
                if (Object::IsNumber(value)) {
                    PushOperand(Object::ToNumber(value));
                }
                else {
                    PushOperand(0.0);
                }
            }
            break;
        case Instruction::ReverseSign: 
            {
                auto value = PopOperand();
                if (Object::IsNumber(value)) {
                    PushOperand(Object::ToNumber(value) * -1);
                }
                else {
                    PushOperand(0.0);
                }
            }
            break;
        case Instruction::GetElement: 
            {
                auto index = PopOperand();
                auto sub = PopOperand();
                if (Object::IsArray(sub) && Object::IsNumber(index)) {
                    PushOperand(Object::GetValueOfArray(sub, index));
                }
                else if (Object::IsMap(sub) && Object::IsString(index)) {
                    PushOperand(Object::GetValueOfMap(sub, index));
                }
                else {
                    PushOperand(nullptr);
                }
            }
            break;
        case Instruction::SetElement: 
            {
                auto index = PopOperand();
                auto sub = PopOperand();
                if (Object::IsArray(sub) && Object::IsNumber(index)) {
                    Object::SetValueOfArray(sub, index, PeekOperand());
                }
                else if (Object::IsMap(sub) && Object::IsString(index)) {
                    Object::SetValueOfMap(sub, index, PeekOperand());
                }
            }
            break;
        case Instruction::GetGlobal: 
            {
                auto name = Object::ToString(code.m_anyOperand);
                if (functionTable.count(name)) {
                    PushOperand(functionTable[name]);
                }
                else if (m_mapBuiltinFunctionTable.count(name)) {
                    PushOperand(m_mapBuiltinFunctionTable[name]);
                }
                else if (m_mapGlobal.count(name)) {
                    PushOperand(m_mapGlobal[name]);
                }
                else {
                    PushOperand(nullptr);
                }
            }
            break;
        case Instruction::SetGlobal: 
            {
                auto name = Object::ToString(code.m_anyOperand);
                m_mapGlobal[name] = PeekOperand();
            }
            break;
        case Instruction::GetLocal: 
            {
                auto index = Object::ToSize(code.m_anyOperand);
                PushOperand(m_vecCallStack.back().m_vecVariable[index]);
            }
            break;
        case Instruction::SetLocal: 
            {
                auto index = Object::ToSize(code.m_anyOperand);
                m_vecCallStack.back().m_vecVariable[index] = PeekOperand();
            }
            break;
        case Instruction::PushNull: 
            {
                PushOperand(nullptr);
            }
            break;
        case Instruction::PushBoolean: 
            {
                PushOperand(code.m_anyOperand);
            }
            break;
        case Instruction::PushNumber: 
            {
                PushOperand(code.m_anyOperand);
            }
            break;
        case Instruction::PushString: 
            {
                PushOperand(code.m_anyOperand);
            }
            break;
        case Instruction::PushArray: 
            {
                auto pResult = std::make_shared<Array>();
                auto size = Object::ToSize(code.m_anyOperand);
                for (auto i = size; i > 0; i--)
                    pResult->m_vecValue.push_back(PopOperand());
                PushOperand(pResult);
                m_vecObject.push_back(pResult);
            }
            break;
        case Instruction::PushMap: 
            {
                auto pResult = std::make_shared<Map>();
                for (size_t i = 0; i < Object::ToSize(code.m_anyOperand); i++) {
                    auto value = PopOperand();
                    auto key = Object::ToString(PopOperand());
                    pResult->m_mapValue[key] = value;
                }
                PushOperand(pResult);
                m_vecObject.push_back(pResult);
            }
            break;
        case Instruction::PopOperand: 
            {
                PopOperand();
            }
            break;
        }
        m_vecCallStack.back().m_instructionPointer += 1;
    }
}

void Machine::PushOperand(std::any _value)
{
	m_vecCallStack.back().m_vecOperandStack.push_back(_value);
}

std::any Machine::PeekOperand()
{
	return m_vecCallStack.back().m_vecOperandStack.back();
}

std::any Machine::PopOperand()
{
	auto value = m_vecCallStack.back().m_vecOperandStack.back();
	m_vecCallStack.back().m_vecOperandStack.pop_back();
	return value;
}

void Machine::CollectGarbage()
{
	for (StackFrame& stackFrame : m_vecCallStack) 
	{
		for (auto& value : stackFrame.m_vecOperandStack)
		{
			MarkObject(value);
		}
		for (auto& value : stackFrame.m_vecVariable)
		{
			MarkObject(value);
		}
	}
	for (auto& [key, value] : m_mapGlobal)
	{
		MarkObject(value);
	}
	SweepObject();
}

void Machine::MarkObject(std::any _object)
{
	if (Object::IsArray(_object)) 
	{
		if (Object::ToArray(_object)->m_bMarked)
		{
			return;
		}
		Object::ToArray(_object)->m_bMarked = true;
		for (auto& value : Object::ToArray(_object)->m_vecValue)
		{
			MarkObject(value);
		}
	}
	else if (Object::IsMap(_object)) 
	{
		if (Object::ToMap(_object)->m_bMarked)
		{
			return;
		}
		Object::ToMap(_object)->m_bMarked = true;
		for (auto& [key, value] : Object::ToMap(_object)->m_mapValue)
		{
			MarkObject(value);
		}
	}
}

void Machine::SweepObject()
{
	m_vecObject.remove_if([](std::shared_ptr<Object> _pObject) 
	{
		if (_pObject->m_bMarked)
		{
			_pObject->m_bMarked = false;
			return false;
		}
		return true;
	});
}
