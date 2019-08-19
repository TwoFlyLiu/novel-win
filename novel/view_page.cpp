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

	// ��������ӵ�С˵�Ƿ��ڹ����б���
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

// �����������������Ƴ����ص�novel
void ViewPage::RemoveNovelViewAndNativeFile(const String &name)
{
	auto iter = std::find_if(views_.begin(), views_.end(), [&name](const NovelView *const view) {
		return view->name == name;
	});
	if (iter != views_.end())
	{
		auto view = *iter;

		// �Ƴ������ļ�
		::DeleteFile(view->novel_path.c_str());
		::DeleteFile(view->bkimage.c_str());

		// �Ƴ��ڴ��е�����
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
		// ��ɾ���ļ�(����С˵�����ļ���ͼ���ļ�)
		::DeleteFile(view->novel_path.c_str());
		::DeleteFile(view->bkimage.c_str());

		// ��ɾ��ѡ��
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

	// ������������Ż�
	if (msg.pSender->GetText() == prev_filter_text_.c_str())
	{
		return;
	}

	// ���������
	for (auto view : views_)
	{
		container->Remove(view->item, true);
	}

	// Ȼ����ѡ����ʾָ������
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
		// �����˵�
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

	// ��ȡ��ָ��С˵��NovelBase��Ϣ���Ϳ��Խ���������
	NovelBase *info = main_window_->Search(view->name.c_str());
	if (!info)
	{
		::MessageBox(*this->main_window_, String(_T("���粻��")).append(view->name).c_str(), _T("����"), MB_OK | MB_ICONWARNING);
	}
	else
	{
		this->main_window_->DownloadOrUpdate(info); // ���и���
		// ����delete�ڴ棬�ڲ��й���ģ������ؽ��������ڴ�ᱻ�Զ��ͷŵ�
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

	// ���ȶ�λ����Ч�ļ��Ŀ�ͷ�����Ե�{�ͻ���
	fin.seekg(2);

	// Ȼ��������ȡ7��
	std::vector<String> lines;
	std::string utf8_line;
	int left_line_count = 7;

	while (left_line_count-- && std::getline(fin, utf8_line))
	{
		lines.push_back(stringer::UTF82WString(utf8_line.c_str()));
	}
	fin.close();

	// ��lines�е����ݽ�����NovelBase
	NovelBase *novel = new NovelBase;
	novel->kind = _T("����");
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