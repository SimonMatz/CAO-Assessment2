#include <Windows.h>
#include <vector>
#include <string>
#include "resource.h"
#include <mutex>
#include <thread>
#include <fstream>
#include <sstream>

#define WINDOW_CLASS_NAME L"MultiThreaded Loader Tool"
const unsigned int _kuiWINDOWWIDTH = 1200;
const unsigned int _kuiWINDOWHEIGHT = 800;
#define MAX_FILES_TO_OPEN 50
#define MAX_CHARACTERS_IN_FILENAME 25

//Global Variables
std::vector<std::wstring> g_vecImageFileNames;
std::vector<std::wstring> g_vecSoundFileNames;
HINSTANCE g_hInstance;
bool g_bIsFileLoaded = false;
std::vector<std::thread> threads;
std::vector<HBITMAP> images;

int xc = 0;
int yc = 0;
HBITMAP hBitMap;
std::mutex gLock;

//returns the number of logical processors available 
int maxThreads = std::thread::hardware_concurrency();

// String stream for displaying the time
std::wstringstream stringStream;

//loads the pictures into the images vector
void loadPicture(int imageNo)
{
	gLock.lock();
	images[imageNo] = (HBITMAP)LoadImageW(NULL, (LPCWSTR)g_vecImageFileNames[imageNo].c_str(), IMAGE_BITMAP, 100, 100, LR_LOADFROMFILE);
	gLock.unlock();
}

//same as loadPicture but used for number of pictures > maxThreads
void loadPicture2(int imageNo, int imagesPerThread)
{
	gLock.lock();
	//runs as often as imagesPerThread value passed in
	for (int i = 0; i < imagesPerThread; i++)
	{
		images[imageNo+i] = (HBITMAP)LoadImageW(NULL, (LPCWSTR)g_vecImageFileNames[imageNo+i].c_str(), IMAGE_BITMAP, 100, 100, LR_LOADFROMFILE);	
	}
	gLock.unlock();
}

// used to paint the loaded pictures in correct location on screen
void controller(HWND wnd, int imageNo)
{
	gLock.lock();
	// from the second image onwards moves next picture 100 to right
	if (imageNo >= 1)
		xc += 100;
	//for first run to place picture at 0, 0
	else
		xc = 0;

	//when window width has been reached - move 100 down to start new row
	if (xc >= _kuiWINDOWWIDTH)
	{
		yc += 100;
		xc = 0;
	}

	wnd = CreateWindow(L"Static", NULL, WS_VISIBLE | WS_CHILD | SS_BITMAP, xc, yc, 0, 0, wnd, NULL, NULL, NULL);
	gLock.unlock();
	SendMessageW(wnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)images[imageNo]);
}

bool ChooseImageFilesToLoad(HWND _hwnd)
{
	OPENFILENAME ofn;
	SecureZeroMemory(&ofn, sizeof(OPENFILENAME)); // Better to use than ZeroMemory
	wchar_t wsFileNames[MAX_FILES_TO_OPEN * MAX_CHARACTERS_IN_FILENAME + MAX_PATH]; //The string to store all the filenames selected in one buffer togther with the complete path name.
	wchar_t _wsPathName[MAX_PATH + 1];
	wchar_t _wstempFile[MAX_PATH + MAX_CHARACTERS_IN_FILENAME]; //Assuming that the filename is not more than 20 characters
	wchar_t _wsFileToOpen[MAX_PATH + MAX_CHARACTERS_IN_FILENAME];
	ZeroMemory(wsFileNames, sizeof(wsFileNames));
	ZeroMemory(_wsPathName, sizeof(_wsPathName));
	ZeroMemory(_wstempFile, sizeof(_wstempFile));

	//Fill out the fields of the structure
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = _hwnd;
	ofn.lpstrFile = wsFileNames;
	ofn.nMaxFile = MAX_FILES_TO_OPEN * 20 + MAX_PATH;  //The size, in charactesr of the buffer pointed to by lpstrFile. The buffer must be atleast 256(MAX_PATH) characters long; otherwise GetOpenFileName and 
													   //GetSaveFileName functions return False
													   // Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
													   // use the contents of wsFileNames to initialize itself.
	ofn.lpstrFile[0] = '\0';
	ofn.lpstrFilter = L"Bitmap Images(.bmp)\0*.bmp\0"; //Filter for bitmap images
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT;

	//If the user makes a selection from the  open dialog box, the API call returns a non-zero value
	if (GetOpenFileName(&ofn) != 0) //user made a selection and pressed the OK button
	{
		//Extract the path name from the wide string -  two ways of doing it
		//First way: just work with wide char arrays
		wcsncpy_s(_wsPathName, wsFileNames, ofn.nFileOffset);
		int i = ofn.nFileOffset;
		int j = 0;

		while (true)
		{
			if (*(wsFileNames + i) == '\0')
			{
				_wstempFile[j] = *(wsFileNames + i);
				wcscpy_s(_wsFileToOpen, _wsPathName);
				wcscat_s(_wsFileToOpen, L"\\");
				wcscat_s(_wsFileToOpen, _wstempFile);
				g_vecImageFileNames.push_back(_wsFileToOpen);
				j = 0;
			}
			else
			{
				_wstempFile[j] = *(wsFileNames + i);
				j++;
			}
			if (*(wsFileNames + i) == '\0' && *(wsFileNames + i + 1) == '\0')
			{
				break;
			}
			else
			{
				i++;
			}

		}

		g_bIsFileLoaded = true;
		return true;
	}
	else // user pressed the cancel button or closed the dialog box or an error occured
	{
		return false;
	}

}

bool ChooseSoundFilesToLoad(HWND _hwnd)
{
	OPENFILENAME ofn;
	SecureZeroMemory(&ofn, sizeof(OPENFILENAME)); // Better to use than ZeroMemory
	wchar_t wsFileNames[MAX_FILES_TO_OPEN * MAX_CHARACTERS_IN_FILENAME + MAX_PATH]; //The string to store all the filenames selected in one buffer togther with the complete path name.
	wchar_t _wsPathName[MAX_PATH + 1];
	wchar_t _wstempFile[MAX_PATH + MAX_CHARACTERS_IN_FILENAME]; //Assuming that the filename is not more than 20 characters
	wchar_t _wsFileToOpen[MAX_PATH + MAX_CHARACTERS_IN_FILENAME];
	ZeroMemory(wsFileNames, sizeof(wsFileNames));
	ZeroMemory(_wsPathName, sizeof(_wsPathName));
	ZeroMemory(_wstempFile, sizeof(_wstempFile));

	//Fill out the fields of the structure
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = _hwnd;
	ofn.lpstrFile = wsFileNames;
	ofn.nMaxFile = MAX_FILES_TO_OPEN * 20 + MAX_PATH;  //The size, in charactesr of the buffer pointed to by lpstrFile. The buffer must be atleast 256(MAX_PATH) characters long; otherwise GetOpenFileName and 
													   //GetSaveFileName functions return False
													   // Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
													   // use the contents of wsFileNames to initialize itself.
	ofn.lpstrFile[0] = '\0';
	ofn.lpstrFilter = L"Wave Files (*.wav)\0*.wav\0All Files (*.*)\0*.*\0"; //Filter for wav files
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT;

	//If the user makes a selection from the  open dialog box, the API call returns a non-zero value
	if (GetOpenFileName(&ofn) != 0) //user made a selection and pressed the OK button
	{
		//Extract the path name from the wide string -  two ways of doing it
		//Second way: work with wide strings and a char pointer 

		std::wstring _wstrPathName = ofn.lpstrFile;

		_wstrPathName.resize(ofn.nFileOffset, '\\');

		wchar_t *_pwcharNextFile = &ofn.lpstrFile[ofn.nFileOffset];

		while (*_pwcharNextFile)
		{
			std::wstring _wstrFileName = _wstrPathName + _pwcharNextFile;

			g_vecSoundFileNames.push_back(_wstrFileName);

			_pwcharNextFile += lstrlenW(_pwcharNextFile) + 1;
		}

		g_bIsFileLoaded = true;
		return true;
	}
	else // user pressed the cancel button or closed the dialog box or an error occured
	{
		return false;
	}

}

LRESULT CALLBACK WindowProc(HWND _hwnd, UINT _uiMsg, WPARAM _wparam, LPARAM _lparam)
{
	PAINTSTRUCT ps;
	HDC _hWindowDC;
	//RECT rect;
	switch (_uiMsg)
	{
	case WM_KEYDOWN:
	{
		switch (_wparam)
		{
		case VK_ESCAPE:
		{
			SendMessage(_hwnd, WM_CLOSE, 0, 0);
			return(0);
		}
		break;
		default:
			break;
		}
	}
	break;
	case WM_PAINT:
	{
		_hWindowDC = BeginPaint(_hwnd, &ps);
		//Painting all loaded images	
		for (int i = 0; i < g_vecImageFileNames.size(); i++)
		{		
			controller(_hwnd, i);

			//Outputting the load time  - https://stackoverflow.com/questions/25829243/win32-programming-textout-wm-paint
			TextOut(_hWindowDC, 50, 300, L"Loading time: ", 17);
			TextOut(_hWindowDC, 150, 300, stringStream.str().c_str(), 6);
			TextOut(_hWindowDC, 185, 300, L"milliseconds ", 13);
		}
		EndPaint(_hwnd, &ps);
		return (0);
	}
	break;
	case WM_COMMAND:
	{
		switch (LOWORD(_wparam))
		{
		case ID_FILE_LOADIMAGE:
		{
			if (ChooseImageFilesToLoad(_hwnd))
			{
				// amount of images that will be loaded per thread
				int chunkSize = 2;
				//thread amount for default less images than maxThreads
				int amountOfThreads = g_vecImageFileNames.size();
				// thread amount for more images than maxThreads
				int amountofThreads2 = g_vecImageFileNames.size() / chunkSize;
				//leftovers for uneven numbers of images
				int imagesLeft = g_vecImageFileNames.size() % chunkSize;
				// used to iterate through images
				int j = 0;

				// reszise the vector to amount of images selected
				images.resize(g_vecImageFileNames.size());

				//start timer here
				std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

				// this runs when amount of images is greater than logical processors available
				if (g_vecImageFileNames.size() > maxThreads)
				{
					for (int i = 0; i < amountofThreads2; i++)
					{
						//for first run
						if(i == 0)
						threads.push_back(std::thread(loadPicture2, 0, chunkSize));							
												 
						else
						threads.push_back(std::thread(loadPicture2, i+j, chunkSize));

						j++;
					}

					// if number of images can't be divided by 2 - load rest of images with 1 thread each
					for (int i = 0; i < imagesLeft; i++)
					{
						threads.push_back(std::thread(loadPicture2, amountofThreads2 + j, 1));
					}
					//joining all started threads
					for (int j = 0; j < amountofThreads2 + imagesLeft; j++)
					{
						threads[j].join();
					}
					//clearing vector for next run
					threads.clear();
				}

				// this is the default when images selected are less or equal to logical proccesors available
				else
				{
					for (int i = 0; i < g_vecImageFileNames.size(); i++)
					{
						threads.push_back(std::thread(loadPicture, i));
					}
					//joining all started threads
					for (int j = 0; j < amountOfThreads; j++)
					{
						threads[j].join();
					}
					//clearing vector for next run
					threads.clear();
				}

				// ending timer here
				std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
				float time = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
				//converting time into LPCWSTR https://stackoverflow.com/questions/2481787/convert-float-to-lpcwstr-lpwstr
				stringStream << time;				
			}
			
			else
			{
				MessageBox(_hwnd, L"No Image File selected", L"Error Message", MB_ICONWARNING);
			}
			RedrawWindow(_hwnd, NULL, NULL, RDW_ERASENOW | RDW_INVALIDATE | RDW_UPDATENOW);

			return (0);
		}
		break;
		case ID_FILE_LOADSOUND:
		{
			if (ChooseSoundFilesToLoad(_hwnd))
			{
				//Write code here to create multiple threads to load sound files in parallel
			}
			else
			{
				MessageBox(_hwnd, L"No Sound File selected", L"Error Message", MB_ICONWARNING);
			}
			return (0);
		}
		break;
		case ID_EXIT:
		{
			SendMessage(_hwnd, WM_CLOSE, 0, 0);
			return (0);
		}
		break;
		default:
			break;
		}
	}
	break;
	case WM_CLOSE:
	{
		PostQuitMessage(0);
	}
	break;
	default:
		break;
	}

	return (DefWindowProc(_hwnd, _uiMsg, _wparam, _lparam));
}


HWND CreateAndRegisterWindow(HINSTANCE _hInstance)
{
	WNDCLASSEX winclass; // This will hold the class we create.
	HWND hwnd;           // Generic window handle.

						 // First fill in the window class structure.
	winclass.cbSize = sizeof(WNDCLASSEX);
	winclass.style = CS_DBLCLKS | CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	winclass.lpfnWndProc = WindowProc;
	winclass.cbClsExtra = 0;
	winclass.cbWndExtra = 0;
	winclass.hInstance = _hInstance;
	winclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	winclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	winclass.hbrBackground =
		static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
	winclass.lpszMenuName = NULL;
	winclass.lpszClassName = WINDOW_CLASS_NAME;
	winclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	// register the window class
	if (!RegisterClassEx(&winclass))
	{
		return (0);
	}

	HMENU _hMenu = LoadMenu(_hInstance, MAKEINTRESOURCE(IDR_MENU1));

	// create the window
	hwnd = CreateWindowEx(NULL, // Extended style.
		WINDOW_CLASS_NAME,      // Class.
		L"MultiThreaded Loader Tool",   // Title.
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		10, 10,                    // Initial x,y.
		_kuiWINDOWWIDTH, _kuiWINDOWHEIGHT,                // Initial width, height.
		NULL,                   // Handle to parent.
		_hMenu,                   // Handle to menu.
		_hInstance,             // Instance of this application.
		NULL);                  // Extra creation parameters.

	return hwnd;
}



int WINAPI WinMain(HINSTANCE _hInstance,
	HINSTANCE _hPrevInstance,
	LPSTR _lpCmdLine,
	int _nCmdShow)
{
	MSG msg;  //Generic Message

	HWND _hwnd = CreateAndRegisterWindow(_hInstance);

	if (!(_hwnd))
	{
		return (0);
	}


	// Enter main event loop
	while (true)
	{
		// Test if there is a message in queue, if so get it.
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			// Test if this is a quit.
			if (msg.message == WM_QUIT)
			{
				break;
			}

			// Translate any accelerator keys.
			TranslateMessage(&msg);
			// Send the message to the window proc.
			DispatchMessage(&msg);
		}

	}

	// Return to Windows like this...
	return (static_cast<int>(msg.wParam));
}