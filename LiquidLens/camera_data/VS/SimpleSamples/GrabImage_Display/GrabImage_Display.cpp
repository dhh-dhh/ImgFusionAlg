#include <stdio.h>
#include <process.h>
#include <conio.h>
#include "windows.h"
#include "MvCameraControl.h"

HWND g_hwnd = NULL;
bool g_bExit = false;

// ch:�ȴ��������� | en:Wait for key press
void WaitForKeyPress(void)
{
    while(!_kbhit())
    {
        Sleep(10);
    }
    _getch();
}

bool PrintDeviceInfo(MV_CC_DEVICE_INFO* pstMVDevInfo)
{
    if (NULL == pstMVDevInfo)
    {
        printf("The Pointer of pstMVDevInfo is NULL!\n");
        return false;
    }
    if (pstMVDevInfo->nTLayerType == MV_GIGE_DEVICE)
    {
        int nIp1 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24);
        int nIp2 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16);
        int nIp3 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8);
        int nIp4 = (pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);

        // ch:��ӡ��ǰ���ip���û��Զ������� | en:print current ip and user defined name
        printf("CurrentIp: %d.%d.%d.%d\n" , nIp1, nIp2, nIp3, nIp4);
        printf("UserDefinedName: %s\n\n" , pstMVDevInfo->SpecialInfo.stGigEInfo.chUserDefinedName);
    }
    else if (pstMVDevInfo->nTLayerType == MV_USB_DEVICE)
    {
        printf("UserDefinedName: %s\n", pstMVDevInfo->SpecialInfo.stUsb3VInfo.chUserDefinedName);
        printf("Serial Number: %s\n", pstMVDevInfo->SpecialInfo.stUsb3VInfo.chSerialNumber);
        printf("Device Number: %d\n\n", pstMVDevInfo->SpecialInfo.stUsb3VInfo.nDeviceNumber);
    }
    else
    {
        printf("Not support.\n");
    }

    return true;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        g_hwnd = NULL;
        break;
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

static  unsigned int __stdcall CreateRenderWindow(void* pUser)
{
    HINSTANCE hInstance = ::GetModuleHandle(NULL);              //��ȡӦ�ó����ģ����
    WNDCLASSEX wc;
    wc.cbSize           = sizeof(wc);
    wc.style            = CS_HREDRAW | CS_VREDRAW;              //���ڵķ��
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = hInstance;
    wc.hIcon            = ::LoadIcon(NULL, IDI_APPLICATION);    //ͼ����
    wc.hIconSm          = ::LoadIcon( NULL, IDI_APPLICATION );
    wc.hbrBackground    = (HBRUSH)( COLOR_WINDOW + 1);          //����ɫ
    wc.hCursor          = ::LoadCursor(NULL, IDC_ARROW);        //�����
    wc.lpfnWndProc      = WndProc;                              //�Զ�����Ϣ��������
    wc.lpszMenuName     = NULL;
    wc.lpszClassName    = "RenderWindow";                       //�ô����������

    if(!RegisterClassEx(&wc))
    {
        return 0;
    }

    DWORD style = WS_OVERLAPPEDWINDOW;
    DWORD styleEx = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
    RECT rect = {0, 0, 640, 480};

    AdjustWindowRectEx(&rect, style, false, styleEx);

    HWND hWnd = CreateWindowEx(styleEx, "RenderWindow", "Display", style, 0, 0, 
        rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, hInstance, NULL);
    if(hWnd == NULL)
    {
        return 0;
    }

    ::UpdateWindow(hWnd);
    ::ShowWindow(hWnd, SW_SHOW);

    g_hwnd = hWnd;

    MSG msg = {0};
    while(msg.message != WM_QUIT)
    {
        if(PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return 0;
}

static  unsigned int __stdcall WorkThread(void* pUser)
{
    int nRet = MV_OK;
    MV_FRAME_OUT stImageInfo = {0};
    MV_DISPLAY_FRAME_INFO stDisplayInfo = {0};

    while(1)
    {
        nRet = MV_CC_GetImageBuffer(pUser, &stImageInfo, 1000);
        if (nRet == MV_OK)
        {
            printf("Get Image Buffer: Width[%d], Height[%d], FrameNum[%d]\n", 
                stImageInfo.stFrameInfo.nWidth, stImageInfo.stFrameInfo.nHeight, stImageInfo.stFrameInfo.nFrameNum);

            if (g_hwnd)
            {
                stDisplayInfo.hWnd = g_hwnd;
                stDisplayInfo.pData = stImageInfo.pBufAddr;
                stDisplayInfo.nDataLen = stImageInfo.stFrameInfo.nFrameLen;
                stDisplayInfo.nWidth = stImageInfo.stFrameInfo.nWidth;
                stDisplayInfo.nHeight = stImageInfo.stFrameInfo.nHeight;
                stDisplayInfo.enPixelType = stImageInfo.stFrameInfo.enPixelType;

                MV_CC_DisplayOneFrame(pUser, &stDisplayInfo);
            }

            nRet = MV_CC_FreeImageBuffer(pUser, &stImageInfo);
            if(nRet != MV_OK)
            {
                printf("Free Image Buffer fail! nRet [0x%x]\n", nRet);
            }
        }
        else
        {
            printf("Get Image fail! nRet [0x%x]\n", nRet);
        }
        if(g_bExit)
        {
            break;
        }
    }

    return 0;
}

int main()
{
    int nRet = MV_OK;
    void* handle = NULL;

    do 
    {
        // ch:ö���豸 | en:Enum device
        MV_CC_DEVICE_INFO_LIST stDeviceList = {0};
        nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &stDeviceList);
        if (MV_OK != nRet)
        {
            printf("Enum Devices fail! nRet [0x%x]\n", nRet);
            break;
        }

        if (stDeviceList.nDeviceNum > 0)
        {
            for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++)
            {
                printf("[device %d]:\n", i);
                MV_CC_DEVICE_INFO* pDeviceInfo = stDeviceList.pDeviceInfo[i];
                if (NULL == pDeviceInfo)
                {
                    break;
                } 
                PrintDeviceInfo(pDeviceInfo);            
            }  
        } 
        else
        {
            printf("Find No Devices!\n");
            break;
        }

        printf("Please Input camera index(0-%d):", stDeviceList.nDeviceNum-1);
        unsigned int nIndex = 0;
        scanf_s("%d", &nIndex);

        if (nIndex >= stDeviceList.nDeviceNum)
        {
            printf("Input error!\n");
            break;
        }

        // ch:ѡ���豸��������� | en:Select device and create handle
        nRet = MV_CC_CreateHandle(&handle, stDeviceList.pDeviceInfo[nIndex]);
        if (MV_OK != nRet)
        {
            printf("Create Handle fail! nRet [0x%x]\n", nRet);
            break;
        }

        // ch:���豸 | en:Open device
        nRet = MV_CC_OpenDevice(handle);
        if (MV_OK != nRet)
        {
            printf("Open Device fail! nRet [0x%x]\n", nRet);
            break;
        }

        // ch:̽��������Ѱ���С(ֻ��GigE�����Ч) | en:Detection network optimal package size(It only works for the GigE camera)
        if (stDeviceList.pDeviceInfo[nIndex]->nTLayerType == MV_GIGE_DEVICE)
        {
            int nPacketSize = MV_CC_GetOptimalPacketSize(handle);
            if (nPacketSize > 0)
            {
                nRet = MV_CC_SetIntValue(handle,"GevSCPSPacketSize",nPacketSize);
                if(nRet != MV_OK)
                {
                    printf("Warning: Set Packet Size fail nRet [0x%x]!", nRet);
                }
            }
            else
            {
                printf("Warning: Get Packet Size fail nRet [0x%x]!", nPacketSize);
            }
        }

        // ch:���ô���ģʽΪoff | en:Set trigger mode as off
        nRet = MV_CC_SetEnumValue(handle, "TriggerMode", 0);
        if (MV_OK != nRet)
        {
            printf("Set Trigger Mode fail! nRet [0x%x]\n", nRet);
            break;
        }

        unsigned int nThreadID = 0;
        void* hCreateWindow = (void*) _beginthreadex( NULL , 0 , CreateRenderWindow , handle, 0 , &nThreadID);
        if (NULL == hCreateWindow)
        {
            break;
        }

        // ch:��ʼȡ�� | en:Start grab image
        nRet = MV_CC_StartGrabbing(handle);
        if (MV_OK != nRet)
        {
            printf("Start Grabbing fail! nRet [0x%x]\n", nRet);
            break;
        }

        nThreadID = 0;
        void* hThreadHandle = (void*) _beginthreadex( NULL , 0 , WorkThread , handle, 0 , &nThreadID);
        if (NULL == hThreadHandle)
        {
            break;
        }

        printf("Press a key to stop grabbing.\n");
        WaitForKeyPress();

        g_bExit = true;
        WaitForSingleObject(hThreadHandle, INFINITE);
        CloseHandle(hThreadHandle);

        // ch:ֹͣȡ�� | en:Stop grab image
        nRet = MV_CC_StopGrabbing(handle);
        if (MV_OK != nRet)
        {
            printf("Stop Grabbing fail! nRet [0x%x]\n", nRet);
            break;
        }

        // ch:�ر��豸 | Close device
        nRet = MV_CC_CloseDevice(handle);
        if (MV_OK != nRet)
        {
            printf("ClosDevice fail! nRet [0x%x]\n", nRet);
            break;
        }

        // ch:���پ�� | Destroy handle
        nRet = MV_CC_DestroyHandle(handle);
        if (MV_OK != nRet)
        {
            printf("Destroy Handle fail! nRet [0x%x]\n", nRet);
            break;
        }
    } while (0);
    

    if (nRet != MV_OK)
    {
        if (handle != NULL)
        {
            MV_CC_DestroyHandle(handle);
            handle = NULL;
        }
    }

    printf("Press a key to exit.\n");
    WaitForKeyPress();

    return 0;
}