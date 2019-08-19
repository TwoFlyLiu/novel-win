#pragma once

class NovelMainWnd;
struct NovelBase;

class SearchPage : public CNotifyPump
{
	DUI_DECLARE_MESSAGE_MAP()
public:
	SearchPage();
	~SearchPage();

	void Init(CPaintManagerUI *paint_manager, NovelMainWnd *main_window);

	void OnDownload(TNotifyUI & msg);

	void OnSearch(TNotifyUI &msg);
	NovelBase *parseCmdline(const String &search_result);
	NovelBase *Search(LPCTSTR novel_name);

	void UpdateSearchContent(NovelBase * novel);
	void UpdateSearchContent(NovelBase *novel, const String &tooltip);
private:
	CPaintManagerUI *paint_manager_;
	NovelMainWnd *main_window_;
	NovelBase * novel_;
};

