#include "timeruler.h"
#include <QPainter>
#include <QPaintEvent>
#include <QTime>
#include <QFont>
#include <QtMath> // Cần cho qMax, qMin
#include <QStyleOption>

TimeRuler::TimeRuler(QWidget *parent)
    : QWidget(parent), m_scrollOffset(0), m_hourHeight(60.0)
    , m_timezoneOffsetSeconds(QDateTime::currentDateTime().offsetFromUtc())
{
    m_use24HourFormat = false; // Mặc định là 12h (AM/PM)
    // Đặt chiều rộng cố định cho thước đo
    setFixedWidth(60);
}

void TimeRuler::setTimezoneOffset(int offsetSeconds)
{
    m_timezoneOffsetSeconds = offsetSeconds;
    update(); // Yêu cầu vẽ lại
}

void TimeRuler::setScrollOffset(int y)
{
    m_scrollOffset = y;
    update();
}

void TimeRuler::setHourHeight(double height)
{
    m_hourHeight = height;
    update(); // Yêu cầu vẽ lại với chiều cao mới
}

void TimeRuler::set24HourFormat(bool is24Hour)
{
    m_use24HourFormat = is24Hour;
    update(); // Yêu cầu vẽ lại với định dạng mới
}

void TimeRuler::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);

    painter.setRenderHint(QPainter::TextAntialiasing);
    painter.setFont(QFont("Segoe UI", 8));

    // Dịch chuyển toàn bộ canvas theo giá trị cuộn
    painter.translate(0, -m_scrollOffset);

    // Tối ưu hóa: Chỉ vẽ các giờ trong khu vực nhìn thấy
    QRect visibleRect = event->rect().translated(0, m_scrollOffset);

    // Tính toán giờ đầu tiên và cuối cùng cần vẽ
    int firstHour = qMax(0, static_cast<int>(visibleRect.top() / m_hourHeight) - 1);
    int lastHour = qMin(24, static_cast<int>(visibleRect.bottom() / m_hourHeight) + 1);

    // Vòng lặp chỉ qua các giờ cần vẽ
    for (int hour = firstHour; hour <= lastHour; ++hour)
    {
        // --- 1. Tính toán vị trí Y ---
        // yPos là vị trí pixel CHÍNH XÁC của đường kẻ giờ (0:00, 1:00, 2:00...)
        double yPos = hour * m_hourHeight;

        // --- 2. Vẽ đường kẻ giờ chính (đậm) ---
        painter.setPen(QColor("#a0a0a0")); // Màu đường kẻ chính
        // Vẽ một đường kẻ ngắn ở mép
        painter.drawLine(QPointF(width() - 10, yPos), QPointF(width(), yPos));


        // --- 3. Vẽ đường kẻ nửa giờ (chấm) ---
        if (hour < 24) // Không vẽ đường 24:30
        {
            painter.setPen(QPen(QColor("#c0c0c0"), 1, Qt::DotLine));
            double yHalfPos = yPos + (m_hourHeight / 2.0);
            painter.drawLine(QPointF(width() - 5, yHalfPos), QPointF(width(), yHalfPos));
        }

        // --- 4. Vẽ nhãn giờ (Text) ---
        if (hour == 24) continue; // Không vẽ nhãn "24:00"

        QTime time(hour, 0);
        QString format;

        // Xử lý định dạng 12h/24h
        if (m_use24HourFormat) {
            format = "HH:00";
        } else {
            format = "h AP"; // Chỉ hiện "1 PM", "2 PM"...
        }

        QString offsetString;
        if (m_timezoneOffsetSeconds == 0) {
            offsetString = "UTC";
        } else {
            int totalMinutes = m_timezoneOffsetSeconds / 60;
            int offsetHours = totalMinutes / 60;
            int offsetMinutes = qAbs(totalMinutes % 60);

            QString sign = (offsetHours >= 0) ? "+" : "";
            offsetString = QString("UTC%1%2").arg(sign).arg(offsetHours);
            if (offsetMinutes > 0) {
                offsetString += QString(":%1").arg(offsetMinutes, 2, 10, QChar('0'));
            }
        }

        QString timeText = time.toString(QString("HH:00 '%1'").arg(offsetString));

        // ** PHẦN SỬA LỖI MẤU CHỐT **
        // Tạo một hình chữ nhật có chiều cao m_hourHeight
        // và đặt TÂM của nó vào đúng vị trí yPos.
        // Do đó, đỉnh (top) của nó là (yPos - m_hourHeight / 2.0).

        painter.setPen(QColor("#333333")); // Màu chữ
        QRectF textRect(0,
                        yPos - (m_hourHeight / 2.0),
                        width() - 15, // Padding 15px từ mép phải
                        m_hourHeight);

        // Vẽ chữ canh phải và canh giữa theo chiều dọc
        painter.drawText(textRect, Qt::AlignRight | Qt::AlignVCenter, timeText);
    }
}
