#include "stdafx.h"
#include "file_path.h"

namespace filepath
{

	void ListDir(LPCTSTR path, std::vector<String> &output)
	{
		WIN32_FIND_DATA wfd;
		ZeroMemory(&wfd, sizeof(wfd));

		String find_name(path);
		find_name.append(_T("\\*"));

		HANDLE find = FindFirstFile(find_name.c_str(), &wfd);

		output.clear();
		if (INVALID_HANDLE_VALUE == find)
		{
			return;
		}

		do 
		{
			if (wfd.cFileName[0] == _T('.'))
			{
				continue;
			}
			output.push_back(String(path) + _T("\\") + wfd.cFileName);
		} while (FindNextFile(find, &wfd));
		FindClose(find);
	}

	std::vector<String> ListDir(LPCTSTR path)
	{
		std::vector<String> output;
		ListDir(path, output);
		return output;
	}

	void Basename(LPCTSTR filepath, String &output)
	{
		int i;
		output.clear();

		int filepath_size = _tcslen(filepath);
		for (i = filepath_size - 1; i >= 0; i--)
		{
			if (filepath[i] == _T('\\') || filepath[i] == _T('/'))
			{
				break;
			}
		}
		if (i < 0)
		{
			return;
		}
		else if (i + 1 < filepath_size)
		{
			output = filepath + i + 1;
		}
	}

	String Basename(LPCTSTR filepath)
	{
		String basename;
		Basename(filepath, basename);
		return basename;
	}

	void SplitExt(LPCTSTR filepath, std::vector<String> & output)
	{
		int filepath_size = _tcslen(filepath);
		output.clear();

		if (filepath_size == 0)
		{
			return;
		}

		String str_filepath = filepath;
		int dot_pos = str_filepath.find_last_of(_T('.'));

		///ee/e/ehshs
		if (dot_pos == String::npos)
		{
			output.push_back(filepath);
		}
		else
		{
			//.
			//a.
			//.a
			//a.a
			if (dot_pos != 0)
			{
				output.push_back(String(filepath, filepath + dot_pos));
				output.push_back(String(filepath + dot_pos + 1, filepath + filepath_size));
			}
		}
	}

	std::vector<String> SplitExt(LPCTSTR filepath)
	{
		std::vector<String> output;
		SplitExt(filepath, output);
		return output;
	}

	bool Exist(LPCTSTR filepath)
	{
		FStream file;

		file.open(filepath, FStream::in);
		return (!file == false);
	}

	bool Exist(const String &filepath)
	{
		return Exist(filepath.c_str());
	}

	void AbslolutePath(LPCTSTR path, String &absolute_path)
	{
		TCHAR current_directory[MAX_PATH];

		absolute_path = path;

		if (absolute_path[0] == '.')
		{
			::GetCurrentDirectory(MAX_PATH, current_directory);
			absolute_path.replace(0, 1, current_directory);
		}

		// ½«/Ìæ»»Îª\ 
		while (true)
		{
			auto pos = absolute_path.find(_T('/'));
			if (pos == absolute_path.npos)
			{
				break;
			}
			absolute_path.replace(pos, 1, _T("\\"));
		}
	}

	String AbslolutePath(LPCTSTR path)
	{
		String absolute_path;
		AbslolutePath(path, absolute_path);
		return absolute_path;
	}

}