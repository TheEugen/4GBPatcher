#pragma once

#include <iostream>


class PathHandler
{
	std::wstring m_filepath;
public:

	void setPath(std::wstring filepath) { m_filepath = filepath; }
	std::wstring getPath() { return m_filepath; }
};