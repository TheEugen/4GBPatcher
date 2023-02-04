#pragma once

#include <iostream>


class PathHandler
{
	std::wstring m_filepath;
public:

	void setPath(std::wstring filepath);
	std::wstring getPath();
};