


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CeleThunk 2.0.200
// Copyright (C) Yonsm 2008-2009, All Rights Reserved.
#pragma once
#include <Windows.h>
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CeleThunk
#pragma pack(push, 1)
#if defined(_M_IX86)
template<typename TPARAM = HWND> struct CeleThunk
{
	DWORD m_dwMovEaxEsp4;	// MOV EAX, [ESP + 4]
	WORD m_wMovParamEax;	// MOV DS:[m_tParam], EAX
	TPARAM* m_ptParam;

	DWORD m_dwMovEsp4;		// MOV [ESP + 4], pThis
	PVOID m_pThis;
	BYTE m_bJump;			// JMP pProc
	DWORD m_dwDelta;

	TPARAM m_tParam;
	operator TPARAM() {return m_tParam;}
	TPARAM operator =(TPARAM tParam) {return m_tParam = tParam;}

	template<typename TPROC> PROC Init(PVOID pThis, TPROC tProc)
	{
		m_dwMovEaxEsp4 = 0x0424448B;
		m_wMovParamEax = 0xA33E;
		m_ptParam = &m_tParam;
		m_dwMovEsp4 = 0x042444C7;
		m_pThis = pThis;
		m_bJump = 0xE9;
		m_dwDelta = (DWORD) (*((INT_PTR*) &tProc) - ((INT_PTR) &m_dwDelta + sizeof(m_dwDelta)));
		FlushInstructionCache(GetCurrentProcess(), this, sizeof(CeleThunk));
		m_tParam = (TPARAM) 0;
		return (PROC) this;
	}
};
#elif defined(_ARM_)
template<typename TPARAM = HWND> struct CeleThunk
{
	DWORD m_dwStrR0Param;	// STR R0, [PC,#12]
	DWORD m_dwMovR0This;	// LDR R0, [PC]
	DWORD m_dwMovPcProc;	// LDR PC, [PC]
	PVOID m_pThis;
	PVOID m_pProc;

	TPARAM m_tParam;
	operator TPARAM() {return m_tParam;}
	TPARAM operator =(TPARAM tParam) {return m_tParam = tParam;}

	template<typename TPROC> PROC Init(PVOID pThis, TPROC tProc)
	{
		m_dwStrR0Param = 0xE58F000C;
		m_dwMovR0This = 0xE59F0000;
		m_dwMovPcProc = 0xE59FF000;
		m_pThis = pThis;
		m_pProc = *((PVOID*) &tProc);
		FlushInstructionCache(GetCurrentProcess(), this, sizeof(CeleThunk));
		m_tParam = (TPARAM) 0;
		return (PROC) this;
	}
};
#else
#error CeleThunk does not support this platform!
#endif
#pragma pack(pop)
/**********************************************************************************************************************/
