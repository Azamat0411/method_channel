#ifndef PRINTING_PLUGIN_PRINT_JOB_H_
#define PRINTING_PLUGIN_PRINT_JOB_H_

//#include <flutter/standard_method_codec.h>
//#include <windows.h>

//#include <map>
//#include <memory>
//#include <sstream>
//#include <vector>

//namespace printingPdf {

    struct Printer {
        const std::string name;
        const std::string url;
        const std::string model;
        const std::string location;
        const std::string comment;
        const bool default;
        const bool available;

        Printer(const std::string& name,
            const std::string& url,
            const std::string& model,
            const std::string& location,
            const std::string& comment,
            bool default,
            bool available)
            : name(name),
            url(url),
            model(model),
            location(location),
            comment(comment),
            default(default),
            available(available) {}
    };

    class PrintJob {
    private:
        int index;
        HGLOBAL hDevMode = nullptr;
        HGLOBAL hDevNames = nullptr;
        HDC hDC = nullptr;
        std::string documentName;

    public:
        PrintJob(int index);

        int id() { return index; }

        std::vector<Printer> listPrinters();

        bool printPdf(const std::string& name,
            std::string printer,
            double width,
            double height,
            bool usePrinterSettings);

        void writeJob(std::vector<uint8_t> data);

        void cancelJob(const std::string& error);

        bool sharePdf(std::vector<uint8_t> data, const std::string& name);

        void pickPrinter(void* result);

        void rasterPdf(std::vector<uint8_t> data,
            std::vector<int> pages,
            double scale);

        std::map<std::string, bool> printingInfo();
    };
//}

#endif