#include "stdafx.h"
#include "p_open.h"

Popen::Popen(LPCTSTR cmdline) :cmdline_(cmdline), valid_size_(0), valid_offset_(0), stdout_reader_(NULL)
{
	ZeroMemory(&startup_info_, sizeof(startup_info_));
	ZeroMemory(&process_inormation_, sizeof(process_inormation_));

	startup_info_.cb = sizeof(startup_info_);
	startup_info_.dwFlags = STARTF_USESTDHANDLES;
}

BOOL Popen::Execute()
{
	// 创建匿名管道
	HANDLE handle_read, handle_write;

	// 一定要保证管道可以被继承
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(sa);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;
	
	handle_read = handle_write = NULL;
	if (!CreatePipe(&handle_read, &handle_write, &sa, 0))
	{
		return FALSE;
	}


	startup_info_.hStdOutput = handle_write;
	startup_info_.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	startup_info_.wShowWindow = SW_HIDE;
	// 子进程退继承父进程的标准输入和标准输出
	BOOL ret = CreateProcess(
		NULL,
		const_cast<LPTSTR>(cmdline_.c_str()),
		NULL,
		NULL,
		TRUE,
		0,
		NULL,
		NULL,
		&startup_info_,
		&process_inormation_
	);

	if (ret)
	{
		::CloseHandle(handle_write); //关掉写句柄，保留读句柄
	}

	stdout_reader_ = handle_read;
	return ret;
}

BOOL Popen::ReadLine(String &line, BOOL &eof)
{
	line.clear();
	eof = false;

	if (startup_info_.hStdOutput == NULL)
	{
		return FALSE;
	}

	std::string utf8_line;
	while (true)
	{
		// 先从本地缓存中进行读取
		if (ReadLineFromBuf(utf8_line))
		{
			std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> cvt;
			line = cvt.from_bytes(utf8_line);
			return TRUE;
		}

		BOOL ret = ::ReadFile(
			stdout_reader_,
			&buf_,
			sizeof(buf_) - 1,
			&valid_size_,
			NULL
		);

		if (!ret)
		{
			eof = (::GetLastError() == ERROR_BROKEN_PIPE);
			return ret;
		}
		buf_[valid_size_] = '\0';
	}

	return TRUE;
}

BOOL Popen::ReadLineFromBuf(std::string &line)
{
	DWORD ret = FALSE;

	for (; valid_offset_ < valid_size_; valid_offset_++)
	{
		if (buf_[valid_offset_] != _T('\n'))
		{
			line.push_back(buf_[valid_offset_]);
		}
		else
		{
			// 应该只有TRUE的时候，缓冲区中还有多余的内容
			line.push_back(_T('\n'));
			valid_offset_++;
			ret = TRUE;
			break;
		}
	}

	if (!ret)
	{
		valid_offset_ = 0;
		valid_size_ = 0;
	}
	return ret;
}


void Popen::Detach()
{
	detached_ = true;

	if (stdout_reader_)
	{
		CloseHandle(stdout_reader_);
	}
}

void Popen::Close()
{
	if (detached_)
	{
		return;
	}

	if (stdout_reader_)
	{
		CloseHandle(stdout_reader_);
	}
	::TerminateProcess(process_inormation_.hProcess, 0);
}

Popen::~Popen()
{
	Close();
}