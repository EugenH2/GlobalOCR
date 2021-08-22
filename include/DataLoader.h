#pragma once

class DataLoader
{
public:
    DataLoader(HDC& hdc) :
        hdc(hdc),
        hScreen(GetDC(NULL)), //Desktop hdc

        wRes(GetSystemMetrics(SM_CXVIRTUALSCREEN)), //DesktopResolution
        hRes(GetSystemMetrics(SM_CYVIRTUALSCREEN))
    {
        //Desktop screenshot parameter
        hDCBitmap = CreateCompatibleDC(hScreen);
        hBitmap = CreateCompatibleBitmap(hScreen, wRes, hRes);
        old_obj = SelectObject(hDCBitmap, hBitmap);

        info.bmiHeader.biSize = sizeof(info.bmiHeader);
        info.bmiHeader.biWidth = wRes;
        info.bmiHeader.biHeight = -hRes;
        info.bmiHeader.biPlanes = 1;
        info.bmiHeader.biBitCount = 24;
        info.bmiHeader.biCompression = BI_RGB;

        cvImage.create(hRes, wRes, CV_8UC3);
    }   

    ~DataLoader()
    {
        SelectObject(hDCBitmap, old_obj);
        DeleteDC(hDCBitmap);
        ReleaseDC(NULL, hScreen);
        DeleteObject(hBitmap);
    }

    void read_DesktopData();

    cv::Mat cvImage;

private:
    int wRes, hRes;
    HDC hdc, hScreen, hDCBitmap;

    HBITMAP hBitmap; 
    HGDIOBJ old_obj;;

    BITMAPINFO info{ }; 
};


void DataLoader::read_DesktopData()
{
    //BitBlt from hScreen to hDCBitmap every round
    BitBlt(hDCBitmap, 0, 0, wRes, hRes, hScreen, 0, 0, SRCCOPY);
    //Desktop screenshot to cv::Mat
    GetDIBits(hdc, hBitmap, 0, hRes, cvImage.data, &info, DIB_RGB_COLORS);
};
