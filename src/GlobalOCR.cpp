// GlobalOCR.cpp : Definiert den Einstiegspunkt für die Anwendung.
#include "pch.h"
#include "framework.h"

#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/dnn/dnn.hpp>

using namespace cv; //todo
using namespace cv::dnn;

#include "GlobalOCR.h"
#include "Model.h"
#include "DataLoader.h"


#pragma comment(lib, "gdiplus.lib")  //Add to the linker here or via Project Properties 
#pragma comment(lib, "comctl32.lib")   //Add to the linker here or via Project Properties
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#pragma endregion

/*#pragma comment(linker,"\"/manifestdependency:type='win32' \
    name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
    processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")*/


void fourPointsTransform(const Mat& frame, const Point2f vertices[], Mat& result);
bool sortPts(const Point& p1, const Point& p2);
void PaintToHDC(Gdiplus::Graphics& graphics, std::wstring& wString, float dTX, float dTY);



DWORD WINAPI AlphaThread(LPVOID lpData)
{
    //SetThreadPriorityBoost(GetCurrentThread(), false); //dynamic boosting is disabled
    //SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);

    //Check for necessary files
    std::string DetectorPath, RecognizerPath, VocabularyPath;
    bool bFoundDetector = 0, bFoundRecognizer = 0, bFoundVocabulary = 0;
    
    const std::filesystem::path DetectorFolder{ "ModelDetection" };
    if (!std::filesystem::exists(DetectorFolder))
        std::filesystem::create_directory(DetectorFolder);

    for (auto const& dir_entry : std::filesystem::directory_iterator{ DetectorFolder })
    {
        DetectorPath = dir_entry.path().string();
        bFoundDetector = true;
    }     
    

    const std::filesystem::path RecognizerFolder{ "ModelRecognition" };
    if (!std::filesystem::exists(RecognizerFolder))
        std::filesystem::create_directory(RecognizerFolder);
    
    for (auto const& dir_entry : std::filesystem::directory_iterator{ RecognizerFolder })
    {
        if (dir_entry.path().string().find("alphabet") != std::string::npos)
        {
            VocabularyPath = dir_entry.path().string();
            bFoundVocabulary = true;
        }
        else
        {
            RecognizerPath = dir_entry.path().string();
            bFoundRecognizer = true;
        }
    }
    

    if (!bFoundDetector || !bFoundRecognizer || !bFoundVocabulary)
    {
        ShowWindow(WindowFromDC(hdc), SW_RESTORE);

        std::string ErrorType = "No ";
        ErrorType = ErrorType + 
            std::string(!bFoundDetector   ? "Detector" : "") +
            std::string(!bFoundRecognizer ? " Recognizer" : "") +
            std::string(!bFoundVocabulary ? " Vocabulary" : "") + " File found";

        MessageBoxA(0, "Read the instruction of the upcoming window to solve this problem", ErrorType.c_str(),
            MB_OK | MB_ICONERROR | MB_SETFOREGROUND);

        ShowWindow(WindowFromDC(hdc), SW_RESTORE);


        CloseHandle(GetCurrentThread());
        return 0;
    }

  
    try
    {
        //Loading Model
        TextModel textModel(DetectorPath, RecognizerPath, VocabularyPath);
        textModel.LoadTextDetectorNet();
        textModel.LoadTextRecognizerNet();

        //Input data objects
        Mat frame, recInput;
        DataLoader dataloader(hdcLayeredWnd);

        //Graphic output on layered fullSreen window
        Gdiplus::Graphics graphics(hdcLayeredWnd);
        std::wstring startMassage = L"Starting Textdetection";
        PaintToHDC(graphics, startMassage, wDesktopResolution/2, hDesktopResolution/2);
        HDC hdc2 = CreateCompatibleDC(hdcLayeredWnd);
        HBITMAP hBM;

        while (!bAThreadSwitchedOff) //Main thread loop
        {
            hBM = CreateCompatibleBitmap(hdcLayeredWnd, wDesktopResolution, hDesktopResolution);
            SelectObject(hdc2, hBM);

            //Desktop screenshot
            dataloader.read_DesktopData();

            //Inference Textdetection
            std::vector< std::vector<Point> > detResults;  
            textModel.detector.detect(dataloader.cvImage, detResults);
           
            //Clear screen
            BitBlt(hdcLayeredWnd, 0, 0, wDesktopResolution, hDesktopResolution, hdc2, 0, 0, SRCCOPY); //Alternative: graphics.Clear(Gdiplus::Color(0, 0, 0));
            recognizedWords = "";

            if (detResults.size() > 0)
            {
                //Convert to monochrome              
                if (!textModel.imreadRGB)
                    cvtColor(dataloader.cvImage, recInput, cv::COLOR_BGR2GRAY);
                else
                    recInput = dataloader.cvImage;


                for (int i = 0; i < detResults.size() && !bAThreadSwitchedOff; i++)
                {
                    //Transform and Crop from Textdetection
                    const auto& quadrangle = detResults[i];
                    CV_CheckEQ(quadrangle.size(), (size_t)4, "");

                    std::vector<Point2f> quadrangle_2f;
                    for (int j = 0; j < 4; j++)
                        quadrangle_2f.emplace_back(quadrangle[j]);
         
                    Mat cropped;
                    fourPointsTransform(recInput, &quadrangle_2f[0], cropped);

                    //Textrecognition
                    std::string recognitionResult = textModel.recognizer.recognize(cropped);
                    recognizedWords = recognizedWords + recognitionResult + " ";

                    //Draw Text on Layered Window
                    std::wstring wString(recognitionResult.begin(), recognitionResult.end());
                    PaintToHDC(graphics, wString, quadrangle[0].x, quadrangle[0].y - 4);
                }
            }
            else
            {
                //"No Text Detected"
            }

            DeleteObject(hBM);
        }

    }
    catch (cv::Exception& e)
    {
        Gdiplus::Graphics graphics(hdcLayeredWnd);
        graphics.Clear(Gdiplus::Color(0, 0, 0));

        int decisionButton = MessageBoxA(0, e.err.c_str(), String("GlobalOCR: " + e.func).c_str(), 
            MB_RETRYCANCEL | MB_ICONERROR | MB_SETFOREGROUND);

        if (decisionButton == IDCANCEL)
        {
            PostMessageA(WindowFromDC(hdcLayeredWnd), WM_QUIT, 0, 0);
            PostMessageA(WindowFromDC(hdc), WM_QUIT, 0, 0);
        }
        else
        {
            CloseHandle(GetCurrentThread());
            h_tAlpha = NULL;
            h_tAlpha = CreateThread(NULL, 0, AlphaThread, (LPVOID)1, 0, NULL);
            return 0;
        }
 
    }

 
    Gdiplus::Graphics graphics(hdcLayeredWnd);
    graphics.Clear(Gdiplus::Color(0, 0, 0));
    CloseHandle(GetCurrentThread());
    h_tAlpha = NULL;
    return 0;
}


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // ---------------------------------------------------------
    // GDI+ Initialisierung:
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR           gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    // ---------------------------------------------------------

    // Globale Zeichenfolgen initialisieren
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_GLOBALOCR, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Anwendungsinitialisierung ausführen:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GLOBALOCR));

    MSG msg;

    // Hauptnachrichtenschleife:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    Gdiplus::GdiplusShutdown(gdiplusToken);
    return (int)msg.wParam;
}


void fourPointsTransform(const Mat& frame, const Point2f vertices[], Mat& result)
{
    const Size outputSize = Size(100, 32);

    Point2f targetVertices[4] = {
        Point(0, outputSize.height - 1),
        Point(0, 0),
        Point(outputSize.width - 1, 0),
        Point(outputSize.width - 1, outputSize.height - 1)
    };

    Mat rotationMatrix = getPerspectiveTransform(vertices, targetVertices);

    warpPerspective(frame, result, rotationMatrix, outputSize);

#if 0
    imshow("roi", result);
    waitKey();
#endif
}

bool sortPts(const Point& p1, const Point& p2)
{
    return p1.x < p2.x;
}

void PaintToHDC(Gdiplus::Graphics &graphics, std::wstring &wString, float dTX, float dTY)
{
    Gdiplus::FontFamily   fontFamily(L"Arial");
    Gdiplus::Font         font(&fontFamily, 8, Gdiplus::FontStyleBold, Gdiplus::UnitPoint);
    //Gdiplus::Font         font(&fontFamily, 12, Gdiplus::FontStyleRegular, Gdiplus::UnitPoint);

    Gdiplus::PointF origin(dTX, dTY);
    Gdiplus::SolidBrush   solidBrush(Gdiplus::Color(2, 120, 2));
    graphics.SetTextRenderingHint(Gdiplus::TextRenderingHintSingleBitPerPixelGridFit);
    graphics.DrawString(wString.c_str(), wString.size(), &font, origin, &solidBrush);
}

