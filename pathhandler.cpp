#include "pathhandler.h"


void PathHandler::setPath(std::wstring filepath)
{
	m_filepath = filepath;
}

std::wstring PathHandler::getPath()
{
	return m_filepath;
}