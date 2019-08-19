#pragma once
#include "stdafx.h"

class DuiLib::CListContainerElementUI;

struct NovelBase
{
	String name; //����
	String author; //����
	String last_update_time; //������ʱ��
	String last_chapter_name;//�½�����
	int progress_value; //���ؽ���ֵ
	String kind; //�������ͣ����ػ��߸���
	bool visible; //ָ�����Ƿ�ɼ�

	String menu_url; //�˵�ҳ����ַ
	String novel_description; //������Ϣ
	String icon_path;

	CListContainerElementUI *list_container_element;
};
