#include <windows.h>
#include <iostream>
#include <list>

LPCWSTR TargetWindowsTitle;
HWND tWnd;
RECT rect;

int marginY = 170;
int marginX = 140;
int gabY = 32;
int gabX = 41;

int level = 10;

// memory
DWORD pId;
HANDLE pHandle;
DWORD address;
DWORD value;
BYTE bValue;
SHORT sValue;
DWORD baseAddress = 0x00400000;
DWORD ecxAddress = baseAddress + 0x0030DEFC;
DWORD arrayStartPoint;
BYTE result[12][22] = { 0 };
BYTE mask[12][22] = { 0 };

// algorithm
typedef struct _vector2D {
    int x;
    int y;
} vector2D;

std::list<vector2D> originList;
std::list<vector2D> cloneList;

int failCnt;
int solveCnt;

void LeftClick()
{
	INPUT    Input = { 0 };

	// left down 
	Input.type = INPUT_MOUSE;
	Input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
	::SendInput(1, &Input, sizeof(INPUT));

	// left up
	::ZeroMemory(&Input, sizeof(INPUT));
	Input.type = INPUT_MOUSE;
	Input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
	::SendInput(1, &Input, sizeof(INPUT));
}

void MouseMove(int x, int y)
{
	double fScreenWidth = ::GetSystemMetrics(SM_CXSCREEN) - 1;
	double fScreenHeight = ::GetSystemMetrics(SM_CYSCREEN) - 1;
	double fx = x * (65535.0f / fScreenWidth);
	double fy = y * (65535.0f / fScreenHeight);
	INPUT  Input = { 0 };
	Input.type = INPUT_MOUSE;
	Input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
	Input.mi.dx = fx;
	Input.mi.dy = fy;
	::SendInput(1, &Input, sizeof(INPUT));
}

void ClickArray(int x, int y) {
	MouseMove(rect.left + marginY + (y * gabY), rect.top + marginX + (x * gabX));
	LeftClick();
}

void initMaskArray() {
	// 마스킹 배열 초기화
	for (int x = 0; x < 12; x++) {
		for (int y = 0; y < 22; y++) {
			mask[x][y] = 0;
		}
	}
}

DWORD WINAPI ThreadProc(LPVOID lParam) {
	TargetWindowsTitle = L"넷마블 사천성 Ver0.65";
	tWnd = FindWindowW(NULL, TargetWindowsTitle);
	if (!tWnd)
	{
		return 0;
	}
	GetWindowRect(tWnd, &rect);
	GetWindowThreadProcessId(tWnd, &pId);
	pHandle = OpenProcess(PROCESS_ALL_ACCESS, 0, pId);
	if (!pHandle) {
		return 0;
	}
	address = ecxAddress;
	ReadProcessMemory(pHandle, (void*)address, &value, sizeof(value), 0);

	DWORD ecxValue = value;
	DWORD gabAddress = 0x0000A168;
	DWORD realAddress = ecxValue + gabAddress;
	arrayStartPoint = realAddress - 0xA127;

	while (true)
	{
		if (GetAsyncKeyState(0x76)) // F7
		{
            // 배열 값 설정
            for (int y = 0; y < 22; y++) {
                for (int x = 0; x < 12; x++) {
                    DWORD nextPoint = arrayStartPoint + (0x34 * x) + (0x270 * y);
                    ReadProcessMemory(pHandle, (void*)nextPoint, &bValue, sizeof(bValue), 0);
                    result[x][y] = bValue;
                }
            }

            initMaskArray();

            // 이웃 가로축 마스킹
            for (int x = 0; x < 12; x++) {
                for (int y = 0; y < 22; y++) {
                    if (y + 1 != 22 && result[x][y] != 0 && result[x][y] != 67 && result[x][y] == result[x][y + 1]) {
                        mask[x][y] = 11;
                        mask[x][y + 1] = 11;
                        ClickArray(x, y);
                        Sleep(level);
                        ClickArray(x, y + 1);
                        Sleep(level);
                        // goto EXIT;
                    }
                }
            }

            // 이웃 세로축 마스킹
            for (int y = 0; y < 22; y++) {
                for (int x = 0; x < 12; x++) {
                    if (x + 1 != 12 && result[x][y] != 0 && result[x][y] != 67 && result[x][y] == result[x + 1][y]) {
                        mask[x][y] = 11;
                        mask[x + 1][y] = 11;
                        ClickArray(x, y);
                        Sleep(level);
                        ClickArray(x + 1, y);
                        Sleep(level);
                        // goto EXIT;
                    }
                }
            }

            // 사천성 알고리즘
            for (int y = 0; y < 22; y++) {
                for (int x = 0; x < 12; x++) {
                    int origin = result[x][y];
                    for (int yy = 0; yy < 22; yy++) {
                        for (int xx = 0; xx < 12; xx++) {
                            if (x == xx && y == yy) {
                                // pass
                            }
                            else {
                                int clone = result[xx][yy];
                                if (result[x][y] != 0 && result[x][y] != 67 && result[xx][yy] != 0 && result[xx][yy] != 67 && result[x][y] == result[xx][yy]) {

                                    originList.clear();
                                    cloneList.clear();

                                    // up (x y)
                                    for (int i = x - 1; i > -1; i--) {
                                        if (result[i][y] == 0) {
                                            vector2D temp = { i, y };
                                            originList.push_back(temp);
                                        }
                                        else {
                                            break;
                                        }
                                    }

                                    // up (xx yy)
                                    for (int i = xx - 1; i > -1; i--) {
                                        if (result[i][yy] == 0) {
                                            vector2D temp = { i, yy };
                                            cloneList.push_back(temp);
                                        }
                                        else {
                                            break;
                                        }
                                    }

                                    // down (x y)
                                    for (int i = x + 1; i < 12; i++) {
                                        if (result[i][y] == 0) {
                                            vector2D temp = { i, y };
                                            originList.push_back(temp);
                                        }
                                        else {
                                            break;
                                        }
                                    }

                                    // down (xx yy)
                                    for (int i = xx + 1; i < 12; i++) {
                                        if (result[i][yy] == 0) {
                                            vector2D temp = { i, yy };
                                            cloneList.push_back(temp);
                                        }
                                        else {
                                            break;
                                        }
                                    }

                                    // right (x y)
                                    for (int i = y + 1; i < 22; i++) {
                                        if (result[x][i] == 0) {
                                            vector2D temp = { x, i };
                                            originList.push_back(temp);
                                        }
                                        else {
                                            break;
                                        }
                                    }

                                    // right (xx yy)
                                    for (int i = yy + 1; i < 22; i++) {
                                        if (result[xx][i] == 0) {
                                            vector2D temp = { xx, i };
                                            cloneList.push_back(temp);
                                        }
                                        else {
                                            break;
                                        }
                                    }

                                    // left (x y)
                                    for (int i = y - 1; i > -1; i--) {
                                        if (result[x][i] == 0) {
                                            vector2D temp = { x, i };
                                            originList.push_back(temp);
                                        }
                                        else {
                                            break;
                                        }
                                    }

                                    // left (xx yy)
                                    for (int i = yy - 1; i > -1; i--) {
                                        if (result[xx][i] == 0) {
                                            vector2D temp = { xx, i };
                                            cloneList.push_back(temp);
                                        }
                                        else {
                                            break;
                                        }
                                    }

                                    std::list<vector2D>::iterator originIter;
                                    std::list<vector2D>::iterator cloneIter;

                                    failCnt = 0;
                                    solveCnt = 0;

                                    for (originIter = originList.begin(); originIter != originList.end(); ++originIter) {
                                        for (cloneIter = cloneList.begin(); cloneIter != cloneList.end(); ++cloneIter) {
                                            // 행 같을 때
                                            if (originIter->x == cloneIter->x) {
                                                // 원본이 우
                                                if (originIter->y > cloneIter->y) {
                                                    for (int i = cloneIter->y + 1; i < originIter->y; i++) {
                                                        if (result[originIter->x][i] != 0) {
                                                            failCnt += 1;
                                                            break;
                                                        }
                                                    }
                                                }
                                                // 원본이 좌
                                                if (originIter->y < cloneIter->y) {
                                                    for (int i = originIter->y + 1; i < cloneIter->y; i++) {
                                                        if (result[originIter->x][i] != 0) {
                                                            failCnt += 1;
                                                            break;
                                                        }
                                                    }
                                                }
                                                if (failCnt == 0) {
                                                    solveCnt += 1;
                                                }
                                            }

                                            // 열 같을 때
                                            if (originIter->y == cloneIter->y) {
                                                // 원본이 하
                                                if (originIter->x > cloneIter->x) {
                                                    for (int i = cloneIter->x + 1; i < originIter->x; i++) {
                                                        if (result[i][originIter->y] != 0) {
                                                            failCnt += 1;
                                                            break;
                                                        }
                                                    }
                                                }
                                                // 원본이 상
                                                if (originIter->x < cloneIter->x) {
                                                    for (int i = originIter->x + 1; i < cloneIter->x; i++) {
                                                        if (result[i][originIter->y] != 0) {
                                                            failCnt += 1;
                                                            break;
                                                        }
                                                    }
                                                }
                                                if (failCnt == 0) {
                                                    solveCnt += 1;
                                                }
                                            }
                                        }
                                    }

                                    if (solveCnt > 0) {
                                        ClickArray(x, y);
                                        Sleep(level);
                                        ClickArray(xx, yy);
                                        Sleep(level);
                                    }
                                }
                            }
                        }
                    }
                }
            }

            EXIT:
                Sleep(100);
		}
		else if (GetAsyncKeyState(0x77)) // F8
		{
			printf("press F8 \n");
		}
		Sleep(1000);
		printf("wait \n");
	}

	return 0;
}

BOOL APIENTRY DllMain( HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved )
{
	HANDLE hThread = NULL;
    switch (ul_reason_for_call)
    {
		case DLL_PROCESS_ATTACH:
			hThread = CreateThread(NULL, 0, ThreadProc, NULL, 0, NULL);
			CloseHandle(hThread);
			break;
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}

