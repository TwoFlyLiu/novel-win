#pragma once

namespace stringer
{
	void Split(const String &src, TCHAR , std::vector<String> &result);
	void Split(const String &src, const String &, std::vector<String> &result);

	void TrimSpace(const String &src, String &output);
	String TrimSpace(const String &src);

	void TrimSpaceLeft(String &text);
	void TrimSpaceRight(String &text);

	void SplitLines(const String &src, std::vector<String>&result);
	std::vector<String> SplitLines(const String &src);

	void Join(const std::vector<String> strings, const String &sep, String &result);
	String Join(const std::vector<String> strings, const String &sep);

	std::string WString2String(const std::wstring& ws);
	std::wstring String2WString(const std::string& s);

	std::wstring UTF82WString(const char *src);
	std::string WString2UTF8(const wchar_t *src);
}
