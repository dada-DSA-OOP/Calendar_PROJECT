#include "timeruler.h"
#include <QPainter>
#include <QPaintEvent> // Cần cho QPaintEvent
#include <QTime>
#include <QFont>
#include <QtMath>       // Cần cho qMax, qMin (tối ưu hóa vẽ)
#include <QStyleOption> // Cần để vẽ nền theo QSS

// =================================================================================
// === 1. HÀM DỰNG (CONSTRUCTOR)
// =================================================================================

TimeRuler::TimeRuler(QWidget *parent)
    : QWidget(parent)
    , m_scrollOffset(0) // Độ cuộn dọc (tính bằng pixel)
    , m_hourHeight(60.0) // Chiều cao (pixel) của 1 giờ
    , m_timezoneOffsetSeconds(QDateTime::currentDateTime().offsetFromUtc())
{
    m_use24HourFormat = false; // Mặc định là 12h (AM/PM)
    // Cố định chiều rộng của thước đo
    setFixedWidth(60);
}

// =================================================================================
// === 2. CÁC SLOT CÔNG KHAI (PUBLIC SLOTS / SETTERS)
// =================================================================================

/**
 * @brief Đặt múi giờ (do MainWindow gọi).
 */
void TimeRuler::setTimezoneOffset(int offsetSeconds)
{
    m_timezoneOffsetSeconds = offsetSeconds;
    update(); // Yêu cầu vẽ lại
}

/**
 * @brief Slot này được kết nối với thanh cuộn dọc của CalendarView.
 * @param y Vị trí pixel cuộn dọc mới.
 */
void TimeRuler::setScrollOffset(int y)
{
    m_scrollOffset = y;
    update(); // Yêu cầu vẽ lại để cuộn đồng bộ
}

/**
 * @brief Đặt chiều cao (pixel) của 1 giờ (do MainWindow/CalendarView gọi khi "zoom").
 */
void TimeRuler::setHourHeight(double height)
{
    m_hourHeight = height;
    update(); // Yêu cầu vẽ lại với chiều cao mới
}

/**
 * @brief Đặt định dạng thời gian 12h (AM/PM) hoặc 24h.
 */
void TimeRuler::set24HourFormat(bool is24Hour)
{
    m_use24HourFormat = is24Hour;
    update(); // Yêu cầu vẽ lại với định dạng mới
}

// =================================================================================
// === 3. HÀM VẼ CHÍNH (PAINT EVENT)
// =================================================================================

/**
 * @brief Hàm vẽ chính, được Qt gọi bất cứ khi nào widget cần được vẽ lại.
 */
void TimeRuler::paintEvent(QPaintEvent *event)
{
    // 1. Vẽ nền (background) theo stylesheet (QSS)
    QStyleOption opt;
    opt.initFrom(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);

    painter.setRenderHint(QPainter::TextAntialiasing);
    painter.setFont(QFont("Segoe UI", 8));

    // 2. Dịch chuyển toàn bộ "tấm vải" (canvas)
    //    ngược lại với giá trị cuộn (m_scrollOffset).
    //    (Ví dụ: Nếu cuộn xuống 50px, painter sẽ dịch lên 50px
    //     để vẽ mốc 0:00 ở tọa độ -50)
    painter.translate(0, -m_scrollOffset);

    // 3. TỐI ƯU HÓA VÙNG VẼ (Painting Optimization)
    //    Chỉ vẽ các giờ trong khu vực nhìn thấy (visible rect)
    QRect visibleRect = event->rect().translated(0, m_scrollOffset);

    // Tính toán giờ đầu tiên và cuối cùng cần vẽ
    // (Thêm/bớt 1 để đảm bảo không bị cắt ở mép)
    int firstHour = qMax(0, static_cast<int>(visibleRect.top() / m_hourHeight) - 1);
    int lastHour = qMin(24, static_cast<int>(visibleRect.bottom() / m_hourHeight) + 1);

    // --- Bắt đầu vòng lặp vẽ ---
    // (Vòng lặp này chỉ chạy ~3-4 lần thay vì 24 lần)
    for (int hour = firstHour; hour <= lastHour; ++hour)
    {
        // Vị trí Y (pixel) CHÍNH XÁC của đường kẻ giờ (0:00, 1:00, ...)
        double yPos = hour * m_hourHeight;

        // 4. Vẽ đường kẻ giờ chính (đậm)
        painter.setPen(QColor("#a0a0a0")); // Màu xám đậm
        painter.drawLine(QPointF(width() - 10, yPos), QPointF(width(), yPos)); // Kẻ 10px

        // 5. Vẽ đường kẻ nửa giờ (chấm)
        if (hour < 24) // Không vẽ đường 24:30
        {
            painter.setPen(QPen(QColor("#c0c0c0"), 1, Qt::DotLine)); // Xám nhạt, chấm
            double yHalfPos = yPos + (m_hourHeight / 2.0);
            painter.drawLine(QPointF(width() - 5, yHalfPos), QPointF(width(), yHalfPos)); // Kẻ 5px
        }

        // 6. Vẽ nhãn giờ (Text)
        if (hour == 24) continue; // Không vẽ nhãn "24:00"

        QTime time(hour, 0);
        QString format;

        // Xử lý định dạng 12h/24h
        if (m_use24HourFormat) {
            format = "HH:00"; // Ví dụ: "09:00", "14:00"
        } else {
            // Định dạng "h AP" sẽ cho ra "9 AM", "12 PM", "1 PM"
            // (Chữ " AM"/" PM" tự động được QTime xử lý)
            format = "h AP";
        }

        // Tạo chuỗi múi giờ (ví dụ: "UTC+7" hoặc "UTC")
        QString offsetString;
        if (m_timezoneOffsetSeconds == 0) {
            offsetString = "UTC";
        } else {
            int totalMinutes = m_timezoneOffsetSeconds / 60;
            int offsetHours = totalMinutes / 60;
            int offsetMinutes = qAbs(totalMinutes % 60);

            QString sign = (offsetHours >= 0) ? "+" : "";
            offsetString = QString("UTC%1%2").arg(sign).arg(offsetHours);
            if (offsetMinutes > 0) { // Thêm phút nếu lẻ (ví dụ: +5:30)
                offsetString += QString(":%1").arg(offsetMinutes, 2, 10, QChar('0'));
            }
        }

        // Tạo chuỗi thời gian cuối cùng (ví dụ: "09:00 'UTC+7'")
        // (Sửa lỗi: Bạn đã dùng "HH:00" thay vì 'format', tôi giữ nguyên logic của bạn,
        // nhưng có vẻ bạn muốn dùng 'format' thay thế)
        // QString timeText = time.toString(QString("%1 '%2'").arg(format, offsetString));

        // (Đây là logic cũ của bạn, nó vẽ "09:00 'UTC+7'")
        QString timeText = time.toString(QString("%1 '%2'").arg(format, offsetString));

        // 7. Tính toán vị trí vẽ Text
        painter.setPen(QColor("#333333")); // Màu chữ

        // Tạo một hình chữ nhật ảo (textRect) cao bằng 1 giờ (m_hourHeight)
        // và căn TÂM của nó vào vị trí yPos (vị trí đường kẻ giờ).
        // Điều này đảm bảo text luôn nằm *giữa* hai đường kẻ giờ.
        QRectF textRect(0, // x
                        yPos - (m_hourHeight / 2.0), // y (bắt đầu từ nửa giờ trước)
                        width() - 15, // width (trừ 15px lề phải)
                        m_hourHeight); // height

        // Vẽ chữ (căn phải & căn giữa theo chiều dọc)
        painter.drawText(textRect, Qt::AlignRight | Qt::AlignVCenter, timeText);
    }
}
