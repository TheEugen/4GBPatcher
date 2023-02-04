#pragma once

#include <iostream>
#include <filesystem>
#include <Windows.h>
#include <bitset>
#include <stdlib.h>
#include <errno.h>

#include "Resource.h"


static void catchOpenFileError(HWND hWnd)
{
	std::wstring errorMsg;
	switch (errno)
	{
		// access denied (e.g. .exe is running atm)
	case EACCES:
	{
		errorMsg = APP_ERROR_OPENFILE_ACCESS;
		break;
	}
	// file too big
	case EFBIG:
	{
		errorMsg = APP_ERROR_OPENFILE_FILETOOBIG;
		break;
	}
	// path too long
	case ENAMETOOLONG:
	{
		errorMsg = APP_ERROR_OPENFILE_PATHTOOLONG;
		break;
	}
	default:
	{
		errorMsg = APP_ERROR_OPENFILE_UNKNOWN;
	}
	}

	OutputDebugStringW(L"error opening file");
	MessageBox(hWnd, errorMsg.c_str(), APP_NAME, MB_OK);
}


static bool patchFile(std::wstring filepath, bool toBePatched, HWND hWnd)
{
	FILE* pFile;

	// copy file and create backup
	std::error_code ec;
	std::wstring bak_filepath;

	if (toBePatched)
		bak_filepath = filepath + POSTFIX_UNPATCHED;
	else
		bak_filepath = filepath + POSTFIX_PATCHED;

	auto copy_options = std::filesystem::copy_options::none;

	// check if backup file already exists
	if (std::filesystem::is_regular_file(bak_filepath))
	{
		// show window to ask user for permission to overwrite existing file
		const std::wstring askOverwriteWStr = L"File " + bak_filepath + L" already exists, do you want to overwrite it?";
		const int overwriteYesNo = MessageBox(hWnd, askOverwriteWStr.c_str(), APP_NAME, MB_YESNO);
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

	// create backup, if it fails -> not enough memory? program will terminate
	if (!std::filesystem::copy_file(filepath, bak_filepath, copy_options, ec))
	{
		MessageBox(hWnd, APP_ERROR_CREATING_BACKUP, APP_NAME, MB_OK);
		return false;
	}

	// open file
	_wfopen_s(&pFile, filepath.c_str(), L"r+");
	if (!pFile)
	{
		catchOpenFileError(hWnd);
		return false;
	}

	// read first 533 bytes
	fseek(pFile, OFFSET_POSITION, SEEK_SET);

	// extract PE offset
	const unsigned int PE_offset = (unsigned int)fgetc(pFile);

	// point to laa flag position
	fseek(pFile, LAA_BYTE_POSITION, SEEK_SET);

	// create bitset of laa byte
	std::bitset<8>laa_bitset{ (unsigned int)fgetc(pFile) };

	// flip the 6th bit
	laa_bitset[5].flip();

	// point to laa flag position
	fseek(pFile, LAA_BYTE_POSITION, SEEK_SET);

	// overwrite laa byte
	fputc((unsigned int)laa_bitset.to_ulong(), pFile);

	// free mem
	fclose(pFile);

	return true;
}

// 0 -> error opening file
// 1 -> app status patched
// 2 -> app status unpatched
static unsigned int detectPatchStatus(std::wstring filepath, HWND hWnd)
{
	FILE* pFile;

	// open file
	_wfopen_s(&pFile, filepath.c_str(), L"r");
	if (!pFile)
	{
		catchOpenFileError(hWnd);
		return 0;
	}

	// point to offset position
	fseek(pFile, OFFSET_POSITION, SEEK_SET);

	// get offset
	const unsigned int PE_offset = (unsigned int)fgetc(pFile);

	// point to laa flag position
	fseek(pFile, LAA_BYTE_POSITION, SEEK_SET);

	// create bitset of LAA byte
	std::bitset<8>laa_bitset{ (unsigned int)fgetc(pFile) };

	// free mem
	fclose(pFile);

	// check if LAA flag is set (6th bit) and return
	if (laa_bitset[5] == true)
		return 1;

	return 2;
}

static void toggleButtons(HWND hWnd, bool patchStatus)
{
	// find handle to patch button
	HWND hWndPatchBtn = GetDlgItem(hWnd, IDC_BUTTON_PATCH);

	// find handle to unpatch button
	HWND hWndUnpatchBtn = GetDlgItem(hWnd, IDC_BUTTON_UNPATCH);

	// toggle patch button
	EnableWindow(hWndPatchBtn, !patchStatus);

	// toggle unpatch button
	EnableWindow(hWndUnpatchBtn, patchStatus);
}