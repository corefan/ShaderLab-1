#ifndef _SHADERLAB_PARSER_VARIABLE_H_
#define _SHADERLAB_PARSER_VARIABLE_H_

#include "VariableType.h"

#include <string>

namespace sl
{
namespace parser
{

class Variable
{
public:
	Variable(VariableType type, std::string name) 
		: m_type(type), m_name(name) {}
	virtual ~Variable() {}

	virtual std::string& ToStatement(std::string& str) const = 0;

	VariableType GetType() const { return m_type; }
	const std::string& GetName() const { return m_name; }

protected:
	VariableType m_type;
	std::string m_name;

}; // Variable

}
}

#endif // _SHADERLAB_PARSER_VARIABLE_H_