#pragma once

#ifndef UNICODE
#define UNICODE
#endif 

// application meta data
#define APP_NAME L"4GB Patcher"

// window parameters
#define WINDOW_SIZE_X 420
#define WINDOW_SIZE_Y 175

// offset and characteristics byte position
#define LAA_BYTE_POSITION 256 + 4 + PE_offset + 18
#define OFFSET_POSITION 0x3c

// button handlers for parent window
#define IDC_BUTTON_CHOOSEFILE (100)
#define IDC_BUTTON_PATCH (101)
#define IDC_BUTTON_UNPATCH (102)
#define IDC_BUTTON_CLOSE (103)
#define IDC_TEXT_STATUS (104)
#define IDC_TEXT_PATH (105)

// button labels
#define BTN_TEXT_PATCH "Patch"
#define BTN_TEXT_UNPATCH "Unpatch"
#define BTN_TEXT_CLOSE "Close"
#define BTN_TEXT_CHOOSE "Choose file"

// texts the user will see
#define OUT_TEXT_APPSTATUS_NO_EXE L"Choose file to view application status"
#define OUT_TEXT_APPSTATUS_PATCHED L"Application status:   PATCHED"
#define OUT_TEXT_APPSTATUS_UNPATCHED L"Application status: UNPATCHED"

#define OUT_TEXT_MB_PATCHED L"Application successfully patched!"
#define OUT_TEXT_MB_UNPATCHED L"Application successfully unpatched!"

// postfixes for the backup file
#define POSTFIX_PATCHED L".patched.backup"
#define POSTFIX_UNPATCHED L".unpatched.backup"

// error messages with APP_ prefix to avoid confusion with constants in winerror.h
#define APP_ERROR_CREATING_BACKUP L"Error occured while creating backup file, aborting"
#define APP_ERROR_OPENFILE_ACCESS L"Access denied, is the program running or the file read-only?"
#define APP_ERROR_OPENFILE_FILETOOBIG L"Could not open file because it's too big"
#define APP_ERROR_OPENFILE_PATHTOOLONG L"Could not open file because file path is too long"
#define APP_ERROR_OPENFILE_UNKNOWN L"Unknown error occured"

/*

256 bytes MS-DOS stub
4 bytes signature "PE\0\0"
x bytes offset - defined in MS-DOS stub at 0x3c
20 bytes COFF file header - LAA in characteristics at offset 18 with size 2

= 278 + (0-255) bytes = 278 <-> 533 bytes

make copy of file
read 0x3c bytes
read 256 + 4 + offset - 0x3c bytes
flip 6th bit of laa_byte

make copy of file
open file
read first 533 bytes
extract offset
edit LAA flag
overwrite first 533 bytes

*/
