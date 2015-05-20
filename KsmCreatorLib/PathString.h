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

#pragma once

class PathString
{
public:
	PathString(void);
	PathString(const char* strPath);
	PathString(const wchar_t* strPath);
	PathString(const wchar_t* strPath1, const wchar_t* strPath2);
	PathString(const char* strPath1, const char* strPath2);
	PathString(const PathString& ps);
	PathString& operator=(const PathString& rhs);
	PathString& PathString::operator=( const wchar_t* rhs );

	// Used to match type PathHelper to type const wchar_t*
	operator const wchar_t*() const;

	const char* ToCStr();

	const wchar_t* ToWCStr();

	size_t CStrLen();

	size_t WStrLen();

	const wchar_t* GetExtension();

	const wchar_t* GetDirectory();

	void ChangeExtension(const wchar_t* newExtension);

	const wchar_t* GetFileNameOnly();

	const wchar_t* GetFullFileName();

	const char* PathString::GetFullFileNameN();

	void Assign(const char* strPath);
	void Assign(const wchar_t* strPath);
	void AssignUTF8(const char* strPath);

	~PathString(void);

private:
	wchar_t _strPath[8000];
	char _cstrPath[8000];

	// points to the . before the file extension
	wchar_t* _strExtension;

	// points to the first character in the filename after the slash
	wchar_t* _strFileName;

	std::wstring _fileNameOnly;

	std::wstring _directoryOnly;
};

