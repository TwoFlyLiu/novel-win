// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once

#define _CRT_SECURE_NO_WARNINGS
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // �� Windows ͷ���ų�����ʹ�õ�����
// Windows ͷ�ļ�: 
#include <windows.h>
#include <objbase.h>

// C ����ʱͷ�ļ�
#include <malloc.h>
#include <memory.h>
#include <tchar.h>


// TODO:  �ڴ˴����ó�����Ҫ������ͷ�ļ�
//cpp ��׼��
#include <vector>
#include <cmath>
#include <locale>
#include <codecvt>
#include <thread>

#include <UIlib.h>
#include <Utils/WinImplBase.h>

using namespace DuiLib;

#ifdef _DEBUG
#	ifdef _UNICODE
#		pragma comment(lib, "DuiLib_ud.lib")
#	else
#		pragma comment(lib, "DuiLib_d.lib")
#	endif
#else
#	ifdef _UNICODE
#		pragma comment(lib, "DuiLib_u.lib")
#	else
#		pragma comment(lib, "DuiLib.lib")
#	endif
#endif

#include <string>
#include <sstream>
#include <algorithm>
#include <fstream>
#include <filesystem>

#ifdef _UNICODE
typedef std::wstring String;
typedef std::wstringstream StringConverter;
typedef std::wfstream FStream;
typedef std::wifstream FInputStream;
typedef std::wofstream FOutputStream;
#else
typedef std::string String;
typedef std::stringstream StringConverter;
typedef std::fstream FStream;
typedef std::ifstream FInputStream;
typedef std::ofstream FOutputStream;
#endif

#define  SEARCH_EXE _T("search.exe")

#define NOVEL_BASE _T("./data")
#define NOVEL_ICON_EXT _T(".img")
#define NOVEL_ICON_DIR NOVEL_BASE _T("/icons")

#define NOVEL_DIR NOVEL_BASE _T("/json")
#define NOVEL_EXT _T(".novel")

#define DOWNLOAD_RECORD NOVEL_BASE _T("/download_record")

#define  WM_UPDATE_PROCESS_VALUE (WM_USER + 1)
