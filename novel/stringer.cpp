#include "stdafx.h"
#include "stringer.h"

namespace 
{
	template <typename T>
	void split(const String &src, const T &sep, std::vector<String> &result)
	{
		result.clear();
		String result_copy(src);
		size_t field_sep_pos = String::npos;

		while ((field_sep_pos = result_copy.find(sep)) != String::npos)
		{
			result.push_back(result_copy.substr(0, field_sep_pos));
			result_copy = result_copy.substr(field_sep_pos + 1);
		}
		result.push_back(result_copy.substr(0, field_sep_pos));
	}

	bool isblank_(TCHAR ch)
	{
		return ch == _T(' ') || ch == _T('\n') || ch == _T('\t');
	}
}

namespace stringer
{

	void Split(const String &src, TCHAR sep, std::vector<String> &result)
	{
		split(src, sep, result);
	}

	void Split(const String &src, const String & sep, std::vector<String> &result)
	{
		split(src, sep, result);
	}

	void TrimSpace(const String &src, String &output)
	{
		int beg, end;

		for (beg = 0; beg < (int)src.size() && isblank_(src[beg]); beg++);
		if (beg == src.size())
		{
			output.clear();
			return;
		}

		for (end = src.size() - 1; end >= 0 && isblank_(src[end]); end--);
		output = src.substr(size_t(beg), size_t(end -  beg + 1));
	}

	String TrimSpace(const String &src)
	{
		String result;
		TrimSpace(src, result);
		return result;
	}

	void TrimSpaceLeft(String &text)
	{
		int beg;
		for (beg = 0; beg < (int)text.size() && isblank_(text[beg]); beg++);
		if (beg == text.size())
		{
			text.clear();
			return;
		}
		text.erase(0, beg);
	}

	void TrimSpaceRight(String &text)
	{
		int end;
		for (end = text.size() - 1; end >= 0 && isblank_(text[end]); end--);
		if (end < 0)
		{
			text.clear();
			return;
		}
		text.erase(end + 1);
	}

	void SplitLines(const String &src, std::vector<String>&result)
	{
		result.clear();
		Split(src, _T('\n'), result);
	}

	std::vector<String> SplitLines(const String &src)
	{
		std::vector<String> result;
		SplitLines(src, result);
		return result;
	}

	void Join(const std::vector<String> strings, const String &sep, String &result)
	{
		result.clear();
		for (size_t i = 0; i < strings.size(); i++)
		{
			result += strings[i] + sep;
		}
	}

	String Join(const std::vector<String> strings, const String &sep)
	{
		String result;
		Join(strings, sep, result);
		return result;
	}
	// wstring => string
	std::string WString2String(const std::wstring& ws)
	{
		std::string strLocale = setlocale(LC_ALL, "");
		const wchar_t* wchSrc = ws.c_str();
		size_t nDestSize = wcstombs(NULL, wchSrc, 0) + 1;
		char *chDest = new char[nDestSize];
		memset(chDest, 0, nDestSize);
		wcstombs(chDest, wchSrc, nDestSize);
		std::string strResult = chDest;
		delete[]chDest;
		setlocale(LC_ALL, strLocale.c_str());
		return strResult;
	}

	// string => wstring
	std::wstring String2WString(const std::string& s)
	{
		std::string strLocale = setlocale(LC_ALL, "");
		const char* chSrc = s.c_str();
		size_t nDestSize = mbstowcs(NULL, chSrc, 0) + 1;
		wchar_t* wchDest = new wchar_t[nDestSize];
		wmemset(wchDest, 0, nDestSize);
		mbstowcs(wchDest, chSrc, nDestSize);
		std::wstring wstrResult = wchDest;
		delete[]wchDest;
		setlocale(LC_ALL, strLocale.c_str());
		return wstrResult;
	}

	std::wstring UTF82WString(const char *src)
	{
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> cvt;
		return cvt.from_bytes(src);
	}

	std::string WString2UTF8(const wchar_t *src)
	{
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> cvt;
		return cvt.to_bytes(src);
	}

}

