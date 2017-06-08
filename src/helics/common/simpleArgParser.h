#ifndef SIMPLE_ARG_PARSER_H_
#define SIMPLE_ARG_PARSER_H_
#pragma once
#include <map>

class SimpleArgParser
{
public:
	SimpleArgParser() noexcept;
	SimpleArgParser(const std::string &arguments);

	void Parse(const std::string &arguments);
	bool isField(const std::string &field) const;
	std::string getField(const std::string &field);

private:
	std::map<std::string, std::string> extracted_fields;


};
#endif
