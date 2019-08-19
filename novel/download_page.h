#pragma once

struct NovelBase;
class DuiLib::CPaintManagerUI;
class NovelMainWnd;

class DownloadPage : public CNotifyPump
{
	DUI_DECLARE_MESSAGE_MAP()
public:
	DownloadPage();
	~DownloadPage();

	void Init(CPaintManagerUI *paint_manager, NovelMainWnd *main_window, DWORD main_thread_id);
	void AddNovelBase(NovelBase *novel);
	void RemoveNovelBase(NovelBase *Novel);
	void RemoveNovelBase(const String &name, const String &author);

	void UpdateNovelBase(NovelBase *novel);
	void FilterDownloadList(const String & text);
	void FilterDownloadList(TNotifyUI & msg);
	void DownloadOrUpdate(NovelBase * novel_);
	void downloadOrUpdate(NovelBase * novel);

	void OnUpdateProgressValue(UINT uMsg, WPARAM wParam, LPARAM lParam);
	void createWorkThreadAndWork(const String &cmdline, NovelBase * novel);
	void updateProgressValue(const String &line, NovelBase * novel);

	void InitWindow();
	void InitDownloadMenu(HMENU main_menu);
	void loadFromNativeDownloadRecords();
	void saveDownloadRecords();
	void OnDownOrUpdateDone(NovelBase *novel);
	void OnRbuttonDown(UINT uMsg, WPARAM wParam, LPARAM lParam);
	bool OnMenu(int menu_id);

	void RemoveSelectedItem();
	void RemoveSelectedItemAndNativeNovel();
private:
	std::vector<NovelBase*> novels_;
	CPaintManagerUI *paint_manager_;
	NovelMainWnd *main_window_;
	DWORD main_thread_id_{ 0 };
	HMENU download_menu_{ NULL };
};

