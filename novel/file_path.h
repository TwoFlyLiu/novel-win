#pragma once

namespace filepath
{

	// 遍历path下的所有文件
	void ListDir(LPCTSTR path, std::vector<String> &output);
	std::vector<String> ListDir(LPCTSTR path);

	void Basename(LPCTSTR filepath, String &output);
	String Basename(LPCTSTR filepath);

	void SplitExt(LPCTSTR filepath, std::vector<String> &output);
	std::vector<String> SplitExt(LPCTSTR filepath);

	// 检测文件是否存在
	bool Exist(LPCTSTR filepath);
	bool Exist(const String &filepath);

	void AbslolutePath(LPCTSTR path, String &absolute_path);
	String AbslolutePath(LPCTSTR path);
}
