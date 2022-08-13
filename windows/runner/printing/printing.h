#ifndef PRINTING_PLUGIN_PRINTING_H_
#define PRINTING_PLUGIN_PRINTING_H_

#include <map>
#include <memory>
#include <sstream>
#include <vector>

#include <flutter/method_channel.h>

class PrintJob;

class Printing {
 private:
 public:
  Printing();

  virtual ~Printing();

  void onPageRasterized(std::vector<uint8_t> data,
                        int width,
                        int height,
                        PrintJob* job);

  void onPageRasterEnd(PrintJob* job, const std::string& error);

  void onLayout(PrintJob* job,
                double pageWidth,
                double pageHeight,
                double marginLeft,
                double marginTop,
                double marginRight,
                double marginBottom);

  void onCompleted(PrintJob* job,
                             bool completed,
                             const std::string& error);
};

#endif