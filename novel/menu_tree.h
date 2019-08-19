#pragma once

class MenuTree : public CListUI
{
public:
	MenuTree();
	virtual ~MenuTree();

	struct NodeData
	{
		int level_;
		bool expand_;
		CDuiString text_;
		CListLabelElementUI *list_element_;
	};

	class Node
	{
	private:
		typedef std::vector<Node*> Children;
		Children children_;
		Node *parent_;
		NodeData data_;

	private:
		void setParent(Node *parent);

	public:
		Node();
		explicit Node(NodeData data);
		Node(NodeData data, Node *parent);
		~Node();

		NodeData &Data();

		int  GetChildrenSize() const;

		Node *GetChild(int index);

		Node *GetParent();

		void SetParent(Node *parent);

		bool HasChildren() const;

		void AddChild(Node *child);

		void RemoveChild(Node *child);

		Node *GetLastChild();


	};

	// ֻ����ListLabelElementUI����
	bool Add(CControlUI *control);

	bool AddAt(CComboUI *control, int index);

	bool Remove(CControlUI* control, bool do_not_destroy/* =false */);

	void RemoveAll();

	void SetVisible(bool bVisible /* = true */);

	void SetInternVisible(bool bVisible /* = true */);

	// ÿ���������Ҫ�����Լ���DoEvent
	void DoEvent(TEventUI& event);

	Node *AddNode(LPCTSTR text, Node *parent = NULL);

	// �Ƴ��ڵ㣨�����ڲ����ݣ���ListLabelElement)
	bool RemoveNode(Node *node);

	void ExpandNode(Node *node, bool expand);

	// ����expander��X����Χ
	SIZE GetExpanderSizeX(Node *node) const;

private:
	Node *root_;
};

class DuiLib::CPaintManagerUI;
class NovelMainWnd;

class MenuTreePage : public CNotifyPump
{
	DUI_DECLARE_MESSAGE_MAP()
public:
	MenuTreePage();
	void Init(CPaintManagerUI *paint_manager_, NovelMainWnd *main_window);
	// ����List�е�����
	void OnItemClick(TNotifyUI &msg);

	// ˫��List�е�����
	void OnItemActivate(TNotifyUI &msg);

	// ѡ��ָ���˵���
	void SelectItem(int index);

protected:
	void switchPage(TNotifyUI &msg);
private:
	CPaintManagerUI *paint_manager_;
	NovelMainWnd *main_window_;
};
