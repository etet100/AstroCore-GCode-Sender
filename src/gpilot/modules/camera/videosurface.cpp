#include "videosurface.h"
#include <QImage>
#include <QPixmap>
#include <QDebug>
#include "qvideoframeconversionhelper.h"

// VideoSurface::VideoSurface()
// {

// }

// VideoSurface::~VideoSurface()
// {

// }

static VideoFrameConvertFunc qConvertFuncs[QVideoFrame::NPixelFormats] = {
    /* Format_Invalid */                nullptr, // Not needed
    /* Format_ARGB32 */                 nullptr, // Not needed
    /* Format_ARGB32_Premultiplied */   nullptr, // Not needed
    /* Format_RGB32 */                  nullptr, // Not needed
    /* Format_RGB24 */                  nullptr, // Not needed
    /* Format_RGB565 */                 nullptr, // Not needed
    /* Format_RGB555 */                 nullptr, // Not needed
    /* Format_ARGB8565_Premultiplied */ nullptr, // Not needed
    /* Format_BGRA32 */                 qt_convert_BGRA32_to_ARGB32,
    /* Format_BGRA32_Premultiplied */   qt_convert_BGRA32_to_ARGB32,
    /* Format_BGR32 */                  qt_convert_BGRA32_to_ARGB32,
    /* Format_BGR24 */                  qt_convert_BGR24_to_ARGB32,
    /* Format_BGR565 */                 qt_convert_BGR565_to_ARGB32,
    /* Format_BGR555 */                 qt_convert_BGR555_to_ARGB32,
    /* Format_BGRA5658_Premultiplied */ nullptr,
    /* Format_AYUV444 */                qt_convert_AYUV444_to_ARGB32,
    /* Format_AYUV444_Premultiplied */  nullptr,
    /* Format_YUV444 */                 qt_convert_YUV444_to_ARGB32,
    /* Format_YUV420P */                qt_convert_YUV420P_to_ARGB32,
    /* Format_YV12 */                   qt_convert_YV12_to_ARGB32,
    /* Format_UYVY */                   qt_convert_UYVY_to_ARGB32,
    /* Format_YUYV */                   qt_convert_YUYV_to_ARGB32,
    /* Format_NV12 */                   qt_convert_NV12_to_ARGB32,
    /* Format_NV21 */                   qt_convert_NV21_to_ARGB32,
    /* Format_IMC1 */                   nullptr,
    /* Format_IMC2 */                   nullptr,
    /* Format_IMC3 */                   nullptr,
    /* Format_IMC4 */                   nullptr,
    /* Format_Y8 */                     nullptr,
    /* Format_Y16 */                    nullptr,
    /* Format_Jpeg */                   nullptr, // Not needed
    /* Format_CameraRaw */              nullptr,
    /* Format_AdobeDng */               nullptr,
};

// static void qInitConvertFuncsAsm()
// {
// #ifdef QT_COMPILER_SUPPORTS_SSE2
//     extern void QT_FASTCALL qt_convert_BGRA32_to_ARGB32_sse2(const QVideoFrame&, uchar*);
//     //if (qCpuHasFeature(SSE2))
//     {
//         qConvertFuncs[QVideoFrame::Format_BGRA32] = qt_convert_BGRA32_to_ARGB32; //_sse2;
//         qConvertFuncs[QVideoFrame::Format_BGRA32_Premultiplied] = qt_convert_BGRA32_to_ARGB32; //_sse2;
//         qConvertFuncs[QVideoFrame::Format_BGR32] = qt_convert_BGRA32_to_ARGB32; //_sse2;
//     }
// #endif
// #ifdef QT_COMPILER_SUPPORTS_SSSE3
//     extern void QT_FASTCALL qt_convert_BGRA32_to_ARGB32_ssse3(const QVideoFrame&, uchar*);
//     //if (qCpuHasFeature(SSSE3))
//     {
//         qConvertFuncs[QVideoFrame::Format_BGRA32] = qt_convert_BGRA32_to_ARGB32; //_ssse3;
//         qConvertFuncs[QVideoFrame::Format_BGRA32_Premultiplied] = qt_convert_BGRA32_to_ARGB32; //_ssse3;
//         qConvertFuncs[QVideoFrame::Format_BGR32] = qt_convert_BGRA32_to_ARGB32; //_ssse3;
//     }
// #endif
// #ifdef QT_COMPILER_SUPPORTS_AVX2
//     extern void QT_FASTCALL qt_convert_BGRA32_to_ARGB32_avx2(const QVideoFrame&, uchar*);
//     if (qCpuHasFeature(AVX2)){
//         qConvertFuncs[QVideoFrame::Format_BGRA32] = qt_convert_BGRA32_to_ARGB32_avx2;
//         qConvertFuncs[QVideoFrame::Format_BGRA32_Premultiplied] = qt_convert_BGRA32_to_ARGB32_avx2;
//         qConvertFuncs[QVideoFrame::Format_BGR32] = qt_convert_BGRA32_to_ARGB32_avx2;
//     }
// #endif
// }

// QList<QVideoFrame::PixelFormat> VideoSurface::supportedPixelFormats(
//         QAbstractVideoBuffer::HandleType type) const
// {
//     QList<QVideoFrame::PixelFormat> list_pixel_formats;
//     list_pixel_formats << QVideoFrame::Format_ARGB32_Premultiplied
//                        << QVideoFrame::Format_RGB32
//                        << QVideoFrame::Format_RGB24
//                        << QVideoFrame::Format_RGB565
//                        << QVideoFrame::Format_RGB555
//                        << QVideoFrame::Format_ARGB8565_Premultiplied
//                        << QVideoFrame::Format_BGRA32
//                        << QVideoFrame::Format_BGRA32_Premultiplied
//                        << QVideoFrame::Format_BGR32
//                        << QVideoFrame::Format_BGR24
//                        << QVideoFrame::Format_BGR565
//                        << QVideoFrame::Format_BGR555
//                        << QVideoFrame::Format_BGRA5658_Premultiplied
//                        << QVideoFrame::Format_AYUV444
//                        << QVideoFrame::Format_AYUV444_Premultiplied
//                        << QVideoFrame::Format_YUV444
//                        << QVideoFrame::Format_YUV420P
//                        << QVideoFrame::Format_YV12
//                        << QVideoFrame::Format_UYVY
//                        << QVideoFrame::Format_YUYV
//                        << QVideoFrame::Format_NV12
//                        << QVideoFrame::Format_NV21
//                        << QVideoFrame::Format_IMC1
//                        << QVideoFrame::Format_IMC2
//                        << QVideoFrame::Format_IMC3
//                        << QVideoFrame::Format_IMC4
//                        << QVideoFrame::Format_Y8
//                        << QVideoFrame::Format_Y16
//                        << QVideoFrame::Format_Jpeg
//                        << QVideoFrame::Format_CameraRaw
//                        << QVideoFrame::Format_AdobeDng;
//     return list_pixel_formats;
// };

// bool VideoSurface::present(const QVideoFrame &frame_)
// {
//     if(frame_.isValid())
//     {
//         QVideoFrame frame = frame_;
//         QImage result;

//         if (!frame.map(QAbstractVideoBuffer::ReadOnly)) {
//             return false;
//         }

//         // Formats supported by QImage don't need conversion
//         QImage::Format imageFormat = QVideoFrame::imageFormatFromPixelFormat(frame.pixelFormat());
//         if (imageFormat != QImage::Format_Invalid) {
//             result = QImage(frame.bits(), frame.width(), frame.height(), frame.bytesPerLine(), imageFormat).copy();
//         }

//         // Load from JPG
//         else if (frame.pixelFormat() == QVideoFrame::Format_Jpeg) {
//             result.loadFromData(frame.bits(), frame.mappedBytes(), "JPG");
//         }

//         // Need conversion
//         else {
//             static bool initAsmFuncsDone = false;
//             if (!initAsmFuncsDone) {
//                 qInitConvertFuncsAsm();
//                 initAsmFuncsDone = true;
//             }
//             VideoFrameConvertFunc convert = qConvertFuncs[frame.pixelFormat()];
//             if (!convert) {
//                 qWarning() << Q_FUNC_INFO << ": unsupported pixel format" << frame.pixelFormat();
//             } else {
//                 result = QImage(frame.width(), frame.height(), QImage::Format_ARGB32);
//                 convert(frame, result.bits());
//             }
//         }

//         frame.unmap();

//         QImage scaled = result.scaledToWidth(
//             this->m_maxSize.width(),
//             Qt::SmoothTransformation
//         );
//         if (m_realSize != scaled.size()) {
//             m_realSize = scaled.size();
//             emit realSizeChanged(m_realSize);
//         }

//         emit image(QPixmap::fromImage(scaled));

//         return true;
//     }
//     return false;
// }

// void VideoSurface::setDisplaySize(int width, int height)
// {
//     m_maxSize = QSize(width, height);
// }
