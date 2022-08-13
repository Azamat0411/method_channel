#include "printing_plugin.h"

#include <windows.h>

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>

#include <map>
#include <memory>
#include <sstream>

#include "print_job.h"
#include "printing.h"

//namespace printingPdf {

std::unique_ptr<flutter::MethodChannel<flutter::EncodableValue>> channel;

class PrintingPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(
      flutter::PluginRegistrarWindows* registrar) {
      channel = std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
          registrar->messenger(), "printing",
          &flutter::StandardMethodCodec::GetInstance());

    auto plugin = std::make_unique<PrintingPlugin>();

    channel->SetMethodCallHandler(
        [plugin_pointer = plugin.get()](const auto& call, auto result) {
          plugin_pointer->HandleMethodCall(call, std::move(result));
        });

    registrar->AddPlugin(std::move(plugin));
  }

  PrintingPlugin() {}

  virtual ~PrintingPlugin() {}

 private:
  Printing printing{};

  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue>& method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    if (method_call.method_name().compare("printPdf") == 0) {
      const auto* arguments =
          std::get_if<flutter::EncodableMap>(method_call.arguments());
      auto vName = arguments->find(flutter::EncodableValue("name"));
      auto name = vName != arguments->end() && !vName->second.IsNull()
                      ? std::get<std::string>(vName->second)
                      : std::string{"document"};
      auto vPrinter = arguments->find(flutter::EncodableValue("printer"));
      auto printer = vPrinter != arguments->end()
                         ? std::get<std::string>(vPrinter->second)
                         : std::string{};
      auto width = std::get<double>(
          arguments->find(flutter::EncodableValue("width"))->second);
      auto height = std::get<double>(
          arguments->find(flutter::EncodableValue("height"))->second);
      auto usePrinterSettings = std::get<bool>(
          arguments->find(flutter::EncodableValue("usePrinterSettings"))
              ->second);
      auto vJob = arguments->find(flutter::EncodableValue("job"));
      auto jobNum = vJob != arguments->end() ? std::get<int>(vJob->second) : -1;
      auto job = new PrintJob{&printing, jobNum};
      auto res =
          job->printPdf(name, printer, width, height, usePrinterSettings);
      if (!res) {
        delete job;
      }
      result->Success(flutter::EncodableValue(res ? 1 : 0));
    } else if (method_call.method_name().compare("sharePdf") == 0) {
      const auto* arguments =
          std::get_if<flutter::EncodableMap>(method_call.arguments());
      auto vName = arguments->find(flutter::EncodableValue("name"));
      auto name = vName != arguments->end() && !vName->second.IsNull()
                      ? std::get<std::string>(vName->second)
                      : std::string{"document.pdf"};
      auto vDoc = arguments->find(flutter::EncodableValue("doc"));
      auto doc = vDoc != arguments->end()
                     ? std::get<std::vector<uint8_t>>(vDoc->second)
                     : std::vector<uint8_t>{};
      auto job = std::make_unique<PrintJob>(&printing, -1);
      auto res = job->sharePdf(doc, name);
      result->Success(flutter::EncodableValue(res ? 1 : 0));
    } else if (method_call.method_name().compare("listPrinters") == 0) {
      auto job = std::make_unique<PrintJob>(&printing, -1);
      auto printers = job->listPrinters();
      auto pl = flutter::EncodableList{};
      for (auto printer : printers) {
        auto mp = flutter::EncodableMap{};
        mp[flutter::EncodableValue("name")] =
            flutter::EncodableValue(printer.name);
        mp[flutter::EncodableValue("url")] =
            flutter::EncodableValue(printer.url);
        mp[flutter::EncodableValue("model")] =
            flutter::EncodableValue(printer.model);
        mp[flutter::EncodableValue("location")] =
            flutter::EncodableValue(printer.location);
        mp[flutter::EncodableValue("comment")] =
            flutter::EncodableValue(printer.comment);
        mp[flutter::EncodableValue("default")] =
            flutter::EncodableValue(printer.default);
        mp[flutter::EncodableValue("available")] =
            flutter::EncodableValue(printer.available);
        pl.push_back(mp);
      }
      result->Success(pl);
    } else if (method_call.method_name().compare("rasterPdf") == 0) {
      const auto* arguments =
          std::get_if<flutter::EncodableMap>(method_call.arguments());
      auto vDoc = arguments->find(flutter::EncodableValue("doc"));
      auto doc = vDoc != arguments->end()
                     ? std::get<std::vector<uint8_t>>(vDoc->second)
                     : std::vector<uint8_t>{};
      auto vPages = arguments->find(flutter::EncodableValue("pages"));
      auto lPages = vPages != arguments->end() && !vPages->second.IsNull()
                        ? std::get<flutter::EncodableList>(vPages->second)
                        : flutter::EncodableList{};
      auto pages = std::vector<int>{};
      for (auto page : lPages) {
        pages.push_back(std::get<int>(page));
      }
      auto vScale = arguments->find(flutter::EncodableValue("scale"));
      auto scale =
          vScale != arguments->end() ? std::get<double>(vScale->second) : 1;
      auto vJob = arguments->find(flutter::EncodableValue("job"));
      auto jobNum = vJob != arguments->end() ? std::get<int>(vJob->second) : -1;
      auto job = std::make_unique<PrintJob>(&printing, jobNum);
      job->rasterPdf(doc, pages, scale);
      result->Success(nullptr);
    } else if (method_call.method_name().compare("printingInfo") == 0) {
      auto job = std::make_unique<PrintJob>(&printing, -1);
      auto map = flutter::EncodableMap{};
      for (auto item : job->printingInfo()) {
        map[flutter::EncodableValue(item.first)] =
            flutter::EncodableValue(item.second);
      }
      result->Success(map);
    } else {
      result->NotImplemented();
    }
  }
};
//}


void PrintingPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {

  printing::PrintingPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}