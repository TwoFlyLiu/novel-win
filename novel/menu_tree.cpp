#include "stdafx.h"
#include "menu_tree.h"
#include "novel.h"


MenuTree::MenuTree() : root_(NULL)
{	
	SetItemShowHtml(true);

	root_ = new Node;
	root_->Data().level_ = -1;
	root_->Data().expand_ = true;
	root_->Data().list_element_ = NULL;
}


MenuTree::~MenuTree()
{
	if (root_ != NULL)
	{
		delete root_;
	}
}

bool MenuTree::Add(CControlUI *control)
{
	if (control == NULL)
	{
		return false;
	}

	if (_tcscmp(control->GetClass(), DUI_CTR_LISTLABELELEMENT) != 0)
	{
		return false;
	}
	return CListUI::Add(control);
}

bool MenuTree::AddAt(CComboUI *control, int index)
{
	if (control == NULL) return false;
	if (_tcscmp(control->GetClass(), DUI_CTR_LISTLABELELEMENT) != 0) return false;
	return CListUI::AddAt(control, index);
}

bool MenuTree::Remove(CControlUI* control, bool do_not_destroy/* =false */)
{
	if (control == NULL) return false;
	if (_tcscmp(control->GetClass(), DUI_CTR_LISTLABELELEMENT) != 0) return false;
	if (reinterpret_cast<Node*>(static_cast<CListLabelElementUI*>(control->GetInterface(DUI_CTR_LISTLABELELEMENT))->GetTag()) == NULL)
		return CListUI::Remove(control, do_not_destroy);
	else
		return RemoveNode(reinterpret_cast<Node*>(static_cast<CListLabelElementUI*>(control->GetInterface(DUI_CTR_LISTLABELELEMENT))->GetTag()));
}

void MenuTree::RemoveAll()
{
	CListUI::RemoveAll();

	// 下面的代码，可能不需要执行
	for (int i = 0; i < root_->GetChildrenSize(); i++)
	{
		Node *child = root_->GetChild(i);
		RemoveNode(child);
	}
	delete root_;

	root_ = new Node;
	root_->Data().level_ = -1;
	root_->Data().expand_ = true;
	root_->Data().list_element_ = NULL;
}

void MenuTree::SetVisible(bool bVisible /* = true */)
{
	if (m_bVisible == bVisible)
	{
		return;
	}
	CControlUI::SetVisible(bVisible);
}

void MenuTree::SetInternVisible(bool bVisible /* = true */)
{
	CControlUI::SetInternVisible(bVisible);
}

void MenuTree::DoEvent(TEventUI& event)
{
	// 自定义自己的滚动操作
	CListUI::DoEvent(event);
}

MenuTree::Node * MenuTree::AddNode(LPCTSTR text, Node *parent /*= NULL*/)
{
	if (!parent) parent = root_;

	CListLabelElementUI *list_element = new CListLabelElementUI;
	Node *node = new Node;
	node->Data().level_ = parent->Data().level_ + 1;
	if (node->Data().level_ == 0)
	{
		node->Data().expand_ = true;
	}
	else
	{
		node->Data().expand_ = false;
	}
	node->Data().text_ = text;
	node->Data().list_element_ = list_element;

	// 对于非根节点，如果父亲没有展开，那么添加的孩子是不显示的
	if (parent != root_)
	{
		if (!(parent->Data().expand_ && parent->Data().list_element_->IsVisible()))
		{
			list_element->SetVisible(false);
		}
	}

	CDuiString html_text;

	// 以level来设置缩进
	html_text += _T("<x 6>");
	for (int i = 0; i < node->Data().level_; i++)
	{
		html_text += _T("<x 24>");
	}

	String text_str(text);
	if (node->Data().level_ < 1 && _T("下载") == text_str.substr(text_str.size() - 2))
	{
		if (node->Data().expand_)
		{
			html_text += _T("<v center><a><i tree_expand.png 2 1></i></a></v>");
		}
		else
		{
			html_text += _T("<v center><a><i tree_expand.png 2 0></i></a></v>");
		}
	}
	html_text += node->Data().text_;
	list_element->SetText(html_text);

	//Tag中，可以存放关联的数据
	list_element->SetTag((UINT_PTR)node);
	if (node->Data().level_ == 0)
	{
		list_element->SetBkImage(_T("file='tree_top.bmp' corner='2,1,2,1' fade='100'"));
	}

	int index = 0;
	if (parent->HasChildren())
	{
		Node *prev = parent->GetLastChild();
		index = prev->Data().list_element_->GetIndex() + 1;
	}
	else
	{
		if (root_ == parent) index = 0;
		else index = parent->Data().list_element_->GetIndex() + 1;
	}

	if (!CListUI::AddAt(list_element, index))
	{
		delete list_element;
		delete node;
		node = NULL;
	}
	parent->AddChild(node);
	return node;
}

bool MenuTree::RemoveNode(Node *node)
{
	if (node == NULL || node == root_)
	{
		return false;
	}
	CListUI::Remove(node->Data().list_element_);
	node->GetParent()->RemoveChild(node);
	delete node;
	return true;
}

void MenuTree::ExpandNode(Node *node, bool expand)
{
	if (!node || node == root_)
	{
		return;
	}

	if (node->Data().expand_ == expand)
	{
		return;
	}

	node->Data().expand_ = expand;

	CDuiString html_text;

	// 以level来设置缩进
	html_text += _T("<x 6>");
	for (int i = 0; i < node->Data().level_; i++)
	{
		html_text += _T("<x 24>");
	}

	// 非房间的节点是可以有孩子的
	if (node->Data().level_ < 1 && _tcsstr(node->Data().text_, _T("下载")) != NULL)
	{
		if (node->Data().expand_)
		{
			html_text += _T("<v center><a><i tree_expand.png 2 1></i></a></v>");
		}
		else
		{
			html_text += _T("<v center><a><i tree_expand.png 2 0></i></a></v>");
		}
	}
	html_text += node->Data().text_;
	node->Data().list_element_->SetText(html_text);

	if (!node->Data().list_element_->IsVisible())
	{
		return;
	}
	if (!node->HasChildren())
	{
		return;
	}

	// 直接通过索引来获取CListUI中的所有子控件
	Node *begin = node->GetChild(0);
	Node *end = node->GetLastChild();

	int begin_index = begin->Data().list_element_->GetIndex();
	int end_index = end->Data().list_element_->GetIndex();
	for (int i = begin_index; i <= end_index; i++)
	{
		CControlUI *control = GetItemAt(i);
		if (_tcscmp(control->GetClass(), DUI_CTR_LISTLABELELEMENT) == 0)
		{
			Node *parent_node = reinterpret_cast<Node*>(control->GetTag())->GetParent();
			bool visible = (parent_node->Data().expand_ && parent_node->Data().list_element_->IsVisible());

			// 这儿使用SetVisible，不要使用SetInternalVisible
			control->SetVisible(visible);
		}
	}

	// 重新绘制（更新）
	NeedUpdate();
}

SIZE MenuTree::GetExpanderSizeX(Node *node) const
{
	if (!node || node == root_)
	{
		return CDuiSize();
	}

	// 对于房间是没有expander的
	if (node->Data().level_ >= 1)
	{
		return CDuiSize();
	}

	SIZE expander = { 0 };
	expander.cx = 6 + 24 * node->Data().level_ - 4;
	expander.cy = expander.cx + 16 + 8;
	return expander;
}

void MenuTree::Node::setParent(Node *parent)
{
	parent_ = parent;
}

MenuTree::Node::Node(NodeData data, Node *parent) : data_(data), parent_(parent)
{

}

MenuTree::Node::Node(NodeData data) : data_(data)
{

}

MenuTree::Node::Node() : parent_(NULL)
{

}

MenuTree::Node::~Node()
{
	for (int i = 0; i < GetChildrenSize(); i++)
	{
		delete children_[i];
	}
}

MenuTree::NodeData & MenuTree::Node::Data()
{
	return data_;
}

int MenuTree::Node::GetChildrenSize() const
{
	return (int)children_.size();
}

MenuTree::Node * MenuTree::Node::GetChild(int index)
{
	return children_[index];
}

MenuTree::Node * MenuTree::Node::GetParent()
{
	return parent_;
}

void MenuTree::Node::SetParent(Node *parent)
{
	parent_ = parent;
}

bool MenuTree::Node::HasChildren() const
{
	return GetChildrenSize() > 0;
}

void MenuTree::Node::AddChild(Node *child)
{
	child->SetParent(this);
	children_.push_back(child);
}

void MenuTree::Node::RemoveChild(Node *child)
{
	for (auto iter = children_.begin(); iter != children_.end(); ++iter)
	{
		if (*iter == child)
		{
			children_.erase(iter);
			return;
		}
	}
}

MenuTree::Node * MenuTree::Node::GetLastChild()
{
	if (HasChildren())
	{
		return GetChild(GetChildrenSize() - 1)->GetLastChild();
	}
	else
	{
		return this;
	}
}

DUI_BEGIN_MESSAGE_MAP(MenuTreePage, CNotifyPump)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_ITEMCLICK, OnItemClick)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_ITEMACTIVATE, OnItemActivate)
DUI_END_MESSAGE_MAP()

MenuTreePage::MenuTreePage() : paint_manager_(NULL)
{

}

void MenuTreePage::Init(CPaintManagerUI *paint_manager, NovelMainWnd *main_window)
{
	main_window_ = main_window;
	paint_manager_ = paint_manager;
}

void MenuTreePage::OnItemClick(TNotifyUI &msg)
{
	MenuTree *menu_tree = static_cast<MenuTree*>(paint_manager_->FindControl(_T("menu_tree")));
	ASSERT(menu_tree && "Can't find MenuTree");

	if (menu_tree->GetItemIndex(msg.pSender) != -1)
	{
		if (_tcscmp(msg.pSender->GetClass(), DUI_CTR_LISTLABELELEMENT) == 0)
		{
			MenuTree::Node* node = (MenuTree::Node*)msg.pSender->GetTag();

			POINT pt = { 0 };
			::GetCursorPos(&pt);
			::ScreenToClient(*main_window_, &pt);
			pt.x -= msg.pSender->GetX(); //右边是控件的起始X位置
			SIZE sz = menu_tree->GetExpanderSizeX(node);
			if (pt.x >= sz.cx && pt.x <= sz.cy)
			{
				menu_tree->ExpandNode(node, !node->Data().expand_);
			}
		}
		switchPage(msg);
	}
}

void MenuTreePage::OnItemActivate(TNotifyUI &msg)
{
	MenuTree* menu_tree = static_cast<MenuTree*>(paint_manager_->FindControl(_T("menu_tree")));
	ASSERT(menu_tree && "Can't find MenuTree");

	if (menu_tree->GetItemIndex(msg.pSender) != -1)
	{
		if (_tcscmp(msg.pSender->GetClass(), DUI_CTR_LISTLABELELEMENT) == 0)
		{
			MenuTree::Node *node = (MenuTree::Node*)(msg.pSender->GetTag());
			menu_tree->ExpandNode(node, !node->Data().expand_);

			// 下面应该进行过滤，但是暂时没有实现
			if (1 == node->Data().level_)
			{
				//TODO: 过滤下载列表
			}
		}
	}
}

void MenuTreePage::SelectItem(int index)
{
	MenuTree* menu_tree = static_cast<MenuTree*>(paint_manager_->FindControl(_T("menu_tree")));
	ASSERT(menu_tree && "Can't find MenuTree");
	menu_tree->SelectItem(index, true, true);


	CTabLayoutUI *client_tabs = static_cast<CTabLayoutUI*>(paint_manager_->FindControl(_T("client_tabs")));
	ASSERT(client_tabs && "Can't found client_tabs");
	switch (index)
	{
	case 0:
		client_tabs->SelectItem(0);
		break;
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	{
		client_tabs->SelectItem(1);
		auto control = static_cast<CListLabelElementUI*>(menu_tree->GetItemAt(index));
		ASSERT(control != NULL);
		auto node = (MenuTree::Node*)(control->GetTag());
		main_window_->FilterDownloadList(String(node->Data().text_));
		break;
	}
	case 6:
		client_tabs->SelectItem(2);
		break;
	default:
		break;
	}
}

static String get_menu_text_(LPCTSTR full_menu_text)
{
	String menu_text(full_menu_text);
	auto index = menu_text.rfind(_T('}'));
	if (String::npos != index)
	{
		menu_text = menu_text.substr(index + 1);
	}
	return menu_text;
}

void MenuTreePage::switchPage(TNotifyUI &msg)
{
	CTabLayoutUI *client_tabs = static_cast<CTabLayoutUI*>(paint_manager_->FindControl(_T("client_tabs")));
	ASSERT(client_tabs && "Can't found client_tabs");

	if (_tcscmp(msg.pSender->GetClass(), DUI_CTR_LISTLABELELEMENT) == 0)
	{
		MenuTree::Node *node = (MenuTree::Node*)(msg.pSender->GetTag());

		if (node->Data().level_ == 0)
		{
			auto menu_text = get_menu_text_(node->Data().text_);
			if (_T("搜索") == menu_text)
			{
				client_tabs->SelectItem(0);
			}
			else if (_T("查看") == menu_text)
			{
				client_tabs->SelectItem(2);
			}
			else
			{
				client_tabs->SelectItem(1);
				main_window_->FilterDownloadList(msg);
			}
		}
		else
		{
			client_tabs->SelectItem(1);
			main_window_->FilterDownloadList(msg);
		}
	}
}
