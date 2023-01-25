#pragma once

#ifndef UNICODE
#define UNICODE
#endif 

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

#define OUT_TEXT_APPSTATUS_NO_EXE "Choose file to view application status"
#define OUT_TEXT_APPSTATUS_PATCHED L"Application status:   PATCHED"
#define OUT_TEXT_APPSTATUS_UNPATCHED L"Application status: UNPATCHED"

#define BTN_TEXT_PATCH "Patch"
#define BTN_TEXT_UNPATCH "Unpatch"
#define BTN_TEXT_CLOSE "Close"
#define BTN_TEXT_CHOOSE "Choose file"

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
