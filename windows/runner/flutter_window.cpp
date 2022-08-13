#include "flutter_window.h"

#include <optional>
#include <objbase.h>
#include "flutter/generated_plugin_registrant.h"
#include <flutter/plugin_registrar_windows.h>

#include <windows.h>

#include <memory>
#include <map>
#include <sstream>
#include <flutter/standard_method_codec.h>
#include <stdlib.h>
#include <flutter/method_channel.h>

const auto pdfDpi = 72;
HDC hDC = nullptr;
HGLOBAL hDevMode = nullptr;
HGLOBAL hDevNames = nullptr;

std::wstring fromUtf8(std::string str) {
    auto len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(),
        static_cast<int>(str.length()), nullptr, 0);
    if (len <= 0) {
        return L"";
    }

    auto wstr = std::wstring{};
    wstr.resize(len);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.length()),
        &wstr[0], len);

    return wstr;
}

bool printPdf(const std::string& name,
    std::string printer,
    double width,
    double height,
    bool usePrinterSettings) {
    //documentName = name;

    auto dm = static_cast<DEVMODE*>(GlobalAlloc(0, sizeof(DEVMODE)));

    if (usePrinterSettings) {
        dm = nullptr;  // to use default driver config
    }
    else {
        ZeroMemory(dm, sizeof(DEVMODE));
        dm->dmSize = sizeof(DEVMODE);
        dm->dmFields =
            DM_ORIENTATION | DM_PAPERSIZE | DM_PAPERLENGTH | DM_PAPERWIDTH;
        dm->dmPaperSize = 0;
        if (width > height) {
            dm->dmOrientation = DMORIENT_LANDSCAPE;
            dm->dmPaperWidth = static_cast<short>(round(height * 254 / 72));
            dm->dmPaperLength = static_cast<short>(round(width * 254 / 72));
        }
        else {
            dm->dmOrientation = DMORIENT_PORTRAIT;
            dm->dmPaperWidth = static_cast<short>(round(width * 254 / 72));
            dm->dmPaperLength = static_cast<short>(round(height * 254 / 72));
        }
    }

    if (printer.empty()) {
        PRINTDLG pd;

        // Initialize PRINTDLG
        ZeroMemory(&pd, sizeof(pd));
        pd.lStructSize = sizeof(pd);

        // Initialize PRINTDLG
        pd.hwndOwner = nullptr;
        pd.hDevMode = dm;
        pd.hDevNames = nullptr;  // Don't forget to free or store hDevNames.
        pd.hDC = nullptr;
        pd.Flags = PD_USEDEVMODECOPIES | PD_RETURNDC | PD_PRINTSETUP |
            PD_NOSELECTION | PD_NOPAGENUMS;
        pd.nCopies = 1;
        pd.nFromPage = 0xFFFF;
        pd.nToPage = 0xFFFF;
        pd.nMinPage = 1;
        pd.nMaxPage = 0xFFFF;

        auto r = PrintDlg(&pd);

        std::cout<<r;

        if (r != 1) {
            //printing.onCompleted(this, false, "");
            DeleteDC(hDC);
            GlobalFree(hDevNames);
            ClosePrinter(hDevMode);
            return true;
        }

        hDC = pd.hDC;
        hDevMode = pd.hDevMode;
        hDevNames = pd.hDevNames;

    }
    else {
        hDC = CreateDC(TEXT("WINSPOOL"), fromUtf8(printer).c_str(), nullptr, dm);
        if (!hDC) {
            return false;
        }
        hDevMode = dm;
        hDevNames = nullptr;
    }

    auto dpiX = static_cast<double>(GetDeviceCaps(hDC, LOGPIXELSX)) / pdfDpi;
    auto dpiY = static_cast<double>(GetDeviceCaps(hDC, LOGPIXELSY)) / pdfDpi;
    auto pageWidth =
        static_cast<double>(GetDeviceCaps(hDC, PHYSICALWIDTH)) / dpiX;
    auto pageHeight =
        static_cast<double>(GetDeviceCaps(hDC, PHYSICALHEIGHT)) / dpiY;
    auto printableWidth = static_cast<double>(GetDeviceCaps(hDC, HORZRES)) / dpiX;
    auto printableHeight =
        static_cast<double>(GetDeviceCaps(hDC, VERTRES)) / dpiY;
    auto marginLeft =
        static_cast<double>(GetDeviceCaps(hDC, PHYSICALOFFSETX)) / dpiX;
    auto marginTop =
        static_cast<double>(GetDeviceCaps(hDC, PHYSICALOFFSETY)) / dpiY;
    auto marginRight = pageWidth - printableWidth - marginLeft;
    auto marginBottom = pageHeight - printableHeight - marginTop;
    std::cout << marginRight;
    std::cout << marginBottom;
    // printing.onLayout(this, pageWidth, pageHeight, marginLeft, marginTop,
     //    marginRight, marginBottom);
    return true;
}

void handle(const flutter::MethodCall<>& call,
                   std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result){
    if (call.method_name().compare("printPdf") == 0) {
        
        const auto arguments =
            std::get_if<flutter::EncodableMap>(call.arguments());
       
        auto vName = arguments->find(flutter::EncodableValue("name"));
        std::string name = vName != arguments->end() && !vName->second.IsNull()
            ? std::get<std::string>(vName->second)
            : std::string{ "document" };
       
        auto vPrinter = arguments->find(flutter::EncodableValue("printer"));
        auto printer = vPrinter != arguments->end()
            ? std::get<std::string>(vPrinter->second)
            : std::string{};
        auto width = std::get<double>(
            arguments->find(flutter::EncodableValue("width"))->second);
        std::cout << width;
        auto height = std::get<double>(
            arguments->find(flutter::EncodableValue("height"))->second);
        std::cout << height;
        auto usePrinterSettings = std::get<bool>(
            arguments->find(flutter::EncodableValue("usePrinterSettings"))
            ->second);
        std::cout << usePrinterSettings;
        auto vJob = arguments->find(flutter::EncodableValue("job"));
       
        auto jobNum = vJob != arguments->end() ? std::get<int>(vJob->second) : -1;
        std::cout << jobNum;
       // auto job = new PrintJob{jobNum};

        auto res = printPdf(name, printer, width, height, usePrinterSettings);
        if (!res) {
            //delete job;
        }
        result->Success(flutter::EncodableValue(res ? 1 : 0));
       
    }
    else {
        result->NotImplemented();
    }

}

FlutterWindow::FlutterWindow(const flutter::DartProject& project)
    : project_(project) {}

FlutterWindow::~FlutterWindow() {}

bool FlutterWindow::OnCreate() {
  if (!Win32Window::OnCreate()) {
    return false;
  }

  RECT frame = GetClientArea();

  flutter_controller_ = std::make_unique<flutter::FlutterViewController>(
      frame.right - frame.left, frame.bottom - frame.top, project_);
  // Ensure that basic setup of the controller was successful.
  if (!flutter_controller_->engine() || !flutter_controller_->view()) {
    return false;
  }
  RegisterPlugins(flutter_controller_->engine());
 
  flutter::MethodChannel<> channel(
        flutter_controller_->engine()->messenger(), "printing",
        &flutter::StandardMethodCodec::GetInstance());

  channel.SetMethodCallHandler(
      [](const flutter::MethodCall<>& call,
          std::unique_ptr<flutter::MethodResult<>> result) {
              handle(call, std::move(result));
      });

  SetChildContent(flutter_controller_->view()->GetNativeWindow());
  return true;
}

void FlutterWindow::OnDestroy() {
  if (flutter_controller_) {
    flutter_controller_ = nullptr;
  }

  Win32Window::OnDestroy();
}

LRESULT
FlutterWindow::MessageHandler(HWND hwnd, UINT const message,
                              WPARAM const wparam,
                              LPARAM const lparam) noexcept {

  if (flutter_controller_) {
    std::optional<LRESULT> result =
        flutter_controller_->HandleTopLevelWindowProc(hwnd, message, wparam,
                                                      lparam);
    if (result) {
      return *result;
    }
  }

  switch (message) {
    case WM_FONTCHANGE:
      flutter_controller_->engine()->ReloadSystemFonts();
      break;
  }

  return Win32Window::MessageHandler(hwnd, message, wparam, lparam);
}