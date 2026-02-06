#include "handwriting_recognizer.h"
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <QDebug>

HandwritingRecognizer::HandwritingRecognizer(QObject* parent)
    : QObject(parent)
    , m_tessApi(nullptr)
    , m_initialized(false)
{
}

HandwritingRecognizer::~HandwritingRecognizer()
{
    if (m_tessApi) {
        auto* api = static_cast<tesseract::TessBaseAPI*>(m_tessApi);
        api->End();
        delete api;
        m_tessApi = nullptr;
    }
}

bool HandwritingRecognizer::initialize()
{
    if (m_initialized) {
        return true;
    }

    auto* api = new tesseract::TessBaseAPI();

    // NULL datapath tells Tesseract to search standard paths:
    // - /usr/share/tesseract-ocr/4.00/tessdata (or 5.00)
    // - TESSDATA_PREFIX environment variable
    // "eng" specifies English language data
    if (api->Init(nullptr, "eng") != 0) {
        qWarning() << "Failed to initialize Tesseract OCR engine";
        qWarning() << "Make sure tesseract-ocr-eng language data is installed";
        qWarning() << "Typical location: /usr/share/tesseract-ocr/4.00/tessdata/eng.traineddata";
        delete api;
        return false;
    }

    // Set page segmentation mode to single block of text
    // PSM_AUTO (3) lets Tesseract figure out the layout
    // This works better for handwritten text than PSM_SINGLE_LINE
    api->SetPageSegMode(tesseract::PSM_AUTO);

    // Configure Tesseract variables for better handwriting recognition
    api->SetVariable("tessedit_char_whitelist",
                     "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 .,!?'-");

    // Improve recognition quality
    api->SetVariable("classify_bln_numeric_mode", "0");

    m_tessApi = api;
    m_initialized = true;

    qDebug() << "Tesseract OCR initialized successfully";
    return true;
}

QString HandwritingRecognizer::recognizeImage(const QImage& image)
{
    if (!m_initialized) {
        qWarning() << "HandwritingRecognizer not initialized";
        return QString();
    }

    if (image.isNull() || image.width() == 0 || image.height() == 0) {
        qWarning() << "Invalid image for recognition";
        return QString();
    }

    qDebug() << "Processing image:" << image.width() << "x" << image.height()
             << "format:" << image.format();

    // Convert to grayscale
    QImage grayImage = image.convertToFormat(QImage::Format_Grayscale8);

    // Calculate Otsu's threshold for automatic thresholding
    // This is better than a fixed threshold
    int histogram[256] = {0};
    for (int y = 0; y < grayImage.height(); ++y) {
        const uchar* line = grayImage.constScanLine(y);
        for (int x = 0; x < grayImage.width(); ++x) {
            histogram[line[x]]++;
        }
    }

    int total = grayImage.width() * grayImage.height();
    float sum = 0;
    for (int i = 0; i < 256; i++) {
        sum += i * histogram[i];
    }

    float sumB = 0;
    int wB = 0;
    int wF = 0;
    float varMax = 0;
    int threshold = 0;

    for (int i = 0; i < 256; i++) {
        wB += histogram[i];
        if (wB == 0) continue;
        wF = total - wB;
        if (wF == 0) break;

        sumB += i * histogram[i];
        float mB = sumB / wB;
        float mF = (sum - sumB) / wF;
        float varBetween = (float)wB * (float)wF * (mB - mF) * (mB - mF);

        if (varBetween > varMax) {
            varMax = varBetween;
            threshold = i;
        }
    }

    qDebug() << "Calculated Otsu threshold:" << threshold;

    // Apply the calculated threshold
    QImage binaryImage = grayImage.copy();
    for (int y = 0; y < binaryImage.height(); ++y) {
        uchar* line = binaryImage.scanLine(y);
        for (int x = 0; x < binaryImage.width(); ++x) {
            line[x] = (line[x] < threshold) ? 0 : 255;
        }
    }

    // Invert if needed (Tesseract expects black text on white background)
    // Count black vs white pixels to determine if we need to invert
    int blackPixels = 0;
    for (int y = 0; y < binaryImage.height(); ++y) {
        const uchar* line = binaryImage.constScanLine(y);
        for (int x = 0; x < binaryImage.width(); ++x) {
            if (line[x] == 0) blackPixels++;
        }
    }

    // If more than 50% is black, invert (background should be white)
    if (blackPixels > total / 2) {
        qDebug() << "Inverting image (black background detected)";
        binaryImage.invertPixels();
    }

    auto* api = static_cast<tesseract::TessBaseAPI*>(m_tessApi);

    // SetImage(imagedata, width, height, bytes_per_pixel, bytes_per_line)
    api->SetImage(binaryImage.bits(),
                  binaryImage.width(),
                  binaryImage.height(),
                  1,
                  binaryImage.bytesPerLine());

    // Get recognized text
    char* outText = api->GetUTF8Text();
    if (!outText) {
        qWarning() << "Tesseract returned null text";
        return QString();
    }

    QString result = QString::fromUtf8(outText).trimmed();
    delete[] outText;

    qDebug() << "Recognized text:" << result;
    return result;
}

QString HandwritingRecognizer::recognizeFile(const QString& filePath)
{
    QImage image(filePath);
    if (image.isNull()) {
        qWarning() << "Failed to load image from" << filePath;
        return QString();
    }

    return recognizeImage(image);
}
