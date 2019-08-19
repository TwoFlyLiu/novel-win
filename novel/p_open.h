#pragma once
class Popen
{
public:
	explicit Popen(LPCTSTR cmdline);

	// 执行时异步的，可以通过调用GetStdin, GetStdout来获取标准输入和输出流
	BOOL Execute();

	BOOL ReadLine(String &line, BOOL &eof);
	BOOL ReadLineFromBuf(std::string &line);
	void Detach();

	void Close();
	~Popen();
private:
	String cmdline_;
	STARTUPINFO startup_info_;
	PROCESS_INFORMATION process_inormation_;
	char buf_[BUFSIZ];
	DWORD valid_size_;
	DWORD valid_offset_;
	HANDLE stdout_reader_;
	bool detached_{ false };
};

