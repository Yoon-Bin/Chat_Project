#pragma once

class Exception : public std::exception
{
public:
	Exception(LPCSTR text) : m_text(text) {}
	~Exception() {}

	std::string m_text;

	const char* what() { return m_text.c_str(); }

};
