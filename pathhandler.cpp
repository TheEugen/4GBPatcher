#include "pathhandler.h"


PathHandler::PathHandler(std::wstring filepath)
{
	m_filepath = std::wstring(filepath);
}

std::wstring PathHandler::getPath()
{
	return m_filepath;
}