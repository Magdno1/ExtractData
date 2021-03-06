#include "StdAfx.h"
#include "YCStdioFile.h"
#include "Utils/ArrayUtils.h"

/// Constructor
YCStdioFile::YCStdioFile()
{
}

/// Destructor
YCStdioFile::~YCStdioFile()
{
	Close();
}

/// Mode to open the file in
///
/// @param file_path  File path
/// @param open_flags Mode
///
bool YCStdioFile::Open(LPCTSTR file_path, u32 open_flags)
{
	Close();

	if (lstrlen(file_path) >= MAX_PATH)
	{
		// Path is too long
		return false;
	}

	// Accessing Mode
	YCString mode;

	if (open_flags & typeBinary)
	{
		// Binary
		if (open_flags & modeRead)
		{
			mode = _T("rb");
		}

		if (open_flags & modeCreate)
		{
			if (open_flags & modeNoTruncate)
			{
				if (open_flags & modeReadWrite)
				{
					mode = _T("rb+");
				}
				else if (open_flags & modeWrite)
				{
					mode = _T("ab");
				}
			}
			else
			{
				if (open_flags & modeReadWrite)
				{
					mode = _T("wb+");
				}
				else if (open_flags & modeWrite)
				{
					mode = _T("wb");
				}
			}
		}
		else
		{
			if (open_flags & modeReadWrite)
			{
				mode = _T("ab+");
			}
			else if (open_flags & modeWrite)
			{
				mode = _T("ab");
			}
		}
	}
	else // Text
	{
		if (open_flags & modeRead)
		{
			mode = _T("r");
		}

		if (open_flags & modeCreate)
		{
			if (open_flags & modeNoTruncate)
			{
				if (open_flags & modeReadWrite)
				{
					mode = _T("r+");
				}
				else if (open_flags & modeWrite)
				{
					mode = _T("a");
				}
			}
			else
			{
				if (open_flags & modeReadWrite)
				{
					mode = _T("w+");
				}
				else if (open_flags & modeWrite)
				{
					mode = _T("w");
				}
			}
		}
		else
		{
			if (open_flags & modeReadWrite)
			{
				mode = _T("a+");
			}
			else if (open_flags & modeWrite)
			{
				mode = _T("a");
			}
		}
	}

	// Open File

	m_stream = _tfopen(file_path, mode);

	m_file_path = file_path;
	m_file_name = m_file_path.GetFileName();
	m_file_extension = m_file_path.GetFileExt();

	return m_stream != nullptr;
}

/// Close File
void YCStdioFile::Close()
{
	if (m_stream != nullptr)
	{
		fclose(m_stream);
		m_stream = nullptr;
	}
}

/// Read File
///
/// @param buffer    Buffer
/// @param read_size Read size
///
DWORD YCStdioFile::Read(void* buffer, u32 read_size)
{
	return fread(buffer, 1, read_size, m_stream);
}

/// Write File
///
/// @param buffer     Buffer
/// @param write_size Write size
///
DWORD YCStdioFile::Write(const void* buffer, u32 write_size)
{
	return fwrite(buffer, 1, write_size, m_stream);
}

/// Read a file line
///
/// @param buffer      Buffer
/// @param buffer_size Buffer size
///
LPTSTR YCStdioFile::ReadString(LPTSTR buffer, u32 buffer_size)
{
	return _fgetts(buffer, buffer_size, m_stream);
}

/// Read a file line
///
/// @param buffer Buffer
///
/// @remark Gets rid of the newline at the end
///
bool YCStdioFile::ReadString(YCString& buffer)
{
	bool  result = false;
	TCHAR szBuffer[1024];

	buffer = _T("");

	while (true)
	{
		if (ReadString(szBuffer, ArrayUtils::ArraySize(szBuffer)) == nullptr)
		{
			// Read until the end

			break;
		}

		buffer += szBuffer;

		result = true;

		if (szBuffer[lstrlen(szBuffer) - 1] == _T('\n'))
		{
			// Read until a newline character

			break;
		}
	}

	return result;
}

/// Writes a line into the file
void YCStdioFile::WriteString(LPCTSTR buffer)
{
	_fputts(buffer, m_stream);
}

/// Move the file pointer (Seek)
///
/// @param offset    Number of bytes to seek
/// @param seek_mode Seek mode
///
u64 YCStdioFile::Seek(s64 offset, u32 seek_mode)
{
	switch (seek_mode)
	{
	case begin:
		seek_mode = SEEK_SET;
		break;

	case current:
		seek_mode = SEEK_CUR;
		break;

	case end:
		seek_mode = SEEK_END;
		break;

	default:
		seek_mode = SEEK_SET;
		break;
	}

	if (_fseeki64(m_stream, offset, seek_mode))
	{
		return static_cast<u64>(_ftelli64(m_stream));
	}

	return 0;
}
