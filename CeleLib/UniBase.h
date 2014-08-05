

/**********************************************************************************************************************/
/* Uniform Base Library 2.0.208 WIN32 */
/* Copyright (C) Yonsm 2006-2010, All Rights Reserved.*/
/* Resource Switch: -n */
/* Compiler Switch: /Ob2 */
/* Global Variable: g_hInst */
/* Global Variable: g_ptzAppName */
#pragma once

#ifdef _DEBUG
#define _TRACE
//#define _TRACE_TIME
//#define _TRACE_TO_FILE
//#define _TRACE_TO_CONSOLE
#endif

#ifdef WINCE
//#define _WINSTR
#else
//#define _SHLSTR
#endif

#if defined(_UNICODE) && !defined(UNICODE)
#define UNICODE
#endif

#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_NON_CONFORMING_SWPRINTFS

#include <TChar.h>
#include <StdIO.h>
#include <StdLib.h>
#include <String.h>
#include <Memory.h>
#include <Windows.h>
#include <ShLwApi.h>
/**********************************************************************************************************************/



/**********************************************************************************************************************/
/* Define */
#ifndef CONST
#define CONST						const
#endif

#ifndef STATIC
#define STATIC						static
#endif

#ifndef INLINE
#define INLINE						__inline
#endif

#ifndef FINLINE
#define FINLINE						__forceinline
#endif

#ifndef ISTATIC
#define ISTATIC						__forceinline static
#endif

#define UCALL						WINAPI
#define UCALLBACK					CALLBACK
#define UAPI(x)						INLINE x UCALL

#ifdef __cplusplus
#define UDEF(x)						= x
#else
#define UDEF(x)
#endif
/**********************************************************************************************************************/



/**********************************************************************************************************************/
/* Type */
#define VOID						void
typedef void						*PVOID;
typedef void						*HANDLE;
typedef HANDLE						*PHANDLE;
typedef int							BOOL, *PBOOL;
typedef float						FLOAT, *PFLOAT;
typedef double						DOUBLE, *PDOUBLE;

typedef int							INT, *PINT;
typedef signed char					INT8, *PINT8;
typedef signed short				INT16, *PINT16;
typedef signed int					INT32, *PINT32;
typedef signed __int64				INT64, *PINT64;

typedef unsigned int				UINT, *PUINT;
typedef unsigned char				UINT8, *PUINT8;
typedef unsigned short				UINT16, *PUINT16;
typedef unsigned int				UINT32, *PUINT32;
typedef unsigned __int64			UINT64, *PUINT64;

typedef unsigned char				BYTE, *PBYTE;
typedef unsigned short				WORD, *PWORD;
typedef unsigned long				DWORD, *PDWORD;
typedef unsigned __int64			QWORD, *PQWORD;

#if defined(_WIN64)
typedef __int64						INT_PTR, *PINT_PTR;
typedef unsigned __int64			UINT_PTR, *PUINT_PTR;
#else
#if (_MSC_VER < 1300)
typedef long						INT_PTR, *PINT_PTR;
typedef unsigned long				UINT_PTR, *PUINT_PTR;
#else
typedef int							INT_PTR, *PINT_PTR;
typedef unsigned int				UINT_PTR, *PUINT_PTR;
#endif
#endif

#ifndef _WCHAR_T_DEFINED
#define _WCHAR_T_DEFINED
typedef unsigned short				wchar_t;
#endif

typedef char						CHAR, *PCHAR;
typedef char						ACHAR, *PACHAR;
typedef wchar_t						WCHAR, *PWCHAR;
#ifdef _UNICODE
typedef WCHAR						TCHAR, *PTCHAR;
typedef WORD						UTCHAR, *PUTCHAR;
#else
typedef ACHAR						TCHAR, *PTCHAR;
typedef BYTE						UTCHAR, *PUTCHAR;
#endif

typedef CHAR						*PSTR;
typedef ACHAR						*PASTR;
typedef WCHAR						*PWSTR;
typedef TCHAR						*PTSTR;
typedef CONST ACHAR					*PCSTR;
typedef CONST ACHAR					*PCASTR;
typedef CONST WCHAR					*PCWSTR;
typedef CONST TCHAR					*PCTSTR;

typedef CONST VOID					*PCVOID;
typedef CONST BYTE					*PCBYTE;

#ifndef VALIST
#define VALIST						va_list
#endif

#ifndef TEXT
#ifdef _UNICODE
#define TEXT(t)						L##t
#else
#define TEXT(t)						t
#endif
#endif
#define TSTR(t)						TEXT(t)
/**********************************************************************************************************************/



/**********************************************************************************************************************/
/* Const */
#ifndef TRUE
#define TRUE						1
#endif

#ifndef FALSE
#define FALSE						0
#endif

#ifndef NULL
#define NULL						0
#endif

#ifndef MAX_STR
#define MAX_STR						1024
#endif

#ifndef MAX_PATH
#define MAX_PATH					260
#endif

#ifndef MAX_NAME
#define MAX_NAME					80
#endif

#define UFILE_READ					GENERIC_READ
#define UFILE_WRITE					GENERIC_WRITE
#define UFILE_READWRITE				(GENERIC_READ | GENERIC_WRITE)

#define UFILE_BEGIN					FILE_BEGIN
#define UFILE_CURRENT				FILE_CURRENT
#define UFILE_END					FILE_END

#define UCP_ANSI					CP_ACP
#define UCP_OEM						CP_OEMCP
#define UCP_MAC						CP_MACCP
#define UCP_SYMBOL					CP_SYMBOL
#define UCP_UTF7					CP_UTF7
#define UCP_UTF8					CP_UTF8
/**********************************************************************************************************************/



/**********************************************************************************************************************/
/* Macro */
#define _NumOf(s)					(sizeof(s) / sizeof(s[0]))
#define _Zero(p)					UMemSet(p, 0, sizeof(*p))

#define _SafeFree(p)				if (p) {UMemFree(p); p = NULL;}
#define _SafeDelete(p)				if (p) {delete p; p = NULL;}
#define _SafeRelease(p)				if (p) {(p)->Release(); p = NULL;}

#define _RectWidth(r)				((r).right - (r).left)
#define _RectHeight(r)				((r).bottom - (r).top)

#define _IsIntRes(r)				((((UINT_PTR) (r)) >> 16) == 0)
#define _MakeIntRes(i)				((PTSTR) ((DWORD) (i)))

#define _DibStride(w, i)			(((((w) * i) + 31) & ~31) / 8)
#define _DibSize(w, i, h)			(_DibStride((w), i) * (h))
#define _DibBits(p, w, i, x, y)		((p) + _DibStride((w), (i)) * (y) + (x) * 3)
#define _DibStride24(w)				(((w) + (w) + (w) + 3) & 0xFFFFFFFC)
#define _DibSize24(w, h)			(_DibStride24(w) * (h))
#define _DibBits24(p, w, x, y)		((p) + _DibStride24(w) * (y) + (x) * 3)
#define _DibStride32(w)				((w) * 4)
#define _DibSize32(w, h)			(_DibStride32(w) * (h))
#define _DibBits32(p, w, x, y)		((p) + _DibStride32(w) * (y) + (x) * 4)
/**********************************************************************************************************************/



/**********************************************************************************************************************/
/* Global */
__if_not_exists (g_hInst) {__declspec(selectany) HINSTANCE g_hInst;}
__if_not_exists (g_ptzAppName) {__declspec(selectany) PCTSTR g_ptzAppName = NULL;}
/**********************************************************************************************************************/



/**********************************************************************************************************************/
/* Memory */
UAPI(PVOID) UMemAlloc(UINT nSize)
{
	return malloc(nSize);
}

UAPI(PVOID) UMemRealloc(PVOID pvMem, UINT nSize)
{
	return realloc(pvMem, nSize);
}

UAPI(VOID) UMemFree(PVOID pvMem)
{
	free(pvMem);
}

UAPI(PVOID) UMemAlignAlloc(UINT nSize, UINT16 uAlign UDEF(16))
{
	PVOID pvAlloc = UMemAlloc(nSize + sizeof(UINT16) + uAlign);
	if (pvAlloc)
	{
		UINT_PTR uMem = (UINT_PTR) pvAlloc + sizeof(UINT16);
		uMem = (uMem + (uAlign - 1)) & ~(uAlign - 1);
		((PUINT16) uMem)[-1] = (UINT16) (uMem - (UINT_PTR) pvAlloc);
		return (PVOID) uMem;
	}
	return NULL;
}

UAPI(VOID) UMemAlignFree(PVOID pvMem)
{
	if (pvMem)
	{
		pvMem = (PVOID) ((UINT_PTR) pvMem - ((PUINT16) pvMem)[-1]);
		UMemFree(pvMem);
	}
}

UAPI(PVOID) UMemSet(PVOID pvMem, ACHAR cVal, UINT nSize)
{
	return memset(pvMem, cVal, nSize);
}

UAPI(PVOID) UMemCopy(PVOID pvDst, PCVOID pvSrc, UINT nSize)
{
	return memcpy(pvDst, pvSrc, nSize);
}

UAPI(PVOID) UMemMove(PVOID pvDst, PCVOID pvSrc, UINT nSize)
{
	return memmove(pvDst, pvSrc, nSize);
}

UAPI(INT) UMemCmp(PCVOID pvMem1, PCVOID pvMem2, UINT nSize)
{
	return memcmp(pvMem1, pvMem2, nSize);
}

UAPI(INT) UMemCmpI(PCVOID pvMem1, PCVOID pvMem2, UINT nSize)
{
	return _memicmp(pvMem1, pvMem2, nSize);
}
/**********************************************************************************************************************/



/**********************************************************************************************************************/
/* ASCII Char */
UAPI(BOOL) AChrIsNum(ACHAR a)
{
	return (a >= '0') && (a <= '9');
}

UAPI(BOOL) AChrIsAlpha(ACHAR a)
{
	return (a >= 'A') && (a <= 'Z') || (a >= 'a') && (a <= 'z');
}

UAPI(BOOL) AChrIsSymbol(ACHAR a)
{
	return (a > ' ') && (a < '0') || (a > '9') && (a < 'A') || (a > 'Z') && (a < 'a') || (a > 'z') && (a < 127);
}

UAPI(BOOL) AChrIsPrintable(ACHAR a)
{
	return ((a >= ' ') && (a <= '~')) || (a == '\r') || (a == '\n') || (a == '\t');
}

UAPI(ACHAR) AChrToLower(ACHAR a)
{
	return ((a >= 'A') && (a <= 'Z')) ? (a - 'A' + 'a') : a;
}

UAPI(ACHAR) AChrToUpper(ACHAR a)
{
	return ((a >= 'a') && (a <= 'z')) ? (a + 'A' - 'a') : a;
}

UAPI(BYTE) AChrToHex(CONST ACHAR a[2])
{
	CONST STATIC BYTE c_bHexVal[128] =
	{
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
	};
	return (c_bHexVal[a[0]] << 4) | c_bHexVal[a[1]];
}

UAPI(VOID) AChrFromHex(ACHAR a[2], BYTE b)
{
	CONST STATIC ACHAR c_azHexChr[] = "0123456789ABCDEF";
	a[0] = c_azHexChr[b >> 4];
	a[1] = c_azHexChr[b & 0x0F];
}

UAPI(BOOL) AChrEqualI(ACHAR a1, ACHAR a2)
{
	return ((a1 == a2) || (AChrToUpper(a1) == AChrToUpper(a2)));
}
/**********************************************************************************************************************/



/**********************************************************************************************************************/
/* UNICODE Char */
UAPI(BOOL) WChrIsNum(WCHAR w)
{
	return (w >= '0') && (w <= '9');
}

UAPI(BOOL) WChrIsAlpha(WCHAR w)
{
	return (w >= 'A') && (w <= 'Z') || (w >= 'a') && (w <= 'z');
}

UAPI(BOOL) WChrIsSymbol(WCHAR w)
{
	return (w > ' ') && (w < '0') || (w > '9') && (w < 'A') || (w > 'Z') && (w < 'a') || (w > 'z') && (w < 127);
}

UAPI(BOOL) WChrIsPrintable(WCHAR w)
{
	return ((w >= ' ') && (w <= '~')) || (w == '\r') || (w == '\n') || (w == '\t');
}

UAPI(WCHAR) WChrToLower(WCHAR w)
{
	return ((w >= 'A') && (w <= 'Z')) ? (w - 'A' + 'a') : w;
}

UAPI(WCHAR) WChrToUpper(WCHAR w)
{
	return ((w >= 'a') && (w <= 'z')) ? (w + 'A' - 'a') : w;
}

UAPI(BYTE) WChrToHex(CONST WCHAR w[2])
{
	CONST STATIC BYTE c_bHexVal[128] =
	{
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	};
	return (c_bHexVal[w[0]] << 4) | c_bHexVal[w[1]];
}

UAPI(VOID) WChrFromHex(WCHAR w[2], BYTE b)
{
	CONST STATIC WCHAR c_wzHexChr[] = L"0123456789ABCDEF";
	w[0] = c_wzHexChr[b >> 4];
	w[1] = c_wzHexChr[b & 0x0F];
}

UAPI(BOOL) WChrEqualI(WCHAR w1, WCHAR w2)
{
	return (w1 == w2) || (WChrToUpper(w1) == WChrToUpper(w2));
}
/**********************************************************************************************************************/



/**********************************************************************************************************************/
/* String Conversion */
UAPI(UINT) AStrToWStr(PWSTR pwzDst, PCASTR pazSrc, UINT nDstLen UDEF(MAX_STR), UINT nSrcLen UDEF(-1), UINT uCodePage UDEF(UCP_ANSI))
{
	return MultiByteToWideChar(uCodePage, 0, pazSrc, nSrcLen, pwzDst, nDstLen);
}

UAPI(UINT) WStrToAStr(PASTR pazDst, PCWSTR pwzSrc, UINT nDstLen UDEF(MAX_STR), UINT nSrcLen UDEF(-1), UINT uCodePage UDEF(UCP_ANSI))
{
	return WideCharToMultiByte(uCodePage, 0, pwzSrc, nSrcLen, pazDst, nDstLen, NULL, NULL);
}

UAPI(UINT) UTF8ToWStr(PWSTR pwzDst, PCASTR puzSrc, UINT nDstLen UDEF(MAX_STR), UINT nSrcLen UDEF(-1))
{
	return AStrToWStr(pwzDst, (PCASTR) puzSrc, nDstLen, nSrcLen, UCP_UTF8);
}

UAPI(UINT) UTF8ToAStr(PASTR pazDst, PCASTR puzSrc, UINT nDstLen UDEF(MAX_STR), UINT nSrcLen UDEF(-1))
{
	WCHAR wzTemp[MAX_STR];
	nSrcLen = UTF8ToWStr(wzTemp, puzSrc, MAX_STR, nSrcLen);
	return WStrToAStr(pazDst, wzTemp, nDstLen, nSrcLen);
}

UAPI(UINT) WStrToUTF8(PASTR puzDst, PCWSTR pwzSrc, UINT nDstLen UDEF(MAX_STR), UINT nSrcLen UDEF(-1))
{
	return WStrToAStr(puzDst, pwzSrc, nDstLen, nSrcLen, UCP_UTF8);
}

UAPI(UINT) AStrToUTF8(PASTR puzDst, PASTR pazSrc, UINT nDstLen UDEF(MAX_STR), UINT nSrcLen UDEF(-1))
{
	WCHAR wzTemp[MAX_STR];
	nSrcLen = AStrToWStr(wzTemp, pazSrc, MAX_STR, nSrcLen);
	return WStrToUTF8(puzDst, wzTemp, nDstLen, nSrcLen);
}

UAPI(BOOL) AStrIsUTF8XML(PASTR pazStr)
{
	if ((pazStr[0] == '<') && (pazStr[1] == '?') && (pazStr[2] == 'x') && (pazStr[3] == 'm') && (pazStr[4] == 'l'))
	{
		for (pazStr += 5; (*pazStr != 0) && (*pazStr != '\n'); pazStr++)
		{
			if (((pazStr[0] == 'u') || (pazStr[0] == 'U')) && 
				((pazStr[1] == 't') || (pazStr[1] == 'T')) && 
				((pazStr[2] == 'f') || (pazStr[2] == 'F')) && 
				(pazStr[3] == '-') && 
				(pazStr[4] == '8'))
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}
/**********************************************************************************************************************/



/**********************************************************************************************************************/
/* ASCII String */
#ifdef _WINSTR
#define AStrFormat wsprintfA
#define AStrFormatV wvsprintfA
#else
#define AStrFormat sprintf
#define AStrFormatV vsprintf
#endif

UAPI(PASTR) AStrEnd(PCASTR pazStr)
{
	while (*pazStr)
	{
		pazStr++;
	}
	return (PASTR) pazStr;
}

UAPI(UINT) AStrLen(PCASTR pazStr)
{
	return (UINT) (AStrEnd(pazStr) - pazStr);
}

UAPI(UINT) AStrCopy(PASTR pazDst, PCASTR pazSrc)
{
	PASTR p = pazDst;
	while (*p++ = *pazSrc++);
	return (UINT) (p - pazDst);
}

UAPI(UINT) AStrCopyN(PASTR pazDst, PCASTR pazSrc, UINT nDstLen)
{
	PASTR p = pazDst;
	while (*pazSrc && ((INT) (--nDstLen) > 0))
	{
		*p++ = *pazSrc++;
	}
	*p++ = 0;
	return (UINT) (p - pazDst);
}

UAPI(UINT) AStrCat(PASTR pazDst, PCASTR pazSrc)
{
	PASTR p = AStrEnd(pazDst);
	return (UINT) (p - pazDst) + AStrCopy(p, pazSrc);
}

UAPI(INT) AStrCmp(PCASTR pazStr1, PCASTR pazStr2)
{
#ifdef _WINSTR
	return lstrcmpA(pazStr1, pazStr2);
#else
	return strcmp(pazStr1, pazStr2);
#endif
}

UAPI(INT) AStrCmpI(PCASTR pazStr1, PCASTR pazStr2)
{
#ifdef _WINSTR
	return lstrcmpiA(pazStr1, pazStr2);
#else
	return _stricmp(pazStr1, pazStr2);
#endif
}

UAPI(INT) AStrCmpN(PCASTR pazStr1, PCASTR pazStr2, UINT nLen)
{
#ifdef _SHLSTR
	return StrCmpNA(pazStr1, pazStr2, nLen);
#else
	return strncmp(pazStr1, pazStr2, nLen);
#endif
}

UAPI(INT) AStrCmpNI(PCASTR pazStr1, PCASTR pazStr2, UINT nLen)
{
#ifdef _SHLSTR
	return StrCmpNIA(pazStr1, pazStr2, nLen);
#else
	return _strnicmp(pazStr1, pazStr2, nLen);
#endif
}

UAPI(PASTR) AStrChr(PCASTR pazStr, ACHAR cChr)
{
#ifdef _SHLSTR
	return (PASTR) StrChrA(pazStr, cChr);
#else
	return (PASTR) strchr(pazStr, cChr);
#endif
}

UAPI(PASTR) AStrRChr(PCASTR pazStr, ACHAR cChr)
{
#ifdef _SHLSTR
	return (PASTR) StrRChrA(pazStr, NULL, cChr);
#else
	return (PASTR) strrchr(pazStr, cChr);
#endif
}

UAPI(PASTR) AStrStr(PCASTR pazStr1, PCASTR pazStr2)
{
#ifdef _SHLSTR
	return (PASTR) StrStrA(pazStr1, pazStr2);
#else
	return (PASTR) strstr(pazStr1, pazStr2);
#endif
}

UAPI(PASTR) AStrStrI(PCASTR pazStr1, PCASTR pazStr2)
{
#ifdef _SHLSTR
	return StrStrIA(pazStr1, pazStr2);
#else
	PASTR p = (PASTR) pazStr1;
	while (*p)
	{
		PASTR s1 = p;
		PASTR s2 = (PASTR) pazStr2;

		while (*s1 && *s2 && AChrEqualI(*s1, *s2))
		{
			s1++;
			s2++;
		}

		if (*s2 == 0)
		{
			return p;
		}

		p++;
	}
	return NULL;
#endif
}

UAPI(PASTR) AStrRep(PASTR pazStr, ACHAR cFind UDEF('|'), ACHAR cRep UDEF(0))
{
	PASTR p = pazStr;
	for (; *p; p++)
	{
		if (*p == cFind)
		{
			*p = cRep;
		}
	}
	return pazStr;
}

UAPI(PASTR) AStrTrim(PASTR pazStr, ACHAR cTrim UDEF('"'))
{
	if (*pazStr == cTrim)
	{
		PASTR p = pazStr + AStrLen(pazStr) - 1;
		if (*p == cTrim)
		{
			*p = 0;
		}
		return pazStr + 1;
	}
	return pazStr;
}

UAPI(PASTR) AStrSplit(PASTR pazStr, ACHAR cSplit)
{
	while (*pazStr)
	{
		if (*pazStr == cSplit)
		{
			*pazStr++ = 0;
			break;
		}
		pazStr++;
	}
	return pazStr;
}

UAPI(PASTR) AStrRSplit(PASTR pazStr, ACHAR cSplit)
{
	PASTR p;
	PASTR pazEnd = AStrEnd(pazStr);
	for (p = pazEnd; p >= pazStr; p--)
	{
		if (*p == cSplit)
		{
			*p++ = 0;
			return p;
		}
	}
	return pazEnd;
}

UAPI(UINT) AStrEqual(PCASTR pazStr1, PCASTR pazStr2)
{
	UINT i = 0;
	while (pazStr1[i] && (pazStr1[i] == pazStr2[i]))
	{
		i++;
	}
	return i;
}

UAPI(UINT) AStrEqualI(PCASTR pazStr1, PCASTR pazStr2)
{
	UINT i = 0;
	while (pazStr1[i] && WChrEqualI(pazStr1[i], pazStr2[i]))
	{
		i++;
	}
	return i;
}

UAPI(BOOL) AStrMatch(PCASTR pazStr, PCASTR pazPat)
{
	PCASTR s, p;
	BOOL bStar = FALSE;

__LoopStart:
	for (s = pazStr, p = pazPat; *s; s++, p++)
	{
		switch (*p)
		{
		case '?':
			/*if (*s == '.') goto __StartCheck;*/
			break;

		case '*':
			bStar = TRUE;
			pazStr = s, pazPat = p;
			if (!*++pazPat) return TRUE;
			goto __LoopStart;

		default:
			if (*s != *p)
			{
				/*__StartCheck:*/
				if (!bStar) return FALSE;
				pazStr++;
				goto __LoopStart;
			}
			break;
		}
	}
	if (*p == '*') ++p;
	return (!*p);
}

UAPI(BOOL) AStrMatchI(PCASTR pazStr, PCASTR pazPat)
{
	PCASTR s, p;
	BOOL bStar = FALSE;

__LoopStart:
	for (s = pazStr, p = pazPat; *s; s++, p++)
	{
		switch (*p)
		{
		case '?':
			/*if (*s == '.') goto __StartCheck;*/
			break;

		case '*':
			bStar = TRUE;
			pazStr = s, pazPat = p;
			if (!*++pazPat) return TRUE;
			goto __LoopStart;

		default:
			if (!AChrEqualI(*s, *p))
			{
				/*__StartCheck:*/
				if (!bStar) return FALSE;
				pazStr++;
				goto __LoopStart;
			}
			break;
		}
	}
	if (*p == '*') ++p;
	return (!*p);
}

UAPI(PASTR) AStrToUpper(PASTR pazStr)
{
#ifdef _WINSTR
	return CharUpperA(pazStr);
#else
	return _strupr(pazStr);
#endif
}

UAPI(PASTR) AStrToLower(PASTR pazStr)
{
#ifdef _WINSTR
	return CharLowerA(pazStr);
#else
	return _strlwr(pazStr);
#endif
}

UAPI(INT) AStrToInt(PCASTR pazStr)
{
#ifdef _SHLSTR
	INT i = 0;
	StrToIntExA(pazStr, STIF_SUPPORT_HEX, &i);
	return i;
#else
	return atoi(pazStr);
#endif
}

UAPI(INT64) AStrToInt64(PCASTR pazStr)
{
	return _atoi64(pazStr);
}

UAPI(DOUBLE) AStrToDouble(PCASTR pazStr)
{
	return atof(pazStr);
}

UAPI(PASTR) AStrFromInt(PASTR pazDst, INT iVal, INT iRadix UDEF(10))
{
	return _itoa(iVal, pazDst, iRadix);
}

UAPI(UINT) AStrFromDouble(PASTR pazDst, DOUBLE dVal)
{
	return AStrFormat(pazDst, "%f", dVal);
}

UAPI(UINT) AStrToHex(PBYTE pbDst, PCASTR pazSrc)
{
	PBYTE pbStart = pbDst;
	while (*pazSrc)
	{
		*pbDst++ = AChrToHex(pazSrc);
		pazSrc += 2;
	}
	return (UINT) (pbDst - pbStart);
}

UAPI(UINT) AStrFromHex(PASTR pazDst, PCBYTE pbSrc, UINT nSize)
{
	UINT i;
	for (i = 0; i < nSize; i++)
	{
		AChrFromHex(&pazDst[i * 2], pbSrc[i]);
	}
	pazDst[nSize * 2] = 0;
	return nSize * 2;
}

UAPI(UINT) AStrLoad(UINT uID, PASTR pazStr, UINT nMax UDEF(MAX_STR))
{
	return LoadStringA(g_hInst, uID, pazStr, nMax);
}

UAPI(PCASTR) AStrGet(UINT uID)
{
	STATIC ACHAR s_azStr[MAX_STR];
	AStrLoad(uID, s_azStr, MAX_STR);
	return s_azStr;
}
/**********************************************************************************************************************/



/**********************************************************************************************************************/
/* UNICODE String */
#ifdef _WINSTR
#define WStrFormat wsprintfW
#define WStrFormatV wvsprintfW
#else
#ifdef WINCE
#define WStrFormat swprintf
#define WStrFormatV vswprintf
#else
#define WStrFormat _swprintf
#define WStrFormatV _vswprintf
#endif
#endif

UAPI(PWSTR) WStrEnd(PCWSTR pwzStr)
{
	while (*pwzStr)
	{
		pwzStr++;
	}
	return (PWSTR) pwzStr;
}

UAPI(UINT) WStrLen(PCWSTR pwzStr)
{
	return (UINT) (WStrEnd(pwzStr) - pwzStr);
}

UAPI(UINT) WStrCopy(PWSTR pwzDst, PCWSTR pwzSrc)
{
	PWSTR p = pwzDst;
	while (*p++ = *pwzSrc++);
	return (UINT) (p - pwzDst);
}

UAPI(UINT) WStrCopyN(PWSTR pwzDst, PCWSTR pwzSrc, UINT nDstLen)
{
	PWSTR p = pwzDst;
	while (*pwzSrc && ((INT) (--nDstLen) > 0))
	{
		*p++ = *pwzSrc++;
	}
	*p++ = 0;
	return (UINT) (p - pwzDst);
}

UAPI(UINT) WStrCat(PWSTR pwzDst, PCWSTR pwzSrc)
{
	PWSTR p = WStrEnd(pwzDst);
	return (UINT) (p - pwzDst) + WStrCopy(p, pwzSrc);
}

UAPI(INT) WStrCmp(PCWSTR pwzStr1, PCWSTR pwzStr2)
{
#ifdef _WINSTR
	return lstrcmpW(pwzStr1, pwzStr2);
#else
	return wcscmp(pwzStr1, pwzStr2);
#endif
}

UAPI(INT) WStrCmpI(PCWSTR pwzStr1, PCWSTR pwzStr2)
{
#ifdef _WINSTR
	return lstrcmpiW(pwzStr1, pwzStr2);
#else
	return _wcsicmp(pwzStr1, pwzStr2);
#endif
}

UAPI(INT) WStrCmpN(PCWSTR pwzStr1, PCWSTR pwzStr2, UINT nLen)
{
#ifdef _SHLSTR
	return StrCmpNW(pwzStr1, pwzStr2, nLen);
#else
	return wcsncmp(pwzStr1, pwzStr2, nLen);
#endif
}

UAPI(INT) WStrCmpNI(PCWSTR pwzStr1, PCWSTR pwzStr2, UINT nLen)
{
#ifdef _SHLSTR
	return StrCmpNIW(pwzStr1, pwzStr2, nLen);
#else
	return _wcsnicmp(pwzStr1, pwzStr2, nLen);
#endif
}

UAPI(PWSTR) WStrChr(PCWSTR pwzStr, WCHAR wChr)
{
#ifdef _SHLSTR
	return (PWSTR) StrChrW(pwzStr, wChr);
#else
	return (PWSTR) wcschr(pwzStr, wChr);
#endif
}

UAPI(PWSTR) WStrRChr(PCWSTR pwzStr, WCHAR wChr)
{
#ifdef _SHLSTR
	return (PWSTR) StrRChrW(pwzStr, NULL, wChr);
#else
	return (PWSTR) wcsrchr(pwzStr, wChr);
#endif
}

UAPI(PWSTR) WStrStr(PCWSTR pwzStr1, PCWSTR pwzStr2)
{
#ifdef _SHLSTR
	return (PWSTR) StrStrW(pwzStr1, pwzStr2);
#else
	return (PWSTR) wcsstr(pwzStr1, pwzStr2);
#endif
}

UAPI(PWSTR) WStrStrI(PCWSTR pwzStr1, PCWSTR pwzStr2)
{
#ifdef _SHLSTR
	return (PWSTR) StrStrIW(pwzStr1, pwzStr2);
#else
	PWSTR p = (PWSTR) pwzStr1;
	while (*p)
	{
		PWSTR s1 = p;
		PWSTR s2 = (PWSTR) pwzStr2;

		while (*s1 && *s2 && WChrEqualI((TCHAR) *s1, (TCHAR) *s2))
		{
			s1++;
			s2++;
		}

		if (*s2 == 0)
		{
			return p;
		}

		p++;
	}
	return NULL;
#endif
}

UAPI(PWSTR) WStrRep(PWSTR pwzStr, WCHAR wFind UDEF('|'), WCHAR wRep UDEF(0))
{
	PWSTR p = pwzStr;
	for (; *p; p++)
	{
		if (*p == wFind)
		{
			*p = wRep;
		}
	}
	return pwzStr;
}

UAPI(PWSTR) WStrTrim(PWSTR pwzStr, WCHAR wTrim UDEF('"'))
{
	if (*pwzStr == wTrim)
	{
		PWSTR p = pwzStr + WStrLen(pwzStr) - 1;
		if (*p == wTrim)
		{
			*p = 0;
		}
		return pwzStr + 1;
	}
	return pwzStr;
}

UAPI(PWSTR) WStrSplit(PWSTR pwzStr, WCHAR wSplit)
{
	while (*pwzStr)
	{
		if (*pwzStr == wSplit)
		{
			*pwzStr++ = 0;
			break;
		}
		pwzStr++;
	}
	return pwzStr;
}

UAPI(PWSTR) WStrRSplit(PWSTR pwzStr, WCHAR wSplit)
{
	PWSTR p;
	PWSTR pwzEnd = WStrEnd(pwzStr);
	for (p = pwzEnd; p >= pwzStr; p--)
	{
		if (*p == wSplit)
		{
			*p++ = 0;
			return p;
		}
	}
	return pwzEnd;
}

UAPI(UINT) WStrEqual(PCWSTR pwzStr1, PCWSTR pwzStr2)
{
	UINT i = 0;
	while (pwzStr1[i] && (pwzStr1[i] == pwzStr2[i]))
	{
		i++;
	}
	return i;
}

UAPI(UINT) WStrEqualI(PCWSTR pwzStr1, PCWSTR pwzStr2)
{
	UINT i = 0;
	while (pwzStr1[i] && WChrEqualI(pwzStr1[i], pwzStr2[i]))
	{
		i++;
	}
	return i;
}

UAPI(BOOL) WStrMatch(PCWSTR pwzStr, PCWSTR pwzPat)
{
	PCWSTR s, p;
	BOOL bStar = FALSE;

__LoopStart:
	for (s = pwzStr, p = pwzPat; *s; s++, p++)
	{
		switch (*p)
		{
		case '?':
			/*if (*s == '.') goto __StartCheck;*/
			break;

		case '*':
			bStar = TRUE;
			pwzStr = s, pwzPat = p;
			if (!*++pwzPat) return TRUE;
			goto __LoopStart;

		default:
			if (*s != *p)
			{
				/*__StartCheck:*/
				if (!bStar) return FALSE;
				pwzStr++;
				goto __LoopStart;
			}
			break;
		}
	}
	if (*p == '*') ++p;
	return (!*p);
}

UAPI(BOOL) WStrMatchI(PCWSTR pwzStr, PCWSTR pwzPat)
{
	PCWSTR s, p;
	BOOL bStar = FALSE;

__LoopStart:
	for (s = pwzStr, p = pwzPat; *s; s++, p++)
	{
		switch (*p)
		{
		case '?':
			/*if (*s == '.') goto __StartCheck;*/
			break;

		case '*':
			bStar = TRUE;
			pwzStr = s, pwzPat = p;
			if (!*++pwzPat) return TRUE;
			goto __LoopStart;

		default:
			if (!WChrEqualI(*s, *p))
			{
				/*__StartCheck:*/
				if (!bStar) return FALSE;
				pwzStr++;
				goto __LoopStart;
			}
			break;
		}
	}
	if (*p == '*') ++p;
	return (!*p);
}

UAPI(PWSTR) WStrToUpper(PWSTR pwzStr)
{
#ifdef _WINSTR
	return CharUpperW(pwzStr);
#else
	return _wcsupr(pwzStr);
#endif
}

UAPI(PWSTR) WStrToLower(PWSTR pwzStr)
{
#ifdef _WINSTR
	return CharLowerW(pwzStr);
#else
	return _wcslwr(pwzStr);
#endif
}

UAPI(INT) WStrToInt(PCWSTR pwzStr)
{
#ifdef _SHLSTR
	INT i = 0;
	StrToIntExW(pwzStr, STIF_SUPPORT_HEX, &i);
	return i;
#else
	return _wtoi(pwzStr);
#endif
}

UAPI(INT64) WStrToInt64(PCWSTR pwzStr)
{
	return _wtoi64(pwzStr);
}

UAPI(DOUBLE) WStrToDouble(PCWSTR pwzStr)
{
	return wcstod(pwzStr, NULL);
}

UAPI(PWSTR) WStrFromInt(PWSTR pwzDst, INT iVal, INT iRadix UDEF(10))
{
	return _itow(iVal, pwzDst, 10);
}

UAPI(UINT) WStrFromDouble(PWSTR pwzDst, DOUBLE dVal)
{
	return WStrFormat(pwzDst, L"%f", dVal);
}

UAPI(UINT) WStrToHex(PBYTE pbDst, PCWSTR pwzSrc)
{
	PBYTE pbStart = pbDst;
	while (*pwzSrc)
	{
		*pbDst++ = WChrToHex(pwzSrc);
		pwzSrc += 2;
	}
	return (UINT) (pbDst - pbStart);
}

UAPI(UINT) WStrFromHex(PWSTR pwzDst, PCBYTE pbSrc, UINT nSize)
{
	UINT i;
	for (i = 0; i < nSize; i++)
	{
		WChrFromHex(&pwzDst[i * 2], pbSrc[i]);
	}
	pwzDst[nSize * 2] = 0;
	return nSize * 2;
}

UAPI(UINT) WStrLoad(UINT uID, PWSTR pwzStr, UINT nMax UDEF(MAX_STR))
{
	return LoadStringW(g_hInst, uID, pwzStr, nMax);
}

UAPI(PCWSTR) WStrGet(UINT uID)
{
#ifdef WINCE
	return (PCWSTR) WStrLoad(uID, NULL, 0);
#else
	HRSRC hRsrc = FindResource(g_hInst, MAKEINTRESOURCE((uID / 16) + 1), RT_STRING);
	if (hRsrc)
	{
		PCWSTR pwzStr = (PCWSTR) LoadResource(g_hInst, hRsrc);
		if (pwzStr)
		{
			UINT i = uID % 16;
			while (i--)
			{
				pwzStr += *pwzStr + 1;
			}
			return pwzStr + 1;
		}
	}
	return NULL;
#endif
}
/**********************************************************************************************************************/



/**********************************************************************************************************************/
/* Transformable String */

#ifdef _UNICODE

#define TChrIsNum					WChrIsNum
#define TChrIsAlpha					WChrIsAlpha
#define TChrIsSymbol				WChrIsSymbol
#define TChrIsPrintable				WChrIsPrintable
#define TChrToLower					WChrToLower
#define TChrToUpper					WChrToUpper
#define TChrToHex					WChrToHex
#define TChrFromHex					WChrFromHex
#define TChrEqualI					WChrEqualI

#define TStrEnd						WStrEnd
#define TStrLen						WStrLen
#define TStrCopy					WStrCopy
#define TStrCopyN					WStrCopyN
#define TStrCat						WStrCat
#define TStrCmp						WStrCmp
#define TStrCmpI					WStrCmpI
#define TStrCmpN					WStrCmpN
#define TStrCmpNI					WStrCmpNI

#define TStrChr						WStrChr
#define TStrRChr					WStrRChr
#define TStrStr						WStrStr
#define TStrStrI					WStrStrI

#define TStrRep						WStrRep
#define TStrTrim					WStrTrim
#define TStrEqual					WStrEqual
#define TStrEqualI					WStrEqualI
#define TStrSplit					WStrSplit
#define TStrRSplit					WStrRSplit
#define TStrMatch					WStrMatch
#define TStrMatchI					WStrMatchI
#define TStrToUpper					WStrToUpper
#define TStrToLower					WStrToLower
#define TStrToInt					WStrToInt
#define TStrToInt64					WStrToInt64
#define TStrToDouble				WStrToDouble
#define TStrFromInt					WStrFromInt
#define TStrFromDouble				WStrFromDouble
#define TStrToHex					WStrToHex
#define TStrFromHex					WStrFromHex
#define TStrFormat					WStrFormat
#define TStrFormatV					WStrFormatV

#define TStrLoad					WStrLoad
#define TStrGet						WStrGet

#define TStrToAStr					WStrToAStr
#define TStrToWStr					WStrCopyN
#define AStrToTStr					AStrToWStr
#define WStrToTStr					WStrCopyN

#define TStrToUTF8					WStrToUTF8
#define UTF8ToTStr					UTF8ToWStr

#else

#define TChrIsNum					AChrIsNum
#define TChrIsAlpha					AChrIsAlpha
#define TChrIsSymbol				AChrIsSymbol
#define TChrIsPrintable				AChrIsPrintable
#define TChrToLower					AChrToLower
#define TChrToUpper					AChrToUpper
#define TChrToHex					AChrToHex
#define TChrFromHex					AChrFromHex
#define TChrEqualI					AChrEqualI

#define TStrEnd						AStrEnd
#define TStrLen						AStrLen
#define TStrCopy					AStrCopy
#define TStrCopyN					AStrCopyN
#define TStrCat						AStrCat
#define TStrCmp						AStrCmp
#define TStrCmpI					AStrCmpI
#define TStrCmpN					AStrCmpN
#define TStrCmpNI					AStrCmpNI

#define TStrChr						AStrChr
#define TStrRChr					AStrRChr
#define TStrStr						AStrStr
#define TStrStrI					AStrStrI

#define TStrRep						AStrRep
#define TStrTrim					AStrTrim
#define TStrEqual					AStrEqual
#define TStrEqualI					AStrEqualI
#define TStrSplit					AStrSplit
#define TStrRSplit					AStrRSplit
#define TStrMatch					AStrMatch
#define TStrMatchI					AStrMatchI
#define TStrToUpper					AStrToUpper
#define TStrToLower					AStrToLower
#define TStrToInt					AStrToInt
#define TStrToInt64					AStrToInt64
#define TStrToDouble				AStrToDouble
#define TStrFromInt					AStrFromInt
#define TStrFromDouble				AStrFromDouble
#define TStrToHex					AStrToHex
#define TStrFromHex					AStrFromHex
#define TStrFormat					AStrFormat
#define TStrFormatV					AStrFormatV

#define TStrLoad					AStrLoad
#define TStrGet						AStrGet

#define TStrToAStr					AStrCopyN
#define TStrToWStr					AStrToWStr
#define AStrToTStr					AStrCopyN
#define WStrToTStr					WStrToAStr

#define TStrToUTF8					AStrToUTF8
#define UTF8ToTStr					UTF8ToAStr

#endif
/**********************************************************************************************************************/



/**********************************************************************************************************************/
/* File */
UAPI(HANDLE) UFileOpen(PCTSTR ptzPath, DWORD dwAccess UDEF(UFILE_READ))
{
	DWORD dwCreate = (dwAccess == UFILE_WRITE) ? CREATE_ALWAYS :
		((dwAccess == UFILE_READWRITE) ? OPEN_ALWAYS : OPEN_EXISTING);
	HANDLE hFile = CreateFile(ptzPath, dwAccess, FILE_SHARE_READ, NULL, dwCreate, 0, NULL);
	return (hFile == INVALID_HANDLE_VALUE) ? NULL : hFile;
}

UAPI(BOOL) UFileClose(HANDLE hFile)
{
	return CloseHandle(hFile);
}

UAPI(UINT) UFileRead(HANDLE hFile, PVOID pvData, UINT nSize)
{
	DWORD dwRead = 0;
	ReadFile(hFile, pvData, nSize, &dwRead, NULL);
	return dwRead;
}

UAPI(UINT) UFileWrite(HANDLE hFile, PCVOID pvData, UINT nSize)
{
	DWORD dwWrite = 0;
	WriteFile(hFile, pvData, nSize, &dwWrite, NULL);
	return dwWrite;
}

UAPI(UINT) UFileSeek(HANDLE hFile, INT iOffset, DWORD dwOrigin UDEF(UFILE_BEGIN))
{
	return SetFilePointer(hFile, iOffset, NULL, dwOrigin);
}

UAPI(UINT) UFileTell(HANDLE hFile)
{
	return UFileSeek(hFile, 0, UFILE_CURRENT);
}

UAPI(UINT) UFileGetSize(HANDLE hFile)
{
	DWORD dwSize = GetFileSize(hFile, NULL);
	return (dwSize == INVALID_FILE_SIZE) ? 0 : dwSize;
}

UAPI(BOOL) UFileSetSize(HANDLE hFile, UINT nSize UDEF(0))
{
	UFileSeek(hFile, nSize, UFILE_BEGIN);
	return SetEndOfFile(hFile);
}

UAPI(PTSTR) UFileToTStr(PCTSTR ptzPath, PUINT puSize UDEF(NULL))
{
	UINT nSize;
	PTSTR ptzData;
	HANDLE hFile = UFileOpen(ptzPath, UFILE_READ);
	if (hFile == NULL)
	{
		return NULL;
	}

	nSize = UFileGetSize(hFile);
	if (puSize && (nSize > *puSize))
	{
		nSize = *puSize;
	}
	ptzData = (PTSTR) UMemAlloc(nSize + 16);
	if (ptzData)
	{
		UINT nRead = UFileRead(hFile, ptzData, 2);
		if (nRead == 2)
		{
			WORD wBom = *((PWORD) ptzData);
			if (wBom == 0xFEFF)
			{
				nRead = 0;
			}
			else if (wBom == 0xBBEF)
			{
				nRead = 0;
				UFileRead(hFile, ptzData, 1);
			}

			nRead += UFileRead(hFile, (PBYTE) ptzData + nRead, nSize - 2);

#ifdef _UNICODE
			if ((nRead >= 2) && (wBom != 0xFEFF) && (((PBYTE) ptzData)[1] != 0))
			{
				PASTR pazTemp = (PASTR) ptzData;
				ptzData = (PTSTR) UMemAlloc((nRead + 16) * sizeof(WCHAR));
				if (ptzData)
				{
					UINT uCodePage = ((wBom == 0xBBEF) || AStrIsUTF8XML(pazTemp)) ? UCP_UTF8 : UCP_ANSI;
					nRead = sizeof(WCHAR) * AStrToWStr(ptzData, pazTemp, nRead + 16, nRead, uCodePage);
				}
				UMemFree(pazTemp);
			}
#else
			if ((nRead >= 2) && ((wBom == 0xFEFF) || (wBom == 0xBBEF) || ((PBYTE) ptzData)[1] == 0))
			{
				PWSTR pwzTemp = (PWSTR) ptzData;
				ptzData = (PTSTR) UMemAlloc((nRead + 16) * sizeof(WCHAR));
				if (ptzData)
				{
					if ((wBom == 0xBBEF) || AStrIsUTF8XML(pazTemp))
					{
						PASTR pazUTF8 = (PASTR) pwzTemp;
						pwzTemp = (PWSTR) ptzData;
						ptzData = (PTSTR) pazUTF8;
						nRead = sizeof(WCHAR) * AStrToWStr(pwzTemp, pazUTF8, nRead, nRead, UCP_UTF8);
					}
					nRead = sizeof(ACHAR) * WStrToAStr(ptzData, pwzTemp, nSize, nRead / sizeof(WCHAR), UCP_ANSI);
				}
				UMemFree(pwzTemp);
			}
#endif
		}

		if (ptzData)
		{
			if (puSize)
			{
				*puSize = nRead;
			}
			((PBYTE) ptzData)[nRead] = 0;
			((PBYTE) ptzData)[nRead + 1] = 0;
			((PBYTE) ptzData)[nRead + 2] = 0;
		}
	}
	UFileClose(hFile);
	return ptzData;
}

UAPI(PVOID) UFileLoad(PCTSTR ptzPath, PUINT puSize UDEF(NULL), PVOID pvData UDEF(NULL))
{
	UINT nSize;
	HANDLE hFile = UFileOpen(ptzPath, UFILE_READ);
	if (hFile == NULL)
	{
		return NULL;
	}

	nSize = UFileGetSize(hFile);
	if (puSize && (nSize > *puSize))
	{
		nSize = *puSize;
	}

	if (pvData == NULL)
	{
		pvData = (PBYTE) UMemAlloc(nSize + 16);
	}

	if (pvData)
	{
		nSize = UFileRead(hFile, pvData, nSize);
		((PBYTE) pvData)[nSize] = 0;
		((PBYTE) pvData)[nSize + 1] = 0;
		if (puSize)
		{
			*puSize = nSize;
		}
	}

	UFileClose(hFile);
	return pvData;
}

UAPI(UINT) UFileSave(PCTSTR ptzPath, PCVOID pvData, UINT nSize, BOOL bAppend UDEF(FALSE))
{
	UINT nWrite;
	HANDLE hFile = UFileOpen(ptzPath, bAppend ? UFILE_READWRITE : UFILE_WRITE);
	if (hFile == NULL)
	{
		return 0;
	}

	if (bAppend)
	{
		UFileSeek(hFile, 0, UFILE_END);
	}
	nWrite = UFileWrite(hFile, pvData, nSize);
	UFileClose(hFile);
	return nWrite;
}
/**********************************************************************************************************************/



/**********************************************************************************************************************/
/* File Management */
UAPI(UINT) UPathMake(PTSTR ptzDir, PCTSTR ptzSub)
{
	PTSTR p = TStrEnd(ptzDir);
	if (p[-1] != '\\')
	{
		*p++ = '\\';
	}
	return (UINT) (p - ptzDir) + TStrCopy(p, ptzSub);
}

UAPI(PTSTR) UPathSplit(PTSTR* pptzPath)
{
	PTSTR p;
	PTSTR ptzEnd = TStrEnd(*pptzPath);
	for (p = ptzEnd; p >= *pptzPath; p--)
	{
		if (*p == '\\')
		{
			*p++ = 0;
			return p;
		}
	}
	p = *pptzPath;
	*pptzPath = ptzEnd;
	return p;
}

UAPI(BOOL) UFileDelete(PCTSTR ptzPath)
{
	return DeleteFile(ptzPath);
}

UAPI(BOOL) UFileCopy(PCTSTR ptzPath, PCTSTR ptzNewPath)
{
	return CopyFile(ptzPath, ptzNewPath, FALSE);
}

UAPI(BOOL) UFileMove(PCTSTR ptzPath, PCTSTR ptzNewPath)
{
	return MoveFile(ptzPath, ptzNewPath);
}

typedef HRESULT (UCALLBACK *UFOUNDFILEPROC)(PCVOID pvParam, PCTSTR ptzDir, CONST PWIN32_FIND_DATA pFind);
UAPI(HRESULT) UFileFind(UFOUNDFILEPROC pFoundFile, PCVOID pvParam, PCTSTR ptzDir UDEF(TEXT("")), PCTSTR ptzWildcard UDEF(TEXT("*")))
{
	HANDLE hFind;
	WIN32_FIND_DATA fd;
	TCHAR tzPath[MAX_PATH];

	HRESULT hResult = ERROR_FILE_NOT_FOUND;
	TStrFormat(tzPath, TEXT("%s\\%s"), ptzDir, ptzWildcard);
	hFind = FindFirstFile(tzPath, &fd);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			hResult = pFoundFile(pvParam, ptzDir, &fd);
		}
		while ((hResult != E_ABORT) && FindNextFile(hFind, &fd));
		FindClose(hFind);
	}

	return hResult;
}

UAPI(BOOL) UFileNameValid(PCTSTR ptzName)
{
	UINT i;
	PCTSTR p;
	CONST STATIC TCHAR c_tzInvalidChar[] = TEXT("\\/:*?\"<>|");
	for (p = ptzName; *p; p++)
	{
		for (i = 0; i < _NumOf(c_tzInvalidChar) - 1; i++)
		{
			if (*p == c_tzInvalidChar[i])
			{
				return FALSE;
			}
		}
	}
	return (*ptzName != 0);
}

UAPI(BOOL) UFileExist(PCTSTR ptzPath)
{
	WIN32_FILE_ATTRIBUTE_DATA a;
	return GetFileAttributesEx(ptzPath, GetFileExInfoStandard, &a) && !(a.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
}

UAPI(BOOL) UDirExist(PCTSTR ptzDir)
{
	WIN32_FILE_ATTRIBUTE_DATA a;
	return GetFileAttributesEx(ptzDir, GetFileExInfoStandard, &a) && (a.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
}

UAPI(BOOL) UDirCreate(PCTSTR ptzDir)
{
	PTSTR p;
	for (p = (PTSTR) ptzDir; p = TStrChr(p, '\\'); *p++ = '\\')
	{
		*p = 0;
		if (!UDirExist(ptzDir))
		{
			CreateDirectory(ptzDir, NULL);
		}
	}
	return TRUE;
}

UAPI(BOOL) UDirDelete(PCTSTR ptzDir)
{
	return RemoveDirectory(ptzDir);
}

UAPI(UINT) UDirGetAppPath(PTSTR ptzPath)
{
	return GetModuleFileName(g_hInst, ptzPath, MAX_PATH);
}

UAPI(UINT) UDirGetAppFile(PTSTR ptzPath, PCTSTR ptzFile)
{
	PTSTR p = ptzPath + UDirGetAppPath(ptzPath);
	for (; p >= ptzPath; p--)
	{
		if (*p == '\\')
		{
			p++;
			break;
		}
	}
	return (UINT) (p - ptzPath) + TStrCopy(p, ptzFile);
}

UAPI(UINT) UDirGetAppExt(PTSTR ptzPath, PCTSTR ptzExt)
{
	PTSTR p;
	PTSTR ptzEnd = ptzPath + UDirGetAppPath(ptzPath);
	for (p = ptzEnd; p >= ptzPath; p--)
	{
		if (*p == '.')
		{
			p++;
			break;
		}
	}
	if (p == ptzPath)
	{
		p = ptzEnd;
		*p++ = '.';
	}
	return (UINT) (p - ptzPath) + TStrCopy(p, ptzExt);
}

UAPI(UINT) UDirGetCurrent(PTSTR ptzDir)
{
#ifdef GetCurrentDirectory
	return GetCurrentDirectory(MAX_PATH, ptzDir);
#else
	UDirGetAppPath(ptzDir);
	return (UINT) (UPathSplit(&ptzDir) - ptzDir - 1);
#endif
}

UAPI(UINT) UDirGetTemp(PTSTR ptzDir)
{
	return GetTempPath(MAX_PATH, ptzDir);
}

UAPI(UINT) UFileGetTemp(PTSTR ptzPath)
{
	UDirGetTemp(ptzPath);
	return GetTempFileName(ptzPath, TEXT("UNI"), 0, ptzPath);
}
/**********************************************************************************************************************/



/**********************************************************************************************************************/
/* Thread */
typedef DWORD (UCALLBACK* UPROC)(PVOID pvParam);
UAPI(HANDLE) UThreadCreate(UPROC upProc, PVOID pvParam UDEF(NULL))
{
	DWORD dwThread;
	return CreateThread(NULL, 0, upProc, pvParam, 0, &dwThread);
}

UAPI(BOOL) UThreadClose(HANDLE hThread)
{
	return CloseHandle(hThread);
}

UAPI(BOOL) UThreadCall(UPROC upProc, PVOID pvParam UDEF(NULL))
{
	return UThreadClose(UThreadCreate(upProc, pvParam));
}

UAPI(BOOL) UThreadTerminate(HANDLE hThread)
{
	return TerminateThread(hThread, 0);
}

UAPI(BOOL) UThreadSuspend(HANDLE hThread)
{
	return SuspendThread(hThread);
}

UAPI(BOOL) UThreadResume(HANDLE hThread)
{
	return ResumeThread(hThread);
}
/**********************************************************************************************************************/



/**********************************************************************************************************************/
/* Misc */
UAPI(UINT) UGetRandom()
{
	srand(GetTickCount());
	return rand();
}

typedef SYSTEMTIME UTIME, *PUTIME;
UAPI(VOID) UGetTime(PUTIME pTime)
{
	GetLocalTime(pTime);
}

UAPI(UINT) UGetTimeStamp()
{
	return GetTickCount();
}

UAPI(VOID) UTrace(PCTSTR ptzFormat, ...)
{
	DWORD i;
	va_list va;
	TCHAR tz[MAX_STR];

#ifdef _TRACE_TIME
	UTIME ut;
	UGetTime(&ut);
	i = TStrFormat(tz, TEXT("%02u:%02u:%02u\t"), (UINT) ut.wHour, (UINT) ut.wMinute, (UINT) ut.wSecond);
#else
	i = 0;
#endif

	va_start(va, ptzFormat);
	i += TStrFormatV(tz + i, ptzFormat, va);
	va_end(va);

	tz[i++] = '\r';
	tz[i++] = '\n';
	tz[i] = 0;

#if defined(_TRACE_TO_FILE)
	UFileSave(_TRACE_TO_FILE, tz, i * sizeof(TCHAR), TRUE);
#elif defined(_TRACE_TO_CONSOLE)
	WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), tz, i, &i, NULL);
#else
	OutputDebugString(tz);
#endif
}

UAPI(VOID) UAssert(PCTSTR ptzExp, PCTSTR ptzFile, UINT uLine)
{
	UINT i;
	TCHAR tzText[MAX_STR];
	TCHAR tzModule[MAX_PATH];

	UDirGetAppPath(tzModule);
	TStrFormat(tzText,
		TEXT("Assertion failed!\n\n")
		TEXT("Program: %s\n")
		TEXT("File: %s\n")
		TEXT("Line: %d\n\n")
		TEXT("Expression: %s\n\n")
		TEXT("Press Retry to debug the application - JIT must be enabled"),
		tzModule, ptzFile, uLine, ptzExp);

	i = MessageBox(NULL, tzText, TEXT("UniDebug"), MB_ICONERROR | MB_ABORTRETRYIGNORE);
#ifdef WIN32_PLATFORM_WFSP
	if (i == 0)
	{
		i = MessageBox(NULL, tzText, TEXT("UniDebug"), MB_ICONERROR | MB_RETRYCANCEL);
	}
#endif
	if (i == IDABORT)
	{
		ExitProcess(0);
	}
	else if (i == IDRETRY)
	{
		DebugBreak();
	}
}
/**********************************************************************************************************************/



/**********************************************************************************************************************/
/* UAutoTrace */
#ifdef __cplusplus
class UAutoTrace
{
private:
	UINT m_uLine;
	PCTSTR m_ptzName;

public:
	UAutoTrace(PCTSTR ptzName, UINT uLine): m_uLine(uLine), m_ptzName(ptzName)
	{
		UTrace(TEXT("Enter %s:%u"), ptzName, uLine);
	}

	~UAutoTrace()
	{
		UTrace(TEXT("Leave %s:%u"), m_ptzName, m_uLine);
	}
};
#endif
/**********************************************************************************************************************/



/**********************************************************************************************************************/
/* Debug */
#ifdef __FUNCTION__
#define __FUNCFILE__				TEXT(__FUNCTION__)
#else
#define __FUNCFILE__				TEXT(__FILE__)
#endif

#ifdef _TRACE
#define _Trace						UTrace
#define _LineTrace()				UTrace(TEXT("Trace %s:%u"), __FUNCFILE__, __LINE__)
#ifdef __cplusplus
#define _AutoTrace()				UAutoTrace at(__FUNCFILE__, __LINE__)
#else
#define _AutoTrace()				_LineTrace()
#endif
#else
#define _Trace
#define _LineTrace()
#define _AutoTrace()
#endif

#ifdef _DEBUG
#define _Assert(e)					(VOID) ((e) || (UAssert(TEXT(#e), TEXT(__FILE__), __LINE__), 0))
#define _Verify(e)					_Assert(e)
#else
#define _Assert(e)					((VOID) 0)
#define _Verify(e)					((VOID) (e))
#endif

#ifdef _CHECK
#ifdef _DEBUG
FINLINE BOOL __Check(BOOL e)		{BOOL b = (e); _Assert(b); return b;}
#define _Check(e)					__Check(e)
#else
#define _Check(e)					(e)
#endif
#else
#ifdef _DEBUG
#define _Check(e)					(_Assert(e), TRUE)
#else
#define _Check(e)					(TRUE)
#endif
#endif
/**********************************************************************************************************************/
