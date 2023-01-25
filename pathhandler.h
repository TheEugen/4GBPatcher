#pragma once

#include <iostream>


class PathHandler
{
	std::wstring m_filepath;
public:

	PathHandler(std::wstring filepath);
	std::wstring getPath();
};