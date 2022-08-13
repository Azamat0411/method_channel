#include "printing.h"

#include "print_job.h"

//namespace printingPdf {

extern std::unique_ptr<flutter::MethodChannel<flutter::EncodableValue>> channel;

Printing::Printing() {}

Printing::~Printing() {}

void Printing::onPageRasterized(std::vector<uint8_t> data,
                                int width,
                                int height,
                                PrintJob* job) {
  channel->InvokeMethod(
      "onPageRasterized",
      std::make_unique<flutter::EncodableValue>(
          flutter::EncodableValue(flutter::EncodableMap{
              {flutter::EncodableValue("image"), flutter::EncodableValue(data)},
              {flutter::EncodableValue("width"),
               flutter::EncodableValue(width)},
              {flutter::EncodableValue("height"),
               flutter::EncodableValue(height)},
              {flutter::EncodableValue("job"),
               flutter::EncodableValue(job->id())},
          })));
}

void Printing::onPageRasterEnd(PrintJob* job, const std::string& error) {
  auto map = flutter::EncodableMap{
      {flutter::EncodableValue("job"), flutter::EncodableValue(job->id())},
  };

  if (!error.empty()) {
    map[flutter::EncodableValue("error")] = flutter::EncodableValue(error);
  }

  channel->InvokeMethod(
      "onPageRasterEnd",
      std::make_unique<flutter::EncodableValue>(flutter::EncodableValue(map)));
}

class OnLayoutResult : public flutter::MethodResult<flutter::EncodableValue> {
 public:
  OnLayoutResult(PrintJob* job) : job{job} {}

 private:
  PrintJob* job;

 protected:
  void SuccessInternal(const flutter::EncodableValue* result) {
    auto doc = std::get<std::vector<uint8_t>>(*result);

    job->writeJob(doc);
    delete job;
  }

  void ErrorInternal(const std::string& error_code,
                     const std::string& error_message,
                     const flutter::EncodableValue* error_details) {
    delete job;
  }

  void NotImplementedInternal() { delete job; }
};

void Printing::onLayout(PrintJob* job,
                        double pageWidth,
                        double pageHeight,
                        double marginLeft,
                        double marginTop,
                        double marginRight,
                        double marginBottom) {
  channel->InvokeMethod("onLayout",
                        std::make_unique<flutter::EncodableValue>(
                            flutter::EncodableValue(flutter::EncodableMap{
                                {flutter::EncodableValue("job"),
                                 flutter::EncodableValue(job->id())},
                                {flutter::EncodableValue("width"),
                                 flutter::EncodableValue(pageWidth)},
                                {flutter::EncodableValue("height"),
                                 flutter::EncodableValue(pageHeight)},
                                {flutter::EncodableValue("marginLeft"),
                                 flutter::EncodableValue(marginLeft)},
                                {flutter::EncodableValue("marginTop"),
                                 flutter::EncodableValue(marginTop)},
                                {flutter::EncodableValue("marginRight"),
                                 flutter::EncodableValue(marginRight)},
                                {flutter::EncodableValue("marginBottom"),
                                 flutter::EncodableValue(marginBottom)},
                            })),
                        std::make_unique<OnLayoutResult>(job));
}

// send completion status to flutter
void Printing::onCompleted(PrintJob* job,
                           bool completed,
                           const std::string& error) {
  auto map = flutter::EncodableMap{
      {flutter::EncodableValue("job"), flutter::EncodableValue(job->id())},
      {flutter::EncodableValue("completed"),
       flutter::EncodableValue(completed)},
  };

  if (!error.empty()) {
    map[flutter::EncodableValue("error")] = flutter::EncodableValue(error);
  }

  channel->InvokeMethod(
      "onCompleted",
      std::make_unique<flutter::EncodableValue>(flutter::EncodableValue(map)));
}
//}