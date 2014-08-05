
#include <Windows.h>
#include "CodeZap.h"

INT APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT iShowCmd)
{
#ifdef _DEBUG
	ZCode();
#endif
	CodeZap(TEXT("D:\\Project\\VDRIVE\\chs\\VDRIVE.CZ.EXE"), TEXT("D:\\Project\\VDRIVE\\chs\\VDRIVE.EXE"), ZCode);
	return TRUE;
}
