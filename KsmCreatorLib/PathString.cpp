//////////////////////////////////////////////////////////////////////////
// (c) 2015 Overclocked Games LLC
// KSMCREATOR PUBLIC LICENSE
//
// KSMCREATOR PUBLIC LICENSE TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION
//
// Do whatever you like with the original work, just don't be a dick.
//
// Being a dick includes - but is not limited to - the following instances:
//
// 1a. Outright copyright infringement - Don't just copy this and change the name.
// 1b. Selling the unmodified original with no work done what-so-ever, that's REALLY being a dick.
// 1c. Modifying the original work to contain hidden harmful content. That would make you a PROPER dick.
//
// If you become rich through modifications, related works/services, or supporting the original work, share the 
// love. Only a dick would make loads off this work and not buy the original works creator(s) a pint.
//
// Code is provided with no warranty. Using somebody else's code and bitching when it goes wrong makes you a    
// DONKEY dick. Fix the problem yourself. A non-dick would submit the fix back.
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PathString.h"

const char g_VolumeSeparator = ':';
const char g_PathSeparator = '\\';
const char g_AltPathSeparator = '/';

PathString::PathString(void)
{
	Assign("");
}

PathString::PathString( const char* strPath )
{
	Assign(strPath);
}

PathString::PathString( const wchar_t* strPath )
{
	Assign(strPath);
}


PathString::PathString( const PathString& ps )
{
	Assign(static_cast<const wchar_t*>(ps));
}

PathString::PathString( const wchar_t* strPath1, const wchar_t* strPath2 )
{
	wchar_t temp[MAX_PATH];
	temp[0] = '\0';
	
	wcsncat_s(temp, strPath1, wcslen(strPath1));
	wcsncat_s(temp, L"\\", 2);
	wcsncat_s(temp, strPath2, wcslen(strPath2));

	Assign(temp);
}

PathString::PathString( const char* strPath1, const char* strPath2 )
{
	char temp[MAX_PATH];
	temp[0] = '\0';

	strcat_s(temp, strPath1);
	strcat_s(temp, "\\");
	strcat_s(temp, strPath2);

	Assign(temp);
}

PathString::~PathString(void)
{}

PathString& PathString::operator=( const PathString& rhs )
{
	Assign( static_cast<const wchar_t*>(rhs));
	return *this;
}

PathString& PathString::operator=( const wchar_t* rhs )
{
	Assign(rhs);
	return *this;
}

PathString::operator const wchar_t*() const
{
	return _strPath;
}

void PathString::Assign( const char* strPath )
{
	size_t origsize = strlen(strPath) + 1;
	size_t convertedChars = 0;
	wchar_t wcstring[MAX_PATH];
	mbstowcs_s(&convertedChars, wcstring, origsize, strPath, _TRUNCATE);

	Assign(wcstring);
}

void PathString::AssignUTF8( const char* strPath )
{
	int srcLength = static_cast<int>(strlen(strPath)) + 1;
	int length = MultiByteToWideChar(CP_UTF8, 0, strPath, srcLength, 0, 0);
	wchar_t *outputBuffer = new wchar_t [length];
	MultiByteToWideChar(CP_UTF8, 0, strPath, srcLength, outputBuffer, length);

	Assign(outputBuffer);
}

void PathString::Assign( const wchar_t* strPath )
{
	// Copy in our new path if one is provided
	if (strPath != nullptr)
	{
		wcscpy_s( _strPath, strPath );
	}
	// TODO: should null strPath be valid?	

	// Convert all slashes to the same slash
	wchar_t* strSrc = _strPath;
	while (*strSrc != '\0')
	{
		if (*strSrc == g_AltPathSeparator)
		{
			*strSrc = g_PathSeparator;
		}
		++strSrc;
	}

	// Look for the file extension
	_strExtension = wcsrchr(_strPath, '.');

	// Look for the file name
	_strFileName = wcsrchr(_strPath, g_PathSeparator);
	if (_strFileName != nullptr)
	{
		++_strFileName;
	}
	else
	{
		_strFileName = _strPath;
	}
}

const char* PathString::ToCStr()
{
	// Create the c_str copy
	size_t origsize = wcslen(_strPath) + 1;
	size_t convertedChars = 0;
	wcstombs_s(&convertedChars, _cstrPath, origsize, _strPath, _TRUNCATE);

	return _cstrPath;
}

const wchar_t* PathString::GetExtension()
{
	return _strExtension;
}

void PathString::ChangeExtension(const wchar_t* newExtension)
{
	size_t extLen = wcslen(newExtension);
	if (wcslen(_strExtension) == extLen)
	{
		memcpy_s(_strExtension, (extLen * sizeof(wchar_t)), newExtension, (extLen * sizeof(wchar_t)));
	}
}

const wchar_t* PathString::ToWCStr()
{
	return &_strPath[0];
}

const wchar_t* PathString::GetFileNameOnly()
{
	std::wstring tmp = _strFileName;
	std::wstring tmp2 = _strExtension;

	_fileNameOnly = tmp.substr(0, tmp.length() - tmp2.length());
	return _fileNameOnly.c_str();
}

const wchar_t* PathString::GetFullFileName()
{
	_fileNameOnly = _strFileName;
	return _fileNameOnly.c_str();
}

const char* PathString::GetFullFileNameN()
{
	size_t origsize = wcslen(_strFileName) + 1;
	size_t convertedChars = 0;
	wcstombs_s(&convertedChars, _cstrPath, origsize, _strFileName, _TRUNCATE);

	return _cstrPath;
}

size_t PathString::CStrLen()
{
	ToCStr();
	return strlen(_cstrPath);
}

size_t PathString::WStrLen()
{
	return wcslen(_strPath);
}

const wchar_t* PathString::GetDirectory()
{
	std::wstring tmp(_strPath);

	_directoryOnly = tmp.substr(0, tmp.length() - wcslen(_strFileName));
	return _directoryOnly.c_str();
}
