


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CeleGraph 2.0.202
// Copyright (C) Yonsm 2005-2010, All Rights Reserved.
#pragma once

#define NO_DSHOW_STRSAFE
#include <DShow.h>

#pragma comment(lib, "StrmIIDs.lib")

#define _GraphQueryCall(IF, CALL) _QueryCall(m_pGraph, IF, CALL)
#define _QueryCall(P, IF, CALL)	\
	IF* pIF;	\
	HRESULT hResult = (P) ? (P)->QueryInterface(__uuidof(IF), reinterpret_cast<PVOID*>(&pIF)) : E_POINTER;	\
	if (SUCCEEDED(hResult))	\
	{	\
	hResult = pIF->##CALL;	\
	pIF->Release();	\
	}	__noop
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CeleGraph class
class CeleGraph
{
public:
	// Constructor
	CeleGraph(IGraphBuilder* pGraph = NULL)
	{
		if (m_pGraph = pGraph)
		{
			m_pGraph->AddRef();
		}
		else
		{
			CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC, IID_IGraphBuilder, reinterpret_cast<PVOID*>(&m_pGraph));
		}
	}

	// Destructor
	~CeleGraph()
	{
		if (m_pGraph)
		{
			m_pGraph->Release();
		}
	}

public:
	operator IGraphBuilder*()
	{
		return m_pGraph;
	}

	IGraphBuilder* operator =(IGraphBuilder* pGraph)
	{
		if (pGraph)
		{
			pGraph->AddRef();
		}
		if (m_pGraph)
		{
			m_pGraph->Release();
		}
		return m_pGraph = pGraph;
	}

	template<class T> operator T()
	{
		T p = NULL;
		QueryInterface(&p);
		return p;
	}

	template<class T> HRESULT QueryInterface(T** pIF)
	{
		return m_pGraph ? m_pGraph->QueryInterface(__uuidof(T), reinterpret_cast<PVOID*>(pIF)) : E_POINTER;
	}

public:
	SIZE GetSize()
	{
		SIZE sz = {0};
		_GraphQueryCall(IBasicVideo, GetVideoSize(&sz.cx, &sz.cy));
		return sz;
	}

	LONGLONG GetDuration()
	{
		LONGLONG llDuration = 0;
		_GraphQueryCall(IMediaSeeking, GetDuration(&llDuration));
		return llDuration;
	}

	OAFilterState GetState()
	{
		OAFilterState lState = State_Stopped;
		_GraphQueryCall(IMediaControl, GetState(10, &lState));
		return lState;
	}

	LONGLONG GetPos()
	{
		LONGLONG c = 0, s;
		_GraphQueryCall(IMediaSeeking, GetPositions(&c, &s));
		return c;
	}

	UINT GetPercent()
	{
		LONGLONG c = 0, s = 0;
		_GraphQueryCall(IMediaSeeking, GetPositions(&c, &s));
		return s ? ((UINT) (c * 100 / s)) : 0;
	}

	LONG GetVolume()
	{
		LONG lVolume = 0;
		_GraphQueryCall(IBasicAudio, get_Volume(&lVolume));
		return lVolume;
	}

	BOOL IsFullScreen()
	{
		LONG lMode = OAFALSE;
		_GraphQueryCall(IVideoWindow, get_FullScreenMode(&lMode));
		return (lMode != OAFALSE);
	}

public:
	HRESULT SetVolume(LONG lVolume = 1000)
	{
		_GraphQueryCall(IBasicAudio, put_Volume(lVolume));
		return hResult;
	}

	HRESULT SetFullScreen(BOOL bFullScreen = OATRUE)
	{
		#define _SetFullScreen put_FullScreenMode(bFullScreen); if (bFullScreen) pIF->SetWindowForeground(OATRUE)
		_GraphQueryCall(IVideoWindow, _SetFullScreen);
		return hResult;
	}

	HRESULT SetVideoWindow(HWND hVideoWnd = NULL)
	{
		#define _SetVideoWindow	\
			put_Owner((OAHWND) hVideoWnd);	\
			hResult = pIF->put_WindowStyle(hVideoWnd ? (WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN) : (WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN));	\
			hResult = pIF->put_MessageDrain((OAHWND) hVideoWnd);	\
			if (hVideoWnd) FitToWindow(hVideoWnd);

		_GraphQueryCall(IVideoWindow, _SetVideoWindow);
		return hResult;
	}

	HRESULT FitToWindow(HWND hVideoWnd)
	{
		RECT rt;
		GetClientRect(hVideoWnd, &rt);
		_GraphQueryCall(IVideoWindow, SetWindowPosition(rt.left, rt.top, rt.right, rt.bottom));
		return hResult;
	}

public:
	HRESULT Run()
	{
		_GraphQueryCall(IMediaControl, Run());
		return hResult;
	}

	HRESULT Pause()
	{
		_GraphQueryCall(IMediaControl, Pause());
		return hResult;
	}

	HRESULT Stop()
	{
		_GraphQueryCall(IMediaControl, Stop()); // Seek();
		return hResult;
	}

	HRESULT Seek(LONGLONG llPos = 0)
	{
		_GraphQueryCall(IMediaSeeking, SetPositions(&llPos, AM_SEEKING_AbsolutePositioning, NULL, AM_SEEKING_NoPositioning));
		return hResult;
	}

public:
	// Add to ROT
	HRESULT AddToRot(PDWORD pdwRot = NULL)
	{
#ifdef WINCE
		return E_NOTIMPL;
#else
		IRunningObjectTable* pRot;
		HRESULT hResult = GetRunningObjectTable(0, &pRot);
		if (SUCCEEDED(hResult))
		{
			IMoniker* pMoniker;
			WCHAR wzText[MAX_PATH];
			wsprintfW(wzText, L"FilterGraph %08x pid %08x", (DWORD_PTR) m_pGraph, GetCurrentProcessId());
			hResult = CreateItemMoniker(L"!", wzText, &pMoniker);
			if (SUCCEEDED(hResult))
			{
				DWORD dwRot;
				hResult = pRot->Register(ROTFLAGS_REGISTRATIONKEEPSALIVE, m_pGraph, pMoniker, pdwRot ? pdwRot : &dwRot);
				pMoniker->Release();
			}
			pRot->Release();
		}
		return hResult;
#endif
	}

	// Remove from ROT
	HRESULT RemoveFromRot(DWORD dwRot)
	{
#ifdef WINCE
		return E_NOTIMPL;
#else
		IRunningObjectTable* pRot;
		HRESULT hResult = GetRunningObjectTable(0, &pRot);
		if (SUCCEEDED(hResult))
		{
			hResult = pRot->Revoke(dwRot);
			pRot->Release();
		}
		return hResult;
#endif
	}

	// Save graph to file
	HRESULT SaveToFile(PCWSTR pwzFileName = L"CeleGraph.grf")
	{
		IStorage* pStorage;
		HRESULT hResult = StgCreateDocfile(pwzFileName, STGM_CREATE | STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, &pStorage);
		if (SUCCEEDED(hResult))
		{
			IStream* pStream;
			hResult = pStorage->CreateStream(L"ActiveMovieGraph", STGM_WRITE | STGM_CREATE | STGM_SHARE_EXCLUSIVE, 0, 0, &pStream);
			if (SUCCEEDED(hResult))
			{
				IPersistStream* pPersist;
				hResult = m_pGraph->QueryInterface(IID_IPersistStream, reinterpret_cast<PVOID*>(&pPersist));
				if (SUCCEEDED(hResult))
				{
					hResult = pPersist->Save(pStream, TRUE);
					if (SUCCEEDED(hResult))
					{
						hResult = pStorage->Commit(STGC_DEFAULT);
					}
					pPersist->Release();
				}
				pStream->Release();
			}
			pStorage->Release();
		}
		return hResult;
	}

	// Load graph from file
	HRESULT LoadFromFile(PCWSTR pwzFileName = L"CeleGraph.grf")
	{
		IStorage* pStorage;
		HRESULT hResult = StgOpenStorage(pwzFileName, 0, STGM_TRANSACTED | STGM_READ | STGM_SHARE_DENY_WRITE, 0, 0, &pStorage);
		if (SUCCEEDED(hResult))
		{
			IPersistStream* pPersist;
			hResult = m_pGraph->QueryInterface(IID_IPersistStream, reinterpret_cast<PVOID*>(&pPersist));
			if (SUCCEEDED(hResult))
			{
				IStream* pStream;
				hResult = pStorage->OpenStream(L"ActiveMovieGraph", 0, STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pStream);
				if (SUCCEEDED(hResult))
				{
					hResult = pPersist->Load(pStream);
					pStream->Release();
				}
				pPersist->Release();
			}
			pStorage->Release();
		}
		return hResult;
	}

public:
	// Add filter by CLSID
	HRESULT AddFilter(REFCLSID cID, PCWSTR pwzName = NULL, PVOID* ppIF = NULL, const IID* pID = NULL)
	{
		IBaseFilter* pFilter;
		HRESULT hResult = CoCreateInstance(cID, 0, CLSCTX_INPROC_SERVER, IID_IBaseFilter, reinterpret_cast<PVOID*>(&pFilter));
		if (SUCCEEDED(hResult))
		{
			hResult = m_pGraph->AddFilter(pFilter, pwzName);
			if (SUCCEEDED(hResult))
			{
				if (ppIF)
				{
					if (pID)
					{
						hResult = pFilter->QueryInterface(*pID, ppIF);
						pFilter->Release();
					}
					else
					{
						*ppIF = pFilter;
					}
				}
				else
				{
					pFilter->Release();
				}
			}
		}
		return hResult;
	}

	// Connect two filters by output pin
	HRESULT ConnectFilter(IPin* pSrc, IBaseFilter* pDst)
	{
		IEnumPins* pEnum;
		HRESULT hResult = pDst ? pDst->EnumPins(&pEnum) : E_POINTER;
		if (SUCCEEDED(hResult))
		{
			hResult = VFW_E_CANNOT_CONNECT;
			for (IPin* pIn = NULL; FAILED(hResult) && SUCCEEDED(pEnum->Next(1, &pIn, 0)); pIn->Release())
			{
				PIN_DIRECTION dir;
				pIn->QueryDirection(&dir);
				if (dir == PINDIR_INPUT)
				{
					IPin* pTemp;
					if (SUCCEEDED(pIn->ConnectedTo(&pTemp)))
					{
						pTemp->Release();
					}
					else
					{
						hResult = m_pGraph->Connect(pSrc, pIn);
					}
				}
			}
			pEnum->Release();
		}
		return hResult;
	}

	// Connect two filters
	HRESULT ConnectFilter(IBaseFilter* pSrc, IBaseFilter* pDst)
	{
		IEnumPins* pEnum;
		HRESULT hResult = pSrc ? pSrc->EnumPins(&pEnum) : E_POINTER;
		if (SUCCEEDED(hResult))
		{
			hResult = VFW_E_CANNOT_CONNECT;
			for (IPin* pOut = NULL; SUCCEEDED(pEnum->Next(1, &pOut, 0)); pOut->Release())
			{
				PIN_DIRECTION dir;
				pOut->QueryDirection(&dir);
				if (dir == PINDIR_OUTPUT)
				{
					IPin* pTemp;
					if (SUCCEEDED(pOut->ConnectedTo(&pTemp)))
					{
						pTemp->Release();
					}
					else if (SUCCEEDED(ConnectFilter(pOut, pDst)))
					{
						hResult = S_OK;
					}
				}
			}
			pEnum->Release();
		}
		return hResult;
	}

	// Render filter
	HRESULT RenderFilter(IBaseFilter* pFilter)
	{
		IEnumPins* pEnum;
		HRESULT hResult = pFilter->EnumPins(&pEnum);
		if (SUCCEEDED(hResult))
		{
			hResult = VFW_E_CANNOT_RENDER;
			for (IPin* pOut = NULL; SUCCEEDED(pEnum->Next(1, &pOut, 0)); pOut->Release())
			{
				PIN_DIRECTION dir;
				pOut->QueryDirection(&dir);
				if (dir == PINDIR_OUTPUT)
				{
					IPin* pTemp;
					if (SUCCEEDED(pOut->ConnectedTo(&pTemp)))
					{
						pTemp->Release();
					}
					else if (SUCCEEDED(m_pGraph->Render(pOut)))
					{
						hResult = S_OK;
					}
				}
			}
			pEnum->Release();
		}
		return hResult;
	}

	// Remove or reserve specified filter(s)
	HRESULT RemoveFilter(PCWSTR pwzName = NULL, BOOL bRemove = TRUE)
	{
		IEnumFilters* pEnum;
		HRESULT hResult = m_pGraph ? m_pGraph->EnumFilters(&pEnum) : E_POINTER;
		if (SUCCEEDED(hResult))
		{
			ULONG nCount;
			IBaseFilter* pFilters[1024];
			hResult = pEnum->Next(1024, pFilters, &nCount);
			if (SUCCEEDED(hResult))
			{
				for (UINT i = 0; i < nCount; pFilters[i++]->Release())
				{
					if (pwzName)
					{
						FILTER_INFO fi;
						if (SUCCEEDED(pFilters[i]->QueryFilterInfo(&fi)))
						{
							fi.pGraph->Release();
							if (bRemove != (_wcsicmp(fi.achName, pwzName) == 0))
							{
								continue;
							}
						}
					}
					hResult = m_pGraph->RemoveFilter(pFilters[i]);
				}
			}
			pEnum->Release();
		}
		return hResult;
	}

	// Remove previous or next filter
	HRESULT RemoveNextFilters(IBaseFilter* pFilter, PIN_DIRECTION dir)
	{
		IEnumPins* pEnum;
		HRESULT hResult = (m_pGraph && pFilter) ? pFilter->EnumPins(&pEnum) : E_POINTER;
		if (SUCCEEDED(hResult))
		{
			for (IPin* pPin = NULL; SUCCEEDED(pEnum->Next(1, &pPin, NULL)); pPin->Release())
			{
				IPin* pTemp;
				if (SUCCEEDED(pPin->ConnectedTo(&pTemp)))
				{
					PIN_INFO pi;
					if (SUCCEEDED(pTemp->QueryPinInfo(&pi)))
					{
						if (pi.dir == dir)
						{
							RemoveNextFilters(pi.pFilter, dir);
							m_pGraph->Disconnect(pTemp);
							m_pGraph->Disconnect(pPin);
							hResult = m_pGraph->RemoveFilter(pi.pFilter);
						}
						pi.pFilter->Release();
					}
					pTemp->Release();
				}
			}
			pEnum->Release();
		}
		return hResult;
	}

	//Get previous or next filter
	HRESULT GetNextFilter(IBaseFilter* pFilter, IBaseFilter** ppNext, PIN_DIRECTION dir = PINDIR_OUTPUT)
	{
		IEnumPins* pEnum;
		HRESULT hResult = (pFilter && ppNext) ? pFilter->EnumPins(&pEnum) : E_POINTER;
		if (SUCCEEDED(hResult))
		{
			hResult = E_FAIL;
			for (IPin* pPin = NULL; FAILED(hResult) && (SUCCEEDED(pEnum->Next(1, &pPin, 0))); pPin->Release())
			{
				PIN_DIRECTION cur;
				pPin->QueryDirection(&cur);
				if (cur == dir)
				{
					IPin* pTemp;
					hResult = pPin->ConnectedTo(&pTemp);
					if (SUCCEEDED(hResult))
					{
						PIN_INFO pi;
						hResult = pTemp->QueryPinInfo(&pi);
						*ppNext = pi.pFilter;
						pTemp->Release();
					}
				}
			}
			pEnum->Release();
		}
		return hResult;
	}

	// Lookup specified pin interface in filter
	HRESULT FindPinInterface(IBaseFilter* pFilter, REFIID iID, PVOID* ppUnknown)
	{
		IEnumPins* pEnum;
		HRESULT hResult = pFilter ? pFilter->EnumPins(&pEnum) : E_POINTER;
		if (SUCCEEDED(hResult))
		{
			hResult = E_NOINTERFACE;
			for (IPin* pPin = NULL; FAILED(hResult) && SUCCEEDED(pEnum->Next(1, &pPin, 0)); pPin->Release())
			{
				hResult = pPin->QueryInterface(iID, ppUnknown);
			}
			pEnum->Release();
		}
		return hResult;
	}

	// Lookup specified filter interface in the graph
	HRESULT FindFilterInterface(REFIID iID, PVOID* ppUnknown)
	{
		IEnumFilters* pEnum;
		HRESULT hResult = m_pGraph ? m_pGraph->EnumFilters(&pEnum) : E_POINTER;
		if (SUCCEEDED(hResult))
		{
			hResult = E_NOINTERFACE;
			for (IBaseFilter* pFilter = NULL; FAILED(hResult) && SUCCEEDED(pEnum->Next(1, &pFilter, 0)); pFilter->Release())
			{
				hResult = pFilter->QueryInterface(iID, ppUnknown);
			}
			pEnum->Release();
		}
		return hResult;
	}

	// Lookup any interface
	HRESULT FindInterface(REFIID iID, PVOID* ppUnknown)
	{
		IEnumFilters* pEnum;
		HRESULT hResult = m_pGraph ? m_pGraph->EnumFilters(&pEnum) : E_POINTER;
		if (SUCCEEDED(hResult))
		{
			hResult = E_NOINTERFACE;
			for (IBaseFilter* pFilter = NULL; FAILED(hResult) && SUCCEEDED(pEnum->Next(1, &pFilter, 0)); pFilter->Release())
			{
				hResult = pFilter->QueryInterface(iID, ppUnknown);
				if (FAILED(hResult))
				{
					hResult = FindPinInterface(pFilter, iID, ppUnknown);
				}
			}
			pEnum->Release();
		}
		return hResult;
	}

	// Render file
	HRESULT RenderFile(PCWSTR pwzFileName, HWND hVideoWnd = NULL)
	{
		HRESULT hResult = m_pGraph ? m_pGraph->RenderFile(pwzFileName, NULL) : E_POINTER;
		if (SUCCEEDED(hResult))
		{
			hResult = SetVideoWindow(hVideoWnd);
		}
		return hResult;
	}

public:
	// Set file to sink
	static HRESULT SetFile(IBaseFilter* pFilter, PCWSTR pwzFileName)
	{
		_QueryCall(pFilter, IFileSinkFilter, SetFileName(pwzFileName, 0));
		return hResult;
	}

	// Load source file
	static HRESULT LoadFile(IBaseFilter* pFilter, PCWSTR pwzFileName)
	{
		_QueryCall(pFilter, IFileSourceFilter, Load(pwzFileName, 0));
		return hResult;
	}

	// Show property page
	static HRESULT ShowProperty(IBaseFilter* pFilter, HWND hParent = NULL)
	{
#ifdef WINCE
		return E_NOTIMPL;
#else
		ISpecifyPropertyPages* pProp;
		HRESULT hResult = pFilter ? pFilter->QueryInterface(IID_ISpecifyPropertyPages, reinterpret_cast<PVOID*>(&pProp)) : E_POINTER;
		if (SUCCEEDED(hResult))
		{
			CAUUID cuPage;
			hResult = pProp->GetPages(&cuPage);
			if (SUCCEEDED(hResult))
			{
				FILTER_INFO fi;
				hResult = pFilter->QueryFilterInfo(&fi);
				if (SUCCEEDED(hResult))
				{
					IUnknown* pUnknown;
					hResult = pFilter->QueryInterface(IID_IUnknown, reinterpret_cast<PVOID*>(&pUnknown));
					if (SUCCEEDED(hResult))
					{
						OleCreatePropertyFrame(hParent, 0, 0, fi.achName, 1, &pUnknown, cuPage.cElems, cuPage.pElems, 0, 0, NULL);
						pUnknown->Release();
					}
					fi.pGraph->Release();
				}
				CoTaskMemFree(cuPage.pElems);
			}
			pProp->Release();
		}
		return hResult;
#endif
	}

	// Display filter information and return next filter
	static IBaseFilter* DisplayFilter(IBaseFilter* pFilter)
	{
		TCHAR tzInfo[10240];
		PTSTR ptzInfo = tzInfo;

		FILTER_INFO fi;
		pFilter->QueryFilterInfo(&fi);
		ptzInfo += wsprintf(ptzInfo, TEXT("Filter:%s\n\n"), fi.achName);
		fi.pGraph->Release();

		PIN_INFO pi2 = {0};

		PPIN pPin;
		ULONG nPin = 1;
		IEnumPins* pEnumPins;
		pFilter->EnumPins(&pEnumPins);

		while (pEnumPins->Next(1, &pPin, &nPin) == S_OK)
		{
			PIN_INFO pi;
			PIN_DIRECTION dr;
			AM_MEDIA_TYPE mt;
			pPin->ConnectionMediaType(&mt);
			pPin->QueryPinInfo(&pi);
			pPin->QueryDirection(&dr);

			TCHAR tzMT[60] = TEXT("Not Connected");
			TCHAR tzST[60] = TEXT("Not Connected");
			static const GUID GUID_ZERO = {0};
			if (mt.majortype != GUID_ZERO)
			{
				if (mt.majortype == MEDIATYPE_Video) lstrcpy(tzMT, TEXT("MEDIATYPE_Video"));
				//else StringFromGUID2(mt.majortype, tzMT, 60);

				if (mt.subtype == MEDIASUBTYPE_RGB565) lstrcpy(tzST, TEXT("MEDIASUBTYPE_RGB565"));
				else if (mt.subtype == MEDIASUBTYPE_RGB555) lstrcpy(tzST, TEXT("MEDIASUBTYPE_RGB555"));
				else if (mt.subtype == MEDIASUBTYPE_RGB24) lstrcpy(tzST, TEXT("MEDIASUBTYPE_RGB24"));
				//else StringFromGUID2(mt.subtype, tzST, 60);
			}

			ptzInfo += wsprintf(ptzInfo,
				TEXT("%s:%s\n")
				TEXT("MT:%s\n")
				TEXT("ST:%s\n\n"),
				(dr == PINDIR_OUTPUT) ? TEXT("OUT") : TEXT("IN"), pi.achName, tzMT, tzST);
			pi.pFilter->Release();

			//FreeMediaType(mt);
			CoTaskMemFree(mt.pbFormat);
			if (mt.pUnk) mt.pUnk->Release();

			if (dr == PINDIR_OUTPUT)
			{
				IEnumMediaTypes* pEnumTypes = NULL;
				pPin->EnumMediaTypes(&pEnumTypes);
				if (pEnumTypes)
				{
					AM_MEDIA_TYPE* pType;
					while (pEnumTypes->Next(1, &pType, NULL) == S_OK)
					{
						if ((pType->formattype == FORMAT_VideoInfo) && (pType->cbFormat == sizeof(VIDEOINFO)) && pType->pbFormat)
						{
							VIDEOINFO* pInfo = (VIDEOINFO*) pType->pbFormat;

							ptzInfo += wsprintf(ptzInfo, TEXT("%dx%d\n"), pInfo->bmiHeader.biWidth, pInfo->bmiHeader.biHeight);
						}

						//DeleteMediaType(pType);
						CoTaskMemFree(pType->pbFormat);
						if (pType->pUnk) pType->pUnk->Release();
						CoTaskMemFree(pType);
					}
					ptzInfo += wsprintf(ptzInfo, TEXT("\n"));
					pEnumTypes->Release();
				}

				IPin* pPin2 = NULL;
				pPin->ConnectedTo(&pPin2);
				if (pPin2)
				{
					if (pi2.pFilter)
					{
						pi2.pFilter->Release();
					}
					pPin2->QueryPinInfo(&pi2);
					pPin2->Release();
				}
			}
			pPin->Release();
		}

		pEnumPins->Release();

		MessageBox(NULL, tzInfo, TEXT("Filter Info"), MB_ICONINFORMATION | MB_TOPMOST);
		pFilter->Release();
		return pi2.pFilter;
	}

protected:
	IGraphBuilder* m_pGraph;
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
