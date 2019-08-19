#pragma once

class ViewerMainWnd;

class MenuPage : public CNotifyPump
{
	DUI_DECLARE_MESSAGE_MAP()
public:
	MenuPage();
	~MenuPage();

	void Init(CPaintManagerUI *paint_manager, ViewerMainWnd *main_window);
	void AddMenu(LPCTSTR text);
	void SelectItem(int index);
	void OnItemClick(TNotifyUI & msg);
	BOOL OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam);
	void NextChapter();
	void PrevChapter();
	int GetSelectItemIndex();
	BOOL IsFocused();
private:
	CPaintManagerUI *paint_manager_;
	ViewerMainWnd *main_window_;
	int selected_item_index_;
};

