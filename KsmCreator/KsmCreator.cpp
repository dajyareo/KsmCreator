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
#include "../KsmCreatorLib/PathString.h"

using namespace std;
using namespace KsmCreator;

void ExitKsmCreator(UINT exitCode)
{
	if (exitCode != 0)
	{
		cout << L"Unrecoverable Error Encountered--Shutting Down!";
	}
	exit(exitCode);
}

void usage()
{
	wcout << L"  usage: ContentPipeline.KsmCreator.exe [input MODEL]" << endl;
	wcout << L"  usage: ContentPipeline.KsmCreator.exe [path to directory of MODELS] [KSM output path]" << endl;
	wcout << endl;
	wcout << L"  Glossary:" << endl;
	wcout << L"  input MODEL:    specifies the input mesh file (DAE/3DS, etc) file" << endl;
	wcout << L"  output KSM:     specifies the output KSM file" << endl;
}

bool ImportBatch(
	_In_ wstring dir,
	_In_ wstring searchPattern,
	_Out_ vector<shared_ptr<KsmModel>>& data)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	wstring fileFileArgs = dir;
	fileFileArgs.append(L"\\");
	fileFileArgs.append(searchPattern);

	hFind = FindFirstFileEx(fileFileArgs.c_str(), FindExInfoStandard, &FindFileData, FindExSearchNameMatch, nullptr, 0);
	if (hFind == INVALID_HANDLE_VALUE) 
	{
		wcout << L"FindFirstFile failed with error:" << GetLastError() << "\nSearch Pattern: " << searchPattern;
		return false;
	} 
	else
	{	
		BOOL filesStillExist = true;

		shared_ptr<KsmModel> firstModel = make_shared<KsmModel>();
		PathString firstFile(dir.c_str(), FindFileData.cFileName);
		wcout << L"Loading Model: " << firstFile.ToWCStr() << endl;
		firstModel->LoadFromDae(firstFile.ToWCStr());
		data.push_back(firstModel);

		while (filesStillExist)
		{
			filesStillExist = FindNextFileW(hFind, &FindFileData);
			{
				shared_ptr<KsmModel> model = make_shared<KsmModel>();
				PathString file(dir.c_str(), FindFileData.cFileName);
				wcout << L"Loading Model: " << file.ToWCStr() << endl;
				model->LoadFromDae(file.ToWCStr());
				data.push_back(model);
			}
		} 	
	}
	FindClose(hFind);
	return true;
}

int _tmain(int argc, _TCHAR* argv[])
{
	// There are two possible run paths. Either this is a batch or a single call. If there are 2 user args it's a single, 1 it's a batch
	if (argc > 3 || argc < 2)
	{
		usage();
		return 1;
	}
	if (argc == 2)
	{
		wchar_t currentDir[MAX_PATH];
		GetCurrentDirectoryW(MAX_PATH, currentDir);

		wstring savePath(currentDir);

		PathString inputModel(argv[1]);

		KsmModel ksmModel;
		if (!ksmModel.LoadFromDae(inputModel.ToWCStr()))
		{
			wcout << inputModel.ToWCStr() << L" failed to load correctly!";
			ExitKsmCreator(1);
		}

		wcout << L"Saving KSM file...";
		ksmModel.Save(savePath);
	}
	if (argc == 3)
	{
		// Get the output path
		wstring outputPath(argv[2]);
		if (!PathIsDirectory(outputPath.c_str()))
		{
			wcout << L"Invalid Output Directory: " << outputPath.c_str();
			return 1;
		}

		wstring inputDirectory(argv[1]);
		if (!PathIsDirectory(inputDirectory.c_str()))
		{
			wcout << L"Invalid Input Directory: " << inputDirectory.c_str();
			return 1;
		}

		vector< shared_ptr<KsmModel>> ksmModels;

		ImportBatch(inputDirectory, L"*.DAE", ksmModels);

		wcout << L"Saving KSM files...";
		for (auto& ksmModel : ksmModels)
		{
			if (!ksmModel->Save(outputPath.c_str()))
			{
				wcout << L"Failed to save " << ksmModel->FileName << endl;
				ExitKsmCreator(1);
			}
		}
	}	

	return 0;
}

