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
	// ���������ܵ�
	HANDLE handle_read, handle_write;

	// һ��Ҫ��֤�ܵ����Ա��̳�
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
	// �ӽ����˼̳и����̵ı�׼����ͱ�׼���
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
		::CloseHandle(handle_write); //�ص�д��������������
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
		// �ȴӱ��ػ����н��ж�ȡ
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
			// Ӧ��ֻ��TRUE��ʱ�򣬻������л��ж��������
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