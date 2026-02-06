#ifndef HANDWRITING_RECOGNIZER_H
#define HANDWRITING_RECOGNIZER_H

#include <QObject>
#include <QString>
#include <QImage>

class HandwritingRecognizer : public QObject
{
    Q_OBJECT

public:
    explicit HandwritingRecognizer(QObject* parent = nullptr);
    ~HandwritingRecognizer();

    // Initialize Tesseract engine. Call once at startup.
    // Returns true if initialization succeeded.
    bool initialize();

    // Recognize text from a QImage (the canvas export)
    Q_INVOKABLE QString recognizeImage(const QImage& image);

    // Recognize text from a file path (PNG)
    Q_INVOKABLE QString recognizeFile(const QString& filePath);

    // Whether the engine is ready
    Q_INVOKABLE bool isReady() const { return m_initialized; }

private:
    void* m_tessApi;  // Opaque pointer to tesseract::TessBaseAPI (avoid header in .h)
    bool m_initialized;
};

#endif
