#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <opencv\cv.h>
#include <opencv\highgui.h>
#include <Windows.h>

using namespace std;
using namespace cv;

int SearchDirectory(std::vector<std::wstring> &refvecFiles,
	const std::wstring        &refcstrRootDirectory,
	const std::wstring        &refcstrExtension,
	bool                     bSearchSubdirectories = true)
{
	std::wstring     strFilePath;             // Filepath
	std::wstring     strPattern;              // Pattern
	std::wstring     strExtension;            // Extension
	HANDLE          hFile;                   // Handle to file
	WIN32_FIND_DATA FileInformation;         // File information

	strPattern = refcstrRootDirectory + L"\\*.*";

	hFile = ::FindFirstFile((LPCWSTR)strPattern.c_str(), &FileInformation);
	if (hFile != INVALID_HANDLE_VALUE){
		do{
			if (FileInformation.cFileName[0] != '.') {
				strFilePath.erase();
				strFilePath = refcstrRootDirectory + L"\\" + FileInformation.cFileName;

				if (FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
					if (bSearchSubdirectories) {
						// Search subdirectory
						int iRC = SearchDirectory(refvecFiles,
							strFilePath,
							refcstrExtension,
							bSearchSubdirectories);
						if (iRC)
							return iRC;
					}
				}
				else {
					// Check extension
					strExtension = FileInformation.cFileName;
					strExtension = strExtension.substr(strExtension.rfind(L".") + 1);

					if (strExtension == refcstrExtension) {
						// Save filename
						refvecFiles.push_back(strFilePath);
					}
				}
			}
		} while (::FindNextFile(hFile, &FileInformation) == TRUE);

		// Close handle
		::FindClose(hFile);

		DWORD dwError = ::GetLastError();
		if (dwError != ERROR_NO_MORE_FILES)
			return dwError;
	}
	return 0;
}

int SearchFold(std::vector<std::wstring> &refvecFiles,
	const std::wstring        &refcstrRootDirectory,
	const std::wstring        &refcstrExtension,
	bool                     bSearchSubdirectories = true)
{
	std::wstring     strFilePath;             // Filepath
	std::wstring     strPattern;              // Pattern
	std::wstring     strExtension;            // Extension
	HANDLE          hFile;                   // Handle to file
	WIN32_FIND_DATA FileInformation;         // File information

	strPattern = refcstrRootDirectory + L"\\*.*";

	hFile = ::FindFirstFile((LPCWSTR)strPattern.c_str(), &FileInformation);
	if (hFile != INVALID_HANDLE_VALUE){
		do{
			if (FileInformation.cFileName[0] != '.') {
				strFilePath.erase();
				strFilePath = refcstrRootDirectory + L"\\" + FileInformation.cFileName;

				if (FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
					// Check extension
					strExtension = FileInformation.cFileName;
					strExtension = strExtension.substr(strExtension.rfind(L".") + 1);
					refvecFiles.push_back(strFilePath);

				}
			}
		} while (::FindNextFile(hFile, &FileInformation) == TRUE);

		// Close handle
		::FindClose(hFile);

		DWORD dwError = ::GetLastError();
		if (dwError != ERROR_NO_MORE_FILES)
			return dwError;
	}
	return 0;
}

void ReadAllImages(std::vector<std::wstring> imgFold, Mat *img)
{
	int counter = 0;

	for (vector<std::wstring>::iterator iterFiles = imgFold.begin(); iterFiles != imgFold.end(); ++iterFiles)
	{
		wstring ws = *iterFiles;
		string FileName(ws.begin(), ws.end());
		FileName.assign(ws.begin(), ws.end());

		img[counter] = cv::imread(FileName.c_str(), CV_LOAD_IMAGE_UNCHANGED);
		counter++;
	}
}

void saveBin(Mat *img, int counter, string binpath){
	FILE *fp;
	const char *tmp;

	cout << "---->save bin:";
	cout << binpath << endl;

	tmp = binpath.data();
	fp = fopen(tmp, "wb");
	if (fp == NULL){
		cout << "file open error!" << endl;
		exit(0);
	}

	fwrite(&counter, sizeof(uint32_t), 1, fp);
	fwrite(&img[0].cols, sizeof(uint32_t), 1, fp);
	fwrite(&img[0].rows, sizeof(uint32_t), 1, fp);
	for (int i = 0; i < counter; ++i){

		for (int m = 0; m < img[i].rows; ++m){
			UINT16 *ptr = (UINT16 *)(img[i].data + m*img[i].step);
			for (int n = 0; n < img[i].cols; ++n){
				uint32_t tmp = ptr[n];
				fwrite(&tmp, sizeof(uint32_t), 1, fp);
			}
		}
		if (i == 0){
			cout << "save frame(total number of frames: " << counter << "):" << endl;
		}
		cout << ".";
	}
	cout << endl;
	fclose(fp);
}

int main()
{
	wstring rootFold = L"E:\\datasets\\04 MSR Action Pairs Dataset\\Depth\\FullFrame";
	
	string binRoot = "C:\\Users\\doufu\\Desktop\\daily\\";

	std::vector<std::wstring> actionFold;
	SearchFold(actionFold, rootFold, L"");

	for (vector<std::wstring>::iterator action = actionFold.begin(); action != actionFold.end(); ++action){

		std::vector<std::wstring> actioni;
		SearchFold(actioni, *action, L"");

		for (vector<std::wstring>::iterator imgs = actioni.begin(); imgs != actioni.end(); ++imgs){

			std::vector<std::wstring> imgFold;
			SearchDirectory(imgFold, *imgs, L"png");
			
			int noImages = imgFold.size();
			Mat *imgsMat = new Mat[noImages]();
			ReadAllImages(imgFold, imgsMat);

			int index = imgs->find_last_of('\\');
			wstring ws = imgs->substr(index + 1, 19);
			string binName(ws.begin(), ws.end());
			binName.assign(ws.begin(), ws.end());
			binName.insert(14, "s");
			binName.erase(0, 2);
			string binPath = binRoot + binName + ".bin";
			
			saveBin(imgsMat, noImages, binPath);

			delete[] imgsMat;
			std::vector<std::wstring>(imgFold).swap(imgFold);

			system("pause");
		}
		std::vector<std::wstring>(actioni).swap(actioni);
	}
	return 0;
}