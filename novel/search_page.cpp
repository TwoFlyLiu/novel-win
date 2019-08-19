#include "stdafx.h"
#include "search_page.h"
#include "p_open.h"
#include "novel_base.h"
#include "stringer.h"
#include "novel.h"

DUI_BEGIN_MESSAGE_MAP(SearchPage, CNotifyPump)
	DUI_ON_CLICK_CTRNAME(_T("search_btn"), OnSearch)
	DUI_ON_CLICK_CTRNAME(_T("download_btn"), OnDownload)
DUI_END_MESSAGE_MAP()

SearchPage::SearchPage() : paint_manager_(NULL), main_window_(NULL), novel_(NULL)
{
}


SearchPage::~SearchPage()
{
}

void SearchPage::Init(CPaintManagerUI *paint_manager, NovelMainWnd *main_window)
{
	paint_manager_ = paint_manager;
	main_window_ = main_window;
}

void SearchPage::OnDownload(TNotifyUI &msg)
{
	main_window_->DownloadOrUpdate(novel_);
	novel_ = NULL;
	paint_manager_->FindControl(_T("search_result_container"))->SetVisible(false);
	paint_manager_->FindControl(_T("search_content"))->SetText(_T(""));
}

NovelBase *SearchPage::Search(LPCTSTR novel_name)
{

	// 拼命令行
	String cmdline(SEARCH_EXE);
	cmdline.append(_T(" -id="))
		.append(NOVEL_ICON_DIR)
		.append(_T(" -ie="))
		.append(NOVEL_ICON_EXT)
		.append(_T(" -ld="))
		.append(NOVEL_BASE)
		.append(_T(" "))
		.append(novel_name);

	Popen p(cmdline.c_str());
	BOOL execute_ret = FALSE;
	if (!(execute_ret = p.Execute()))
	{
		return NULL;
	}
	
	String line;
	String search_result;
	BOOL eof;
	while (p.ReadLine(line, eof))
	{
		search_result += line;
	}
	ASSERT(eof && "Read from search failed!");

	// 解析命令行返回的内容
	search_result = stringer::TrimSpace(search_result);
	if (search_result.size() > 0 && search_result != _T("None"))
	{
		auto ret = parseCmdline(search_result);
		if (ret == NULL)
		{
			ret = Search(novel_name); //防止参数解析不全
		}
		return ret;
	}

	return NULL;
}

void SearchPage::OnSearch(TNotifyUI &msg)
{
	CEditUI *search_content = static_cast<CEditUI*>(paint_manager_->FindControl(_T("search_content")));
	ASSERT(search_content && "Can't found search_content");
	static bool alread_reload = false;

	if (search_content->GetText().GetLength() == 0) return;

	auto novel = Search(search_content->GetText());
	if (novel != NULL)
	{
		UpdateSearchContent(novel);
		paint_manager_->FindControl(_T("search_result_container"))->SetVisible(true);
	}
	else
	{
		::MessageBox(*main_window_, search_content->GetText() + _T(" 没有搜索到"),  _T("提示"),MB_OK | MB_ICONINFORMATION);
	}
}

NovelBase *SearchPage::parseCmdline(const String &search_result)
{
	// 分割字符串
	std::vector<String> fields;
	stringer::Split(search_result, _T('|'), fields);

	ASSERT(fields.size() == 7 && "parse search result failed!");

	for (auto &field : fields)
	{
		if (stringer::TrimSpace(field).size() == 0)
		{
			return NULL;
		}
	}

	NovelBase *novel = new NovelBase;
	novel->menu_url = fields[0];
	novel->name = fields[1];
	novel->author = fields[2];
	novel->novel_description = fields[3];
	novel->last_update_time = fields[4];
	novel->last_chapter_name = fields[5];
	novel->icon_path = fields[6];
	novel->visible = true;
	novel->progress_value = 0;

	auto lines = stringer::SplitLines(novel->novel_description);
	for (auto &line : lines) {
		line = stringer::TrimSpace(line);
	}

	novel->novel_description.clear();
	for (size_t i = 0; i < lines.size() && i < 3; i++)
	{
		novel->novel_description += lines[i] + _T("\n\n");
	}
	if (lines.size() > 3)
	{
		novel->novel_description += _T("...");
	}

	return novel;
}


void SearchPage::UpdateSearchContent(NovelBase *novel)
{
	UpdateSearchContent(novel, stringer::Join(stringer::SplitLines(novel->novel_description), _T("\n\n")));

	if (novel_ != NULL)
	{
		delete novel_;
	}
	novel_ = novel;
}

void SearchPage::UpdateSearchContent(NovelBase *novel, const String &tooltip)
{
	paint_manager_->FindControl(_T("novel_icon"))->SetAttribute(_T("bkimage"), novel->icon_path.c_str());
	paint_manager_->FindControl(_T("novel_name"))->SetText(novel->name.c_str());
	paint_manager_->FindControl(_T("novel_author"))->SetText(novel->author.c_str());
	paint_manager_->FindControl(_T("novel_last_update_time"))->SetText(novel->last_update_time.c_str());
	paint_manager_->FindControl(_T("novel_last_chapter_name"))->SetText(novel->last_chapter_name.c_str());
	paint_manager_->FindControl(_T("novel_description"))->SetText(novel->novel_description.c_str());
	paint_manager_->FindControl(_T("novel_description"))->SetToolTip(tooltip.c_str());
	paint_manager_->FindControl(_T("download_btn"))->SetText(main_window_->NovelExist(novel) ? _T("更新") : _T("下载"));
}
