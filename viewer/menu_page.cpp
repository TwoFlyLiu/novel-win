#include "stdafx.h"
#include "menu_page.h"
#include "viewer.h"

DUI_BEGIN_MESSAGE_MAP(MenuPage, CNotifyPump)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_ITEMCLICK, OnItemClick)
DUI_END_MESSAGE_MAP()

MenuPage::MenuPage() : paint_manager_(NULL), main_window_(NULL), selected_item_index_(0)
{
}


MenuPage::~MenuPage()
{
}

void MenuPage::Init(CPaintManagerUI *paint_manager, ViewerMainWnd *main_window)
{
	paint_manager_ = paint_manager;
	main_window_ = main_window;
}

void MenuPage::AddMenu(LPCTSTR text)
{
	ASSERT(paint_manager_ && main_window_ && "paint manger and main window does not be setted!");
	auto menu_container = static_cast<CListUI*>(paint_manager_->FindControl(_T("menu_page")));
	ASSERT(menu_container && "menu page does not be setted correctly!");

	auto element = new CListLabelElementUI;
	element->SetText(text);
	menu_container->Add(element);
}

void MenuPage::SelectItem(int index)
{
	ASSERT(paint_manager_ && main_window_ && "paint manger and main window does not be setted!");
	auto menu_container = static_cast<CListUI*>(paint_manager_->FindControl(_T("menu_page")));
	menu_container->SelectItem(index);
	selected_item_index_ = index;

	int vline_size = menu_container->GetItemVLineSize();

	// list的行高是20像素, 保证第一次 菜单选中项在菜单页的首行
	menu_container->Scroll(0, index * 20);
}

void MenuPage::OnItemClick(TNotifyUI &msg)
{
	ASSERT(paint_manager_ && main_window_ && "paint manger and main window does not be setted!");
	auto menu_container = static_cast<CListUI*>(paint_manager_->FindControl(_T("menu_page")));

	selected_item_index_ = menu_container->GetItemIndex(msg.pSender);
	if (selected_item_index_ != -1)
	{
		main_window_->UpdateContent(selected_item_index_);
	}
}

BOOL MenuPage::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	BOOL ret = FALSE;
	switch (wParam)
	{
	case VK_RIGHT:
		this->NextChapter();
		ret = TRUE;
		break;
	case VK_LEFT:
		this->PrevChapter();
		ret = TRUE;
		break;
	}
	return ret;
}

void MenuPage::NextChapter()
{
	ASSERT(paint_manager_ && main_window_ && "paint manger and main window does not be setted!");
	auto menu_container = static_cast<CListUI*>(paint_manager_->FindControl(_T("menu_page")));

	if (menu_container->GetItemAt(selected_item_index_ + 1) != NULL)
	{
		menu_container->SelectItem(++selected_item_index_);
		main_window_->UpdateContent(selected_item_index_);
	}
}

void MenuPage::PrevChapter()
{
	ASSERT(paint_manager_ && main_window_ && "paint manger and main window does not be setted!");
	auto menu_container = static_cast<CListUI*>(paint_manager_->FindControl(_T("menu_page")));
	if (menu_container->GetItemAt(selected_item_index_ - 1) != NULL)
	{
		menu_container->SelectItem(--selected_item_index_);
		main_window_->UpdateContent(selected_item_index_);
	}
}

int MenuPage::GetSelectItemIndex()
{
	return selected_item_index_;
}

BOOL MenuPage::IsFocused()
{
	ASSERT(paint_manager_ && main_window_ && "paint manger and main window does not be setted!");
	auto menu_container = static_cast<CListUI*>(paint_manager_->FindControl(_T("menu_page")));
	return menu_container->IsFocused();
}
