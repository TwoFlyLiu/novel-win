#pragma once

class NovelMainWnd;
class DuiLib::CPaintManagerUI;
struct NovelBase;

class ViewPage :
	public CNotifyPump
{
	DUI_DECLARE_MESSAGE_MAP()
public:
	struct NovelView
	{
		String bkimage;
		String name;
		String novel_path;
		CControlUI *item;
	};

public:
	ViewPage();
	~ViewPage();

	void AddNovelView(NovelView *view);
	void AddNovelView(const String &novel_file);
	void RemoveNovelView(NovelView *view);
	void RemoveNovelViewAndNativeFile(const String & name);

	void RemoveCurrentSelectedNovel();

	void Init(CPaintManagerUI *paint_manager, NovelMainWnd *main_window);
	void InitWindow();
	void OnSelectChanged(TNotifyUI &msg);
	void OnTextChanged(TNotifyUI &msg);

	void SelectItem(CControlUI *item, bool selected = true);
	void OnLbuttonDownDBLClick(UINT uMsg, WPARAM wParam, LPARAM lParam);
	bool NovelExist(NovelBase * novel);
	bool OnRbuttonDown(UINT uMsg, WPARAM wParam, LPARAM lParam);

	void InitViewMenu(HMENU novel_menu);
	bool OnMenu(int menu_id);
	void ReadNovel(CControlUI * control);
	void UpdateNovel();

protected:
	NovelBase * extractNovelBase(const String &novel_path);

private:
	NovelMainWnd *main_window_;
	CPaintManagerUI *paint_manager_;
	CControlUI *prev_selected_option_;
	std::vector<NovelView*> views_;
	HMENU view_menu_;
	String prev_filter_text_;
};

