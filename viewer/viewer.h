#pragma once

#include "resource.h"

class MenuPage;

class ViewerMainWnd : public WindowImplBase
{
	DUI_DECLARE_MESSAGE_MAP()

public:
	struct DownloadRecordItem
	{
		String name;
		int menu_index;
		int chapter_scroll_pos;
	};
	typedef std::map<String, DownloadRecordItem*> DownloadRecord;

	ViewerMainWnd();
	~ViewerMainWnd();

	virtual CDuiString GetSkinFolder() override;

	virtual CDuiString GetSkinFile() override;

	virtual LPCTSTR GetWindowClassName(void) const override;

	virtual void OnFinalMessage(HWND hWnd) override;


	virtual void InitWindow() override;
	void View(LPTSTR lpCmdLine);

private:
	bool parseJsonData(const std::string &filepath);
	void updateUI();

public:
	void UpdateContent(int index);

	virtual LRESULT OnKeyDown(UINT, WPARAM, LPARAM, BOOL& bHandled) override;
	void HandleContentPage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	void initLineHeight();
	bool loadViewerRecord();
	void saveViewerRecord();
	void InitScrollPos();

	virtual LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM, bool&) override;

	virtual LRESULT ResponseDefaultKeyEvent(WPARAM wParam) override;

protected:

	// 弹出错误消息，结束整个应用
	void fatal_(LPCTSTR msg);

public:
	std::unique_ptr<MenuPage> menu_page_;
	LPTSTR novel_name_;
	rapidjson::Document novel_;
	CRichEditUI *content_{ NULL };
	LONG line_height_{ 0 };
	DownloadRecord download_record_;
};