

// CeleLib 2.0.226
// Copyright (C) Yonsm 2005-2011, All Rights Reserved.
#pragma once

#include "UniBase.h"

#include "CeleAbout.h"
#include "CeleAuto.h"
#include "CeleCfg.h"
#include "CeleCom.h"
#include "CeleMemDC.h"
#include "CeleMemFile.h"
#include "CeleReg.h"
#include "CeleShortcut.h"
#include "CeleThunk.h"
#include "CeleTouch.h"
#include "CeleUtil.h"
#include "CeleXml.h"
#include "CeleCtrl.h"

#if (!defined(_WIN32_WCE) || (_WIN32_WCE >= 0x0500))
#include "CeleGraph.h"
#endif

#ifdef WINCE
#include "CeleTapi.h"
#include <TpcShell.h>
#pragma comment(lib, "CommCtrl.lib")
#else
#include "CeleIni.h"
#pragma comment(lib, "ComCtl32.lib")
#endif

#include <ShlObj.h>
#include <CommCtrl.h>
