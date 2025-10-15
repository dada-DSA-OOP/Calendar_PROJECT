#include "timeruler.h"
#include <QPainter>

TimeRuler::TimeRuler(QWidget *parent)
    : QWidget(parent), m_scrollOffset(0), m_hourHeight(60.0)
{
    // Đặt chiều rộng cố định cho thước đo
    setFixedWidth(60);
}

void TimeRuler::setScrollOffset(int y)
{
    m_scrollOffset = y;
    update();
}

void TimeRuler::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setFont(QFont("Segoe UI", 8));
    painter.setPen(Qt::gray);

    for (int hour = 0; hour < 24; ++hour) {
        double y = hour * m_hourHeight - m_scrollOffset;
        QRectF hourRect(0, y, width(), m_hourHeight);

        if (hourRect.bottom() < 0 || hourRect.top() > height()) {
            continue;
        }

        // Vẽ mốc thời gian
        QString timeText = QString("%1:00").arg(hour);
        painter.drawText(hourRect.adjusted(5, 5, -5, -5), Qt::AlignTop | Qt::AlignRight, timeText);

        // Vẽ đường kẻ ngang
        painter.setPen(Qt::lightGray);
        painter.drawLine(QPointF(width() - 10, y), QPointF(width(), y));
    }
}
