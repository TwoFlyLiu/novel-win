#pragma once

namespace filepath
{

	// ����path�µ������ļ�
	void ListDir(LPCTSTR path, std::vector<String> &output);
	std::vector<String> ListDir(LPCTSTR path);

	void Basename(LPCTSTR filepath, String &output);
	String Basename(LPCTSTR filepath);

	void SplitExt(LPCTSTR filepath, std::vector<String> &output);
	std::vector<String> SplitExt(LPCTSTR filepath);

	// ����ļ��Ƿ����
	bool Exist(LPCTSTR filepath);
	bool Exist(const String &filepath);

	void AbslolutePath(LPCTSTR path, String &absolute_path);
	String AbslolutePath(LPCTSTR path);
}
