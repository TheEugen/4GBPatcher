#pragma once

#include <iostream>
#include <filesystem>
#include <Windows.h>
#include <bitset>
#include <stdlib.h>
#include <errno.h>

#include "resources.h"


void catchOpenFileError(HWND hwnd)
{
	std::wstring error_msg;
	switch (errno)
	{
		// access denied (e.g. .exe is running atm)
	case EACCES:
	{
		error_msg = L"Access denied, is the program running or the file read-only?";
		break;
	}
	// file too big
	case EFBIG:
	{
		error_msg = L"Could not open file because it's too big";
		break;
	}
	// path too long
	case ENAMETOOLONG:
	{
		error_msg = L"Could not open file because file path is too long";
		break;
	}
	default:
	{
		error_msg = L"Unknown error occured";
	}
	}

	OutputDebugStringW(L"error opening file");
	MessageBox(hwnd, error_msg.c_str(), L"4GBPatcher", MB_OK);
}


bool patchFile(std::wstring filepath, bool toBePatched, HWND hwnd)
{
	FILE* pFile;

	// copy file and create backup
	std::error_code ec;
	std::wstring bak_filepath;
	if (toBePatched)
		bak_filepath = filepath + L".unpatched.backup";
	else
		bak_filepath = filepath + L".patched.backup";

	auto copy_options = std::filesystem::copy_options::none;

	// backup file already exists
	if (std::filesystem::is_regular_file(bak_filepath))
	{
		// show window to ask user permission to overwrite existing file
		const std::wstring askOverwriteWStr = L"File " + bak_filepath + L" already exists, do you want to overwrite it?";
		LPCWSTR askOverwriteLPCWStr = askOverwriteWStr.c_str();
		const int overwriteYesNo = MessageBox(hwnd, askOverwriteLPCWStr, L"4GB Patcher", MB_YESNO);
		switch (overwriteYesNo)
		{
		case IDYES:
		{
			copy_options = std::filesystem::copy_options::none | std::filesystem::copy_options::overwrite_existing;
			break;
		}
		case IDNO:
		{
			return false;
		}
		}
	}

	// not enough memory? program will terminate
	if (!std::filesystem::copy_file(filepath, bak_filepath, copy_options, ec))
	{
		MessageBox(hwnd, L"Error occured creating backup file, aborting", L"4GB Patcher", MB_OK);
		return false;
	}
	
	// open file
	_wfopen_s(&pFile, filepath.c_str(), L"r+");
	if (!pFile)
	{
		catchOpenFileError(hwnd);
		return false;
	}

	// read first 533 bytes
	fseek(pFile, OFFSET_POSITION, SEEK_SET);

	// extract PE offset
	const unsigned int PE_offset = (unsigned int)fgetc(pFile);

	// point to laa flag position
	fseek(pFile, LAA_BYTE_POSITION, SEEK_SET);

	// create bitset of laa byte
	std::bitset<8>laa_bitset{ (unsigned int) fgetc(pFile) };

	// flip the 6th bit
	laa_bitset[5].flip();

	// point to laa flag position
	fseek(pFile, LAA_BYTE_POSITION, SEEK_SET);

	// overwrite laa byte
	fputc((unsigned int) laa_bitset.to_ulong(), pFile);

	// free mem
	fclose(pFile);

	return true;
}

bool detectPatchStatus(std::wstring filepath, HWND hwnd)
{
	FILE* pFile;

	// open file
	_wfopen_s(&pFile, filepath.c_str(), L"r+");
	if (!pFile)
	{
		catchOpenFileError(hwnd);
		return false;
	}

	// point to offset position
	fseek(pFile, OFFSET_POSITION, SEEK_SET);

	// get offset
	const unsigned int PE_offset = (unsigned int) fgetc(pFile);

	// point to laa flag position
	fseek(pFile, LAA_BYTE_POSITION, SEEK_SET);

	// create bitset of LAA byte
	std::bitset<8>laa_bitset{ (unsigned int) fgetc(pFile) };

	// free mem
	fclose(pFile);

	// check if LAA flag is set (6th bit) and return
	return laa_bitset[5] == true;
}
