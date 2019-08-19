#pragma once
#include "stdafx.h"

class DuiLib::CListContainerElementUI;

struct NovelBase
{
	String name; //书名
	String author; //作者
	String last_update_time; //最后更新时间
	String last_chapter_name;//章节名称
	int progress_value; //下载进度值
	String kind; //下载类型：下载或者更新
	bool visible; //指定项是否可见

	String menu_url; //菜单页的网址
	String novel_description; //描述信息
	String icon_path;

	CListContainerElementUI *list_container_element;
};
