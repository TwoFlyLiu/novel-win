// viewer.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "viewer.h"
#include "menu_page.h"
#include "stringer.h"

DUI_BEGIN_MESSAGE_MAP(ViewerMainWnd, WindowImplBase)
DUI_END_MESSAGE_MAP()

ViewerMainWnd::ViewerMainWnd() : menu_page_(new MenuPage), novel_name_(NULL)
{
	menu_page_->Init(&m_PaintManager, this);
	AddVirtualWnd(_T("menu_page"), menu_page_.get());
}


ViewerMainWnd::~ViewerMainWnd()
{
	// 避免内存泄露
	for (auto iter = download_record_.begin(); iter != download_record_.end(); ++iter) 
	{
		delete iter->second;
	}
}

DuiLib::CDuiString ViewerMainWnd::GetSkinFolder()
{
	return _T("skin");
}

DuiLib::CDuiString ViewerMainWnd::GetSkinFile()
{
	return _T("viewer_main_window.xml");
}

LPCTSTR ViewerMainWnd::GetWindowClassName(void) const
{
	return _T("ViewerMainWnd");
}

void ViewerMainWnd::OnFinalMessage(HWND hWnd)
{
	this->saveViewerRecord();
	::PostQuitMessage(0);
}

static std::string readFile(const char *path)
{
	FILE *file = fopen(path, "rb");
	if (!file)
		return std::string("");
	fseek(file, 0, SEEK_END);
	long size = ftell(file);
	fseek(file, 0, SEEK_SET);
	std::string text;
	text.resize(size);

	if (fread(const_cast<char*>(text.c_str()), 1, size, file) != (unsigned long)size)
		text.clear();
	fclose(file);
	return text;
}


void ViewerMainWnd::InitWindow()
{
	// 设置图标
	HICON icon = ::LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_VIEWER));
	::SendMessage(*this, WM_SETICON, ICON_BIG, (LPARAM)icon);
	::SendMessage(*this, WM_SETICON, ICON_SMALL, (LPARAM)icon);

	if (NULL == novel_name_ || _tcslen(novel_name_) == 0)
	{
		fatal_(_T("对不起，您没有指定命令行参数"));
		return;
	}

	String novel_path = String(NOVEL_DIR).append(_T("/")).append(novel_name_).append(NOVEL_EXT);

	// 如何novel_name是具体的novel文件路径
	String name(novel_name_);
	size_t ext_len = _tcslen(NOVEL_EXT);

	if (name.size() > ext_len && name.substr(name.size() - _tcslen(NOVEL_EXT)) == NOVEL_EXT)
	{
		if (std::experimental::filesystem::exists(novel_name_))
		{
			novel_path = novel_name_;
		}
		else
		{
			String msg;
			msg.append(_T("文件\""))
				.append(novel_name_)
				.append(_T("\"不存在"));
			fatal_(msg.c_str());
			return;
		}
	}
	else if (!std::experimental::filesystem::exists(novel_path))
	{
		String msg;
		msg.append(_T("小说\"")).append(novel_name_).append(_T("\"不存在！"));
		fatal_(msg.c_str());
		return;
	}

	this->initLineHeight();

	if (!parseJsonData(stringer::WString2String(novel_path))) return;
	updateUI();
	content_->SetFocus();
}

void ViewerMainWnd::View(LPTSTR lpCmdLine)
{
	novel_name_ = lpCmdLine;

	if (NULL == lpCmdLine) return;

	// 去除收尾多余的\"
	int len = _tcslen(lpCmdLine);
	LPTSTR end = lpCmdLine + len;

	if (*lpCmdLine == _T('\"'))
	{
		lpCmdLine += 1;
	}

	if (lpCmdLine == end)
	{
		novel_name_ = NULL;
		return;
	}

	if (*(end - 1) == _T('\"'))
	{
		*(end - 1) = _T('\0');
	}
	novel_name_ = lpCmdLine;
}

bool ViewerMainWnd::parseJsonData(const std::string &filepath)
{
	// 从文件中读取所有内容到字符串中
	std::string data = readFile(filepath.c_str());
	rapidjson::StringStream s(data.c_str());
	novel_.ParseStream(s);

	return true;
}

void ViewerMainWnd::updateUI()
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> cvt;

	ASSERT(novel_.HasMember("Menus") && novel_["Menus"].IsArray());
	auto &menus = novel_["Menus"];
	for (rapidjson::SizeType i = 0; i < menus.Size(); i++)
	{
		ASSERT(menus[i].HasMember("Name"));

		std::wstring unicode = cvt.from_bytes(menus[i]["Name"].GetString());
		if (unicode.size() >= 22)
		{
			unicode = unicode.substr(0, 22) + _T("...");
		}
		menu_page_->AddMenu(unicode.c_str());
	}

	if (!loadViewerRecord())
	{
		// 当文件不存在，要设置默认值
		auto download_record_item = new DownloadRecordItem;
		download_record_item->name = novel_name_;
		download_record_item->menu_index = 0;
		download_record_item->chapter_scroll_pos = 0;
		download_record_[novel_name_] = download_record_item;

		menu_page_->SelectItem(0);
		UpdateContent(0);
	}
}

void ViewerMainWnd::UpdateContent(int index)
{
	if (content_ == NULL)
	{
		content_ = static_cast<CRichEditUI*>(m_PaintManager.FindControl(_T("content")));
	}
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> cvt;
	if (content_ != NULL)
	{
		if (novel_["Chapters"][index].IsNull())
		{
			content_->SetText(_T(""));
			return;
		}

		auto title = string(novel_["Menus"][index]["Name"].GetString()) + "\n\n";
		auto content_text = novel_["Chapters"][index]["Content"].GetString();

		String ct(cvt.from_bytes(content_text));
		std::vector<String> lines, pure_lines;

		stringer::SplitLines(ct, lines);

		String line; 
		for (size_t i = 0; i < lines.size(); i++)
		{
			line.clear();
			stringer::TrimSpace(lines[i], line);
			if (!line.empty())
			{
				line.insert(0, _T("      "));
				pure_lines.push_back(line);
			}
		}

		stringer::Join(pure_lines, _T("\n\n"), ct);
		ct.insert(0, cvt.from_bytes(title)); //插入标题
		content_->SetText(ct.c_str());

		// 每次翻页，都将内容切换到首行
		auto pos = content_->GetScrollPos();
		pos.cy = 0;
		content_->SetScrollPos(pos);
		content_->SetFocus();
	}
}


LRESULT ViewerMainWnd::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (!menu_page_->OnKeyDown(uMsg, wParam, lParam))
	{
		this->HandleContentPage(uMsg, wParam, lParam);
	}

	bHandled = (wParam == VK_SPACE);
	return 0;
}


void ViewerMainWnd::HandleContentPage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	SIZE scroll_pos, scroll_range;
	RECT content_client;

	if (content_ == NULL)
	{
		return;
	}

	auto line_size = content_->GetVerticalScrollBar()->GetLineSize();

	switch (wParam)
	{
	case VK_HOME:
		scroll_pos = content_->GetScrollPos();
		scroll_pos.cy = 0;
		content_->SetScrollPos(scroll_pos);
		break;
	case VK_END:
		scroll_pos = content_->GetScrollRange();
		content_->SetScrollPos(scroll_pos);
		break;
	case VK_DOWN:
		scroll_range = content_->GetScrollRange();
		scroll_pos = content_->GetScrollPos();
		scroll_pos.cy = min(scroll_range.cy, scroll_pos.cy + line_height_);
		content_->SetScrollPos(scroll_pos);
		break;
	case VK_UP:
		scroll_pos = content_->GetScrollPos();
		scroll_pos.cy = max(0, scroll_pos.cy - line_height_);
		content_->SetScrollPos(scroll_pos);
		break;
	case VK_PRIOR:
		content_client = content_->GetClientPos();

		scroll_range = content_->GetScrollRange();
		scroll_pos = content_->GetScrollPos();
		scroll_pos.cy = max(0, scroll_pos.cy - (content_client.bottom - content_client.top - 2 * line_height_));
		content_->SetScrollPos(scroll_pos);
		break;
	case VK_SPACE:
	case VK_NEXT:
		content_client = content_->GetClientPos();

		scroll_range = content_->GetScrollRange();
		scroll_pos = content_->GetScrollPos();

		// 滚动时候，不完全滚动一页，保证最后2行文字放到行首
		scroll_pos.cy = min(scroll_range.cy, scroll_pos.cy + (content_client.bottom - content_client.top - 2 * line_height_));
		content_->SetScrollPos(scroll_pos);
		break;
	}
}


void ViewerMainWnd::initLineHeight()
{
	if (content_ == NULL)
	{
		content_ = static_cast<CRichEditUI*>(m_PaintManager.FindControl(_T("content")));
		if (content_ == NULL) return;
	}
	
	line_height_ = m_PaintManager.GetFontInfo(content_->GetFont())
		->tm.tmHeight;
}

static int __ttoi(String str)
{
	size_t index;
	while ((index = str.find(_T(',')) != String::npos))
	{
		str.erase(index, 1);
	}
	return _ttoi(str.c_str());
}

bool ViewerMainWnd::loadViewerRecord()
{
	FInputStream in(VIEWER_RECORD);
	if (!in)
	{
		return false;
	}
	std::locale loc("");
	in.imbue(loc);

	// 将本地文件内容加载到内存中
	TCHAR buf[BUFSIZ];
	std::vector<String> fields;
	while (in.getline(buf, BUFSIZ))
	{
		stringer::Split(String(buf), _T('|'), fields);
		if (fields.size() != 3)
		{
			continue;
		}

		auto download_record_item = new DownloadRecordItem;
		download_record_item->name = fields[0];
		download_record_item->menu_index = __ttoi(fields[1]);

		download_record_item->chapter_scroll_pos = __ttoi(fields[2]);
		download_record_[fields[0]] = download_record_item;
	}

	if (download_record_.find(novel_name_) == download_record_.end())
	{
		auto download_record_item = new DownloadRecordItem;
		download_record_item->name = novel_name_;
		download_record_item->menu_index = 0;
		download_record_item->chapter_scroll_pos = 0;
		download_record_[novel_name_] = download_record_item;
	}

	menu_page_->SelectItem(download_record_[novel_name_]->menu_index);
	UpdateContent(download_record_[novel_name_]->menu_index);

	auto size = content_->GetScrollPos();
	size.cy = download_record_[novel_name_]->chapter_scroll_pos;
	//content_->SetScrollPos(size);

	//延时设置滚动条位置
	//延时的时间，经过手动测试至少要大于350,然后更新上次richedit保存的滚动条位置才有效
	//richedit的滚动条范围应该也是延时计算的
	::SetTimer(*this, INIT_SCROLL_TIMERID, 350, NULL);

	return true;
}


void ViewerMainWnd::saveViewerRecord()
{
	FOutputStream out(VIEWER_RECORD);
	ASSERT(out && "Save Download Record failed!");

	std::locale loc("");
	out.imbue(loc);

	// 更新最大打开时的状态
	auto download_record_item = download_record_[novel_name_];
	download_record_item->menu_index = menu_page_->GetSelectItemIndex();

	download_record_item->chapter_scroll_pos = content_->GetScrollPos().cy;

	for (auto iter = download_record_.begin(); iter != download_record_.end(); ++iter)
	{
		out << iter->second->name << _T("|") << iter->second->menu_index << _T("|") << iter->second->chapter_scroll_pos << _T("\n");
	}
	out.close();
}


void ViewerMainWnd::InitScrollPos()
{
	auto size = content_->GetScrollPos();
	size.cy = download_record_[novel_name_]->chapter_scroll_pos;
	content_->SetScrollPos(size);
	menu_page_->SelectItem(download_record_[novel_name_]->menu_index); //这儿是强制滚动选中的菜单项
}


LRESULT ViewerMainWnd::MessageHandler(UINT uMsg, WPARAM wParam, LPARAM, bool&)
{
	if (WM_TIMER == uMsg && INIT_SCROLL_TIMERID == wParam)
	{
		InitScrollPos();
		::KillTimer(*this, INIT_SCROLL_TIMERID);
	}
	return FALSE;
}


LRESULT ViewerMainWnd::ResponseDefaultKeyEvent(WPARAM wParam)
{
	return TRUE;
}


void ViewerMainWnd::fatal_(LPCTSTR msg)
{
	::MessageBox(*this, msg, _T("错误"), MB_OK | MB_ICONERROR);
	::PostQuitMessage(0);
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

	std::unique_ptr<ViewerMainWnd> frame(new ViewerMainWnd);
	ASSERT(frame != NULL && "内存不足");

	frame->View(lpCmdLine);
	frame->Create(NULL, _T("小说阅读器"), UI_WNDSTYLE_FRAME, WS_EX_WINDOWEDGE);
	frame->CenterWindow();

	// 最大化显示窗口
	::SendMessage(*frame, WM_SYSCOMMAND, SC_MAXIMIZE, 0);

	// 更新滚动条位置
	//frame.InitScrollPos();

	CPaintManagerUI::MessageLoop(); //消息循环

	::CoUninitialize();

	frame.reset(); //显示删除指针，为了保证在下面函数调用之前，所有相关内存都释放掉
	_CrtDumpMemoryLeaks();

	return 0;
}