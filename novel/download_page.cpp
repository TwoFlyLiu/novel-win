#include "stdafx.h"
#include "download_page.h"
#include "novel_base.h"
#include "menu_tree.h"
#include "novel.h"
#include "p_open.h"
#include "stringer.h"
#include "resource.h"

DUI_BEGIN_MESSAGE_MAP(DownloadPage, CNotifyPump)
DUI_END_MESSAGE_MAP()

DownloadPage::DownloadPage() : main_window_(NULL), paint_manager_(NULL)
{
}


DownloadPage::~DownloadPage()
{
	saveDownloadRecords(); //保存下载记录到本地

	for (auto novel : novels_)
	{
		if (novel != NULL)
		{
			delete novel;
		}
	}
	novels_.clear(); //删除所有元素
}

void DownloadPage::Init(CPaintManagerUI *paint_manager, NovelMainWnd *main_window, DWORD main_thread_id)
{
	paint_manager_ = paint_manager;
	main_window_ = main_window;
	main_thread_id_ = main_thread_id;
}

void DownloadPage::AddNovelBase(NovelBase *novel)
{
	CListUI *download_list = static_cast<CListUI*>(paint_manager_->FindControl(_T("download_list")));
	ASSERT(download_list && "Can't find download list");

	novels_.push_back(novel);

	CDialogBuilder builder;
	CListContainerElementUI *element = static_cast<CListContainerElementUI*>(builder.Create(_T("download_list_element.xml")));
	ASSERT(element && "Create ListContainerElement failed");
	novel->list_container_element = element;

	UpdateNovelBase(novel);
	download_list->AddAt(element, 0);
	download_list->SelectItem(download_list->GetItemIndex(element));
}

void DownloadPage::RemoveNovelBase(NovelBase *novel)
{
	CListUI *download_list = static_cast<CListUI*>(paint_manager_->FindControl(_T("download_list")));
	ASSERT(download_list && "Can't find download list");
	download_list->Remove(novel->list_container_element);

	auto iter = std::find(novels_.begin(), novels_.end(), novel);

	if (iter != novels_.end())
	{
		novels_.erase(iter);
		download_list->Remove(novel->list_container_element);
		delete novel;
	}
}


void DownloadPage::RemoveNovelBase(const String &name, const String &author)
{
	auto iter = std::find_if(novels_.begin(), novels_.end(), [&name, &author](NovelBase *const novel)->bool {
		return (novel->name == name && novel->author == author);
	});
	if (iter != novels_.end())
	{
		this->RemoveNovelBase(*iter);
	}
}

void DownloadPage::UpdateNovelBase(NovelBase *novel)
{
	CListContainerElementUI *element = novel->list_container_element;

	element->FindSubControl(_T("name"))->SetText(novel->name.c_str());
	element->FindSubControl(_T("author"))->SetText(novel->author.c_str());
	element->FindSubControl(_T("last_update_time"))->SetText(novel->last_update_time.c_str());
	element->FindSubControl(_T("last_chapter_name"))->SetText(novel->last_chapter_name.c_str());

	StringConverter converter;
	converter << novel->progress_value << _T("%");
	element->FindSubControl(_T("download_progress"))->SetText(converter.str().c_str());
	static_cast<CProgressUI*>(element->FindSubControl(_T("download_progress")))->SetValue(novel->progress_value);

	element->FindSubControl(_T("download_op"))->SetText(novel->kind.c_str());

	if (element->IsVisible() != novel->visible)
	{
		element->SetVisible(novel->visible);
	}
}

void DownloadPage::FilterDownloadList(const String &text)
{
	if (text == _T("下载操作"))
	{
		for (auto novel : novels_)
		{
			novel->visible = (novel->kind == _T("下载"));
			UpdateNovelBase(novel);
		}
	}
	else if (text == _T("更新操作"))
	{
		for (auto novel : novels_)
		{
			novel->visible = (novel->kind == _T("更新"));
			UpdateNovelBase(novel);
		}
	}
	else if (text == _T("已经完成"))
	{
		for (auto novel : novels_)
		{
			novel->visible = (novel->progress_value == 100);
			UpdateNovelBase(novel);
		}
	}
	else if (text == _T("正在下载"))
	{
		for (auto novel : novels_)
		{
			novel->visible = (novel->progress_value != 100);
			UpdateNovelBase(novel);
		}
	}
	else
	{
		for (auto novel : novels_)
		{
			novel->visible = true;
			UpdateNovelBase(novel);
		}
	}
}

void DownloadPage::FilterDownloadList(TNotifyUI & msg)
{
	ASSERT(_tcscmp(msg.pSender->GetClass(), DUI_CTR_LISTLABELELEMENT) == 0 && "Not ListLabelElement menu item");
	MenuTree::Node *node = (MenuTree::Node*)(msg.pSender->GetTag());

	String text = node->Data().text_;
	auto index = text.rfind(_T('}'));
	if (index != String::npos)
	{
		text = text.substr(index + 1);
	}

	FilterDownloadList(text);
}

void DownloadPage::DownloadOrUpdate(NovelBase * novel)
{
	novel->kind = (main_window_->NovelExist(novel)) ? _T("更新") : _T("下载");

	// 如果是更新，则要删除原来的条目
	if (_T("更新") == novel->kind)
	{
		this->RemoveNovelBase(novel->name, novel->author);
	}

	// 添加新的
	this->AddNovelBase(novel);
	this->downloadOrUpdate(novel);
}


void DownloadPage::downloadOrUpdate(NovelBase * novel)
{
	// 拼参数
	String cmdline(_T("backend.exe "));
	cmdline.append(novel->kind == _T("下载") ? _T("-g ") : _T("-u "))
		.append(_T("-d "))
		.append(NOVEL_DIR)
		.append(_T(" -e "))
		.append(NOVEL_EXT)
		.append(_T(" -ld "))
		.append(NOVEL_BASE)
		.append(_T(" "));
	
	if (novel->kind == _T("下载"))
		cmdline.append(novel->menu_url);
	else
		cmdline.append(novel->name);

	createWorkThreadAndWork(cmdline, novel);
}

// 处理WM_UPDATE_PROGRESS_VALUE, 是主线程
void DownloadPage::OnUpdateProgressValue(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	NovelBase *novel = (NovelBase*)wParam;
	CProgressUI *progress = (CProgressUI*)novel->list_container_element->FindSubControl(_T("download_progress"));
	if (progress != NULL)
	{
		StringConverter sc;
		if (progress->GetValue() != novel->progress_value)
		{
			progress->SetValue(novel->progress_value);
			sc.clear();
			sc << novel->progress_value << "%";
			progress->SetText(sc.str().c_str());
		}
	}
}

static void work(const String &cmdline, NovelBase *novel, DownloadPage *download_page)
{
	Popen exe(cmdline.c_str());

	if (!exe.Execute())
	{
		return;
	}

	BOOL eof = FALSE;
	String line;
	while (exe.ReadLine(line, eof))
	{
		download_page->updateProgressValue(line, novel);
	}
	if (line.size() == 0 && eof)
	{
		download_page->updateProgressValue(String{_T("<%100.0>")}, novel);
	}
	
	download_page->OnDownOrUpdateDone(novel);
}

void DownloadPage::createWorkThreadAndWork(const String &cmdline, NovelBase * novel)
{
	std::thread(work, cmdline, novel, this).detach();
}

void DownloadPage::updateProgressValue(const String &line, NovelBase * novel)
{
	// 提取取出进度值
	if (line[0] != _T('<'))
	{
		return;
	}
	double progress_value = _tcstof(line.substr(2).c_str(), NULL);
	novel->progress_value = (int)progress_value;

	::PostMessage(*main_window_, WM_UPDATE_PROCESS_VALUE, (WPARAM)novel, NULL);
}

void DownloadPage::InitWindow()
{
	if (std::experimental::filesystem::exists(DOWNLOAD_RECORD))
	{
		loadFromNativeDownloadRecords();
	}
}

void DownloadPage::InitDownloadMenu(HMENU main_menu)
{
	download_menu_ = ::GetSubMenu(main_menu, 1);
}

void DownloadPage::loadFromNativeDownloadRecords()
{
	// 对于宽字符，默认是C的locale环境，不识别中文，系统默认的识别中文
	FInputStream file(DOWNLOAD_RECORD);
	std::locale loc("");
	file.imbue(loc);

	TCHAR buf[BUFSIZ];

	while (file.getline(buf, BUFSIZ))
	{
		String line = stringer::TrimSpace(buf);

		std::vector<String> fields;
		stringer::Split(line, _T('|'), fields);
		
		if (fields.size() != 6)
		{
			continue;
		}
		NovelBase *base = new NovelBase;
		base->name = fields[0];
		base->author = fields[1];
		base->last_update_time = fields[2];
		base->last_chapter_name = fields[3];
		base->progress_value = 100;
		base->kind = fields[5];
		base->visible = true;

		this->AddNovelBase(base);
	}
}

void DownloadPage::saveDownloadRecords()
{
	FOutputStream outfile(DOWNLOAD_RECORD);
	std::locale loc(""); //默认是系统默认的locale，识别中文
	outfile.imbue(loc);

	if (!outfile)
	{
		return;
	}

	for (auto &novel : novels_)
	{
		if (novel->progress_value != 100)
		{
			continue;
		}

		outfile << novel->name << _T('|') << novel->author << _T('|')
			<< novel->last_update_time << _T('|') << novel->last_chapter_name << _T('|')
			<< novel->progress_value << _T('|') << novel->kind << _T('\n');
	}
	outfile.close();
}

void DownloadPage::OnDownOrUpdateDone(NovelBase *novel)
{
	if (novel->kind == _T("下载"))
	{
		main_window_->OnDownloadDone(novel);
	}
}

void DownloadPage::OnRbuttonDown(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CListUI *download_list = static_cast<CListUI*>(paint_manager_->FindControl(_T("download_list")));
	ASSERT(download_list);

	POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
	CControlUI *control = paint_manager_->FindControl(pt);

	if (-1 == download_list->GetCurSel())
	{
		return;
	}

	bool mouse_in_select_item = false;
	auto cur_selected_item = download_list->GetItemAt(download_list->GetCurSel());
	while(!mouse_in_select_item && control)
	{
		mouse_in_select_item = (cur_selected_item == control);
		control = control->GetParent();
	}

	// 只有光标在选中项上面，才会弹出快捷菜单
	if (mouse_in_select_item)
	{
		TrackPopupMenu(download_menu_, TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_TOPALIGN, pt.x, pt.y, 0, *main_window_, NULL);
	}
}

bool DownloadPage::OnMenu(int menu_id)
{
	bool ret = false;

	switch (menu_id)
	{
	case ID_DOWNLOAD_REMOVE:
		RemoveSelectedItem();
		break;
	case ID_DOWNLOAD_REMOVE_NOVEL:
		RemoveSelectedItemAndNativeNovel();
		break;
	}

	return ret;
}

void DownloadPage::RemoveSelectedItem()
{

	CListUI *download_list = static_cast<CListUI*>(paint_manager_->FindControl(_T("download_list")));
	ASSERT(download_list && download_list->GetCurSel() != -1);

	int cur_sel = download_list->GetCurSel();
	if (cur_sel == -1)
	{
		return;
	}

	// 删除list中的内容
	// 下面第二个参数如果为false, 可能会造成程序异常退出，因该算是Duilib库的bug
	download_list->RemoveAt(cur_sel, true);

	// 防止内存泄露
	auto to_remove_novel_idx = novels_.size() - 1 - cur_sel;
	auto to_remove_novel = novels_[to_remove_novel_idx];
	novels_.erase(novels_.begin() + to_remove_novel_idx);
	delete to_remove_novel;
}

void DownloadPage::RemoveSelectedItemAndNativeNovel()
{
	CListUI *download_list = static_cast<CListUI*>(paint_manager_->FindControl(_T("download_list")));
	ASSERT(download_list && download_list->GetCurSel() != -1);

	int cur_sel = download_list->GetCurSel();

	// 显示顺序和保存在内存中的顺序是逆序关系
	String name = novels_[novels_.size() - 1 - cur_sel]->name;
	
	RemoveSelectedItem();
	this->main_window_->RemoveNovelViewAndNativeFile(name);
}
