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

    // Set page segmentation mode to single line
    // Task names are typically a single line of text
    api->SetPageSegMode(tesseract::PSM_SINGLE_LINE);

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

    // Convert to grayscale for Tesseract
    QImage grayImage = image.convertToFormat(QImage::Format_Grayscale8);

    auto* api = static_cast<tesseract::TessBaseAPI*>(m_tessApi);

    // SetImage(imagedata, width, height, bytes_per_pixel, bytes_per_line)
    // Format_Grayscale8 has 1 byte per pixel
    api->SetImage(grayImage.bits(),
                  grayImage.width(),
                  grayImage.height(),
                  1,
                  grayImage.bytesPerLine());

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
