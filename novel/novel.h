#pragma once

#include "resource.h"

class MenuTreePage;
class DownloadPage;
class ViewPage;
class SearchPage;
struct NovelBase;

class NovelMainWnd : public WindowImplBase 
{
	DUI_DECLARE_MESSAGE_MAP()

public:
	NovelMainWnd();
	virtual ~NovelMainWnd();


	virtual CDuiString GetSkinFolder() override;

	virtual CDuiString GetSkinFile() override;

	virtual LPCTSTR GetWindowClassName(void) const override;

	virtual void OnFinalMessage(HWND hWnd) override;

	CControlUI *CreateControl(LPCTSTR pstrClass) override;

	virtual void InitWindow() override;
	void FilterDownloadList(const String & menu_text);
	void FilterDownloadList(TNotifyUI & msg);

	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
	void DownloadOrUpdate(NovelBase * novel_);
	bool NovelExist(NovelBase *novel);
	void OnDownloadDone(NovelBase * novel);

	void OnMenu(int menu_id);
	NovelBase * Search(LPCTSTR novel_name);
	void RemoveNovelViewAndNativeFile(const String &name);
private:
	MenuTreePage *menu_tree_page_;
	DownloadPage *download_page_;
	ViewPage *view_page_;
	SearchPage *search_page_;
};
