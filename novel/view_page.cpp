#include "stdafx.h"
#include "view_page.h"
#include "novel_base.h"
#include "novel.h"
#include "file_path.h"
#include "p_open.h"
#include "stringer.h"
#include "resource.h"

DUI_BEGIN_MESSAGE_MAP(ViewPage, CNotifyPump)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_SELECTCHANGED, OnSelectChanged)
	DUI_ON_MSGTYPE_CTRNAME(DUI_MSGTYPE_TEXTCHANGED, _T("filter_novel_name"), OnTextChanged)
DUI_END_MESSAGE_MAP()


ViewPage::ViewPage() : main_window_(NULL), paint_manager_(NULL), prev_selected_option_(NULL), view_menu_(NULL)
{
}


ViewPage::~ViewPage()
{
	for (auto view : views_)
	{
		delete view;
	}
}

void ViewPage::AddNovelView(NovelView *view)
{
	CTileLayoutUI *container = static_cast<CTileLayoutUI*>(paint_manager_->FindControl(_T("novels")));
	ASSERT(container && "Can't found TileLayout container");

	CDialogBuilder builder;
	CVerticalLayoutUI *element = static_cast<CVerticalLayoutUI*>(builder.Create(_T("novel_view.xml")));
	ASSERT(element && "Can't create element!");

	view->item = element;
	element->FindSubControl(_T("view_item_option"))->SetAttribute(_T("bkimage"), view->bkimage.c_str());
	element->FindSubControl(_T("name"))->SetAttribute(_T("text"), view->name.c_str());

	element->FindSubControl(_T("view_item_option"))->SetTag((UINT_PTR)view);

	views_.push_back(view);

	// 测名新添加的小说是否在过滤列表中
	auto filter_text_control = paint_manager_->FindControl(_T("filter_novel_name"));
	ASSERT(filter_text_control);

	String filter_text = filter_text_control->GetText();
	if (filter_text.empty() || view->name.find(filter_text) != String::npos)
	{
		container->Add(element);
	}
}

void ViewPage::RemoveNovelView(NovelView *view)
{
	CTileLayoutUI *container = static_cast<CTileLayoutUI*>(paint_manager_->FindControl(_T("novels")));
	ASSERT(container && "Can't found TileLayout container");

	container->Remove(view->item);
	auto iter = std::find(views_.begin(), views_.end(), view);
	if (iter != views_.end())
	{
		delete *iter;
		views_.erase(iter);
	}
}

// 根据书名和作者来移除本地的novel
void ViewPage::RemoveNovelViewAndNativeFile(const String &name)
{
	auto iter = std::find_if(views_.begin(), views_.end(), [&name](const NovelView *const view) {
		return view->name == name;
	});
	if (iter != views_.end())
	{
		auto view = *iter;

		// 移除本地文件
		::DeleteFile(view->novel_path.c_str());
		::DeleteFile(view->bkimage.c_str());

		// 移除内存中的内容
		RemoveNovelView(view);
	}
}

void ViewPage::RemoveCurrentSelectedNovel()
{
	auto control = prev_selected_option_;
	if (control == NULL) return;

	NovelView *view = (NovelView*)control->GetTag();

	if (view != NULL)
	{
		// 先删除文件(包含小说内容文件和图标文件)
		::DeleteFile(view->novel_path.c_str());
		::DeleteFile(view->bkimage.c_str());

		// 在删除选项
		RemoveNovelView(view);
		prev_selected_option_ = NULL;
	}
}

void ViewPage::Init(CPaintManagerUI *paint_manager, NovelMainWnd *main_window)
{
	paint_manager_ = paint_manager;
	main_window_ = main_window;
}

void ViewPage::InitWindow()
{
	auto novels = filepath::ListDir(NOVEL_DIR);
	
	for (auto &novel : novels)
	{
		this->AddNovelView(novel);
	}
}

void ViewPage::OnSelectChanged(TNotifyUI &msg)
{
	if (prev_selected_option_ != msg.pSender)
	{
		SelectItem(msg.pSender);

		if (prev_selected_option_ != NULL)
		{
			SelectItem(prev_selected_option_, false);
		}
		prev_selected_option_ = msg.pSender;
	}
}

void ViewPage::OnTextChanged(TNotifyUI &msg)
{
	CTileLayoutUI *container = static_cast<CTileLayoutUI*>(paint_manager_->FindControl(_T("novels")));

	// 这儿进行显著优化
	if (msg.pSender->GetText() == prev_filter_text_.c_str())
	{
		return;
	}

	// 先添加所有
	for (auto view : views_)
	{
		container->Remove(view->item, true);
	}

	// 然后有选择显示指定内容
	for (auto view : views_)
	{
		if (msg.pSender->GetText().GetLength() == 0 || view->name.find(msg.pSender->GetText()) != String::npos)
		{
			container->Add(view->item);
		}
	}

	prev_filter_text_ = msg.pSender->GetText();
}

void ViewPage::SelectItem(CControlUI *item, bool selected /*= true*/)
{
	if (NULL == item)
	{
		return;
	}

	CVerticalLayoutUI *parent = static_cast<CVerticalLayoutUI*>(item->GetParent()->GetParent());
	if (selected)
	{
		parent->SetAttribute(_T("bkcolor"), _T("#FF0000FF"));
		parent->FindSubControl(_T("name"))
			->SetAttribute(_T("textcolor"), _T("#FFFFFFFF"));
	}
	else
	{
		parent->SetAttribute(_T("bkcolor"), _T(""));
		parent->FindSubControl(_T("name"))
			->SetAttribute(_T("textcolor"), _T(""));
	}
}

void ViewPage::OnLbuttonDownDBLClick(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
	CControlUI *control = paint_manager_->FindControl(pt);

	if (control == prev_selected_option_)
	{
		this->ReadNovel(control);
	}
}

bool ViewPage::NovelExist(NovelBase * novel)
{
	auto iter = std::find_if(views_.cbegin(), views_.cend(), [novel](NovelView *const e)->bool {
		return (novel->name == e->name);
	});
	return (iter != views_.cend());
}

bool ViewPage::OnRbuttonDown(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
	CControlUI *control = paint_manager_->FindControl(pt);

	if (control == nullptr)
	{
		return false;
	}
	if (_tcscmp(_T("TileLayout"), control->GetClass()) == 0)
	{
		return true;
	}
	else if (prev_selected_option_ == control)
	{
		// 弹出菜单
		TrackPopupMenu(view_menu_, TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_TOPALIGN, pt.x, pt.y, 0, *main_window_, NULL);
		return true;
	}
	else if (_tcscmp(_T("Option"), control->GetClass()) == 0 && control->GetName() == _T("view_item_option"))
	{
		return true;
	}
	
	return false;
}

void ViewPage::InitViewMenu(HMENU novel_menu)
{
	view_menu_ = ::GetSubMenu(novel_menu, 0);
}

bool ViewPage::OnMenu(int menu_id)
{
	bool ret = false;

	switch (menu_id)
	{
	case ID_VIEW_READ:
		if (prev_selected_option_ != NULL)
		{
			ReadNovel(prev_selected_option_);
		}
		break;
	case ID_VIEW_REMOVE:
		RemoveCurrentSelectedNovel();
		break;
	case ID_VIEW_UPDATE:
		UpdateNovel();
		break;
	}
	return ret;
}

void ViewPage::ReadNovel(CControlUI * control)
{
	NovelView *view = (NovelView *)control->GetTag();
	ASSERT(view);

	//MessageBox(*main_window_, name, _T("itemactivate"), MB_ICONINFORMATION | MB_OK);
	String cmdline{_T("viewer.exe ")};
	cmdline.append(view->name);

	WinExec(stringer::WString2String(cmdline).c_str(), SW_SHOWMAXIMIZED);
}

void ViewPage::UpdateNovel()
{
	NovelView *view = (NovelView *)prev_selected_option_->GetTag();
	ASSERT(view);

	// 提取出指定小说的NovelBase信息，就可以进行下载了
	NovelBase *info = main_window_->Search(view->name.c_str());
	if (!info)
	{
		::MessageBox(*this->main_window_, String(_T("网络不畅")).append(view->name).c_str(), _T("警告"), MB_OK | MB_ICONWARNING);
	}
	else
	{
		this->main_window_->DownloadOrUpdate(info); // 进行更新
		// 不用delete内存，内部有管理的，当下载结束，改内存会被自动释放掉
	}
}

void ViewPage::AddNovelView(const String &novel_file)
{
	auto basename = filepath::Basename(novel_file.c_str());
	auto novel_name = filepath::SplitExt(basename.c_str())[0];

	auto novel_view = new NovelView;
	novel_view->novel_path = novel_file;
	novel_view->name = novel_name;
	novel_view->bkimage = String(NOVEL_ICON_DIR).append(_T("/")).append(novel_name).append(NOVEL_ICON_EXT);
	AddNovelView(novel_view);

}

static String extractValue(const String &key_value)
{
	auto size = key_value.size();
	auto last_left_quot = key_value.rfind(_T('\"'), size - 3);
	ASSERT(last_left_quot != String::npos);
	return key_value.substr(last_left_quot + 1, size - last_left_quot - 3);
}

NovelBase * ViewPage::extractNovelBase(const String &novel_path)
{
	std::ifstream fin(stringer::WString2String(novel_path));
	if (!fin)
	{
		return NULL;
	}

	// 首先定位到有效文件的开头，忽略掉{和换行
	fin.seekg(2);

	// 然后连续读取7行
	std::vector<String> lines;
	std::string utf8_line;
	int left_line_count = 7;

	while (left_line_count-- && std::getline(fin, utf8_line))
	{
		lines.push_back(stringer::UTF82WString(utf8_line.c_str()));
	}
	fin.close();

	// 将lines中的内容解析成NovelBase
	NovelBase *novel = new NovelBase;
	novel->kind = _T("更新");
	novel->progress_value = 0;
	novel->name = extractValue(lines[0]);
	novel->last_update_time = extractValue(lines[1]);
	novel->author = extractValue(lines[2]);
	novel->menu_url = extractValue(lines[3]);
	//novel->icon_path
	novel->last_chapter_name = extractValue(lines[5]);
	novel->novel_description = extractValue(lines[6]);
	novel->visible = true;

	return novel;
}