// novel.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "novel.h"
#include "menu_tree.h"
#include "download_page.h"
#include "novel_base.h"
#include "view_page.h"
#include "search_page.h"
#include "resource.h"

DUI_BEGIN_MESSAGE_MAP(NovelMainWnd, WindowImplBase)
DUI_END_MESSAGE_MAP()

NovelMainWnd::NovelMainWnd() : menu_tree_page_(new MenuTreePage), 
	download_page_(new DownloadPage), view_page_(new ViewPage),
	search_page_(new SearchPage)
{
	DWORD this_thread_id = GetCurrentThreadId();

	menu_tree_page_->Init(&m_PaintManager, this);
	this->AddVirtualWnd(_T("menu_tree_page"), menu_tree_page_);

	download_page_->Init(&m_PaintManager, this, this_thread_id);
	this->AddVirtualWnd(_T("download_list_page"), download_page_);

	view_page_->Init(&m_PaintManager, this);
	this->AddVirtualWnd(_T("view_page"), view_page_);

	search_page_->Init(&m_PaintManager, this);
	this->AddVirtualWnd(_T("search_page"), search_page_);

	// 加载主菜单(这个菜单不手动销毁，让他一直保存到程序结束)
	HMENU novel_menu = ::LoadMenu(GetModuleHandle(NULL), MAKEINTRESOURCE(IDC_NOVEL));
	view_page_->InitViewMenu(novel_menu);
	download_page_->InitDownloadMenu(novel_menu);

	/*
	HICON normal_icon = ::LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_NOVEL));
	HICON small_icon = ::LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_SMALL));
	
	::SendMessage(*this, WM_SETICON, ICON_BIG, (LPARAM)normal_icon);
	::SendMessage(*this, WM_SETICON, ICON_SMALL, (LPARAM)small_icon);
	*/
}

NovelMainWnd::~NovelMainWnd()
{
	delete menu_tree_page_;
	delete download_page_;
	delete search_page_;
	delete view_page_;
}

DuiLib::CDuiString NovelMainWnd::GetSkinFolder()
{
	return _T("skin");
}

DuiLib::CDuiString NovelMainWnd::GetSkinFile()
{
	return _T("main_window.xml");
}

LPCTSTR NovelMainWnd::GetWindowClassName(void) const
{
	return _T("NovelMainWnd");
}

void NovelMainWnd::OnFinalMessage(HWND hWnd)
{
	PostQuitMessage(0);
}

CControlUI * NovelMainWnd::CreateControl(LPCTSTR pstrClass)
{
	if (_tcscmp(pstrClass, _T("MenuTree")) == 0)
	{
		return new MenuTree;
	}
	return NULL;
}

void NovelMainWnd::InitWindow()
{

	MenuTree *menu_tree = static_cast<MenuTree*>(m_PaintManager.FindControl(_T("menu_tree")));
	ASSERT(menu_tree && "Load MenuTree failed!");

	MenuTree::Node *menu;

	//menu_tree->AddNode(_T("{x 4}{i gameicons.png 18 3}{x 4}搜索"));
	menu_tree->AddNode(_T("{x 4}{i search.png}搜索"));

	menu = menu_tree->AddNode(_T("{x 4}下载"));
	menu_tree->AddNode(_T("{i download.png}已经完成"), menu);
	menu_tree->AddNode(_T("{i download.png}正在下载"), menu);
	menu_tree->AddNode(_T("{i download.png}下载操作"), menu);
	menu_tree->AddNode(_T("{i download.png}更新操作"), menu);

	menu_tree->AddNode(_T("{x 4}{i eye.png}查看"));
	menu_tree->SelectItem(0);

	view_page_->InitWindow();
	download_page_->InitWindow();

	HICON icon = ::LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_NOVEL));
	::SendMessage(*this, WM_SETICON, ICON_BIG, (LPARAM)icon);
	::SendMessage(*this, WM_SETICON, ICON_SMALL, (LPARAM)icon);
}

void NovelMainWnd::FilterDownloadList(const String& menu_text)
{
	download_page_->FilterDownloadList(menu_text);
}

void NovelMainWnd::FilterDownloadList(TNotifyUI & msg)
{
	download_page_->FilterDownloadList(msg);
}

LRESULT NovelMainWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_LBUTTONDBLCLK)
	{
		view_page_->OnLbuttonDownDBLClick(uMsg, wParam, lParam);
	}
	else if (uMsg == WM_UPDATE_PROCESS_VALUE) // 用来同步滚动条的当前值
	{
		download_page_->OnUpdateProgressValue(uMsg, wParam, lParam);
	}
	else if (uMsg == WM_RBUTTONDOWN)
	{
		if (!view_page_->OnRbuttonDown(uMsg, wParam, lParam))
			download_page_->OnRbuttonDown(uMsg, wParam, lParam);
	}
	else if (uMsg == WM_COMMAND && HIWORD(wParam) == 0 && 0 == lParam)
	{
		this->OnMenu(LOWORD(wParam));
	}
	return WindowImplBase::HandleMessage(uMsg, wParam, lParam);
}

void NovelMainWnd::DownloadOrUpdate(NovelBase * novel_)
{
	menu_tree_page_->SelectItem(1);
	download_page_->DownloadOrUpdate(novel_);
}

bool NovelMainWnd::NovelExist(NovelBase *novel)
{
	return view_page_->NovelExist(novel);
}

void NovelMainWnd::OnDownloadDone(NovelBase * novel)
{
	auto view = new ViewPage::NovelView;
	view->bkimage = novel->icon_path;
	view->name = novel->name;
	view->novel_path = String(NOVEL_DIR).append(_T("/")).append(novel->name).append(NOVEL_EXT);
	view_page_->AddNovelView(view);
}

void NovelMainWnd::OnMenu(int menu_id)
{
	if (!download_page_->OnMenu(menu_id))
	{
		view_page_->OnMenu(menu_id);
	}
}

NovelBase *NovelMainWnd::Search(LPCTSTR novel_name)
{
	return search_page_->Search(novel_name);
}

void NovelMainWnd::RemoveNovelViewAndNativeFile(const String &name)
{
	this->view_page_->RemoveNovelViewAndNativeFile(name);
}

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nShowCmd)
{
	CPaintManagerUI::SetInstance(hInstance);
	HRESULT hr = ::CoInitialize(NULL); //用到com组件必须要先进行初始化
	if (FAILED(hr))
		return 0;

	// 设置资源路径是exe文件所在的路径
	//CPaintManagerUI::SetResourcePath(CPaintManagerUI::GetInstancePath());

	::SetCurrentDirectory(CPaintManagerUI::GetInstancePath());

	NovelMainWnd *frame = new NovelMainWnd;
	frame->Create(NULL, _T("小说搜索下载阅读器"), UI_WNDSTYLE_FRAME, WS_EX_WINDOWEDGE);
	frame->CenterWindow();

	// 最大化显示窗口
	::SendMessage(*frame, WM_SYSCOMMAND, SC_MAXIMIZE, 0);

	CPaintManagerUI::MessageLoop(); //消息循环
	delete frame;

	::CoUninitialize();
	_CrtDumpMemoryLeaks();
	return 0;
}