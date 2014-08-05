

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CeleCom 2.0.202
// Copyright (C) Yonsm 2008-2009, All Rights Reserved.
#pragma once
#include <Windows.h>
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CeleComObject template class
__declspec(selectany) LONG g_nDllRef = 0;
template<class TBase> class CeleComObject: public TBase
{
protected:
	LONG m_nRef;

public:
	CeleComObject()
	{
		m_nRef = 1;
		InterlockedIncrement(&g_nDllRef);
	}

	virtual ~CeleComObject()
	{
		InterlockedDecrement(&g_nDllRef);
	}

public:
	STDMETHOD_(ULONG, AddRef)()
	{
		return InterlockedIncrement(&m_nRef);
	}

	STDMETHOD_(ULONG, Release)()
	{
		if (InterlockedDecrement(&m_nRef) == 0)
		{
			delete this;
			return 0;
		}
		return m_nRef;
	}
};

#define IUnknown_QueryBegin()		STDMETHOD(QueryInterface)(REFIID iID, PVOID* ppIF) {if (ppIF == NULL) return E_INVALIDARG
#define IUnknown_Query(I)			IUnknown_Query2(I, I)
#define IUnknown_Query2(I, P)		else if (iID == IID_##I) *ppIF = static_cast<P*>(this)
#define IUnknown_QueryEnd()			else {*ppIF = NULL; return E_NOINTERFACE;} ((IUnknown*) (*ppIF))->AddRef();return S_OK;}
#define IUnknown_QueryForward(P)	else return P::QueryInterface(iID, ppIF); ((IUnknown*) (*ppIF))->AddRef();return S_OK;}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CeleComFactory template class
template<class TObject> class CeleComFactory: public CeleComObject<IClassFactory>
{
public:
	// IUnknown
	IUnknown_QueryBegin();
	IUnknown_Query(IUnknown);
	IUnknown_Query(IClassFactory);
	IUnknown_QueryEnd();

public:
	// IClassFactory
	STDMETHOD(CreateInstance)(IUnknown* pUnknown, REFIID iID, PVOID* ppIF)
	{
		if (ppIF == NULL)
		{
			return E_POINTER;
		}

		if (pUnknown != NULL)
		{
			return CLASS_E_NOAGGREGATION;
		}

		TObject* pObj = new TObject;
		if (pObj == NULL)
		{
			return E_OUTOFMEMORY;
		}

		HRESULT hResult = pObj->QueryInterface(iID, ppIF);
		pObj->Release();
		return hResult;
	}

	STDMETHOD(LockServer)(BOOL bLock)
	{
		if (bLock)
		{
			InterlockedIncrement(&g_nDllRef);
		}
		else
		{
			InterlockedDecrement(&g_nDllRef);
		}
		return S_OK;
	}
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
