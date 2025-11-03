#include "timeruler.h"
#include <QPainter>
#include <QStyleOption>

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

void TimeRuler::setHourHeight(double height)
{
    m_hourHeight = height;
    update(); // Yêu cầu vẽ lại với chiều cao mới
}

void TimeRuler::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QStyleOption opt;
    opt.initFrom(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);

    painter.setRenderHint(QPainter::Antialiasing);

    // --- THAY ĐỔI 1: ĐỔI MÀU SANG ĐEN TUYỆT ĐỐI ---
    painter.setPen(QColor("#000000")); // Đen tuyền

    // --- THAY ĐỔI 2: ĐỔI FONT SANG BOLD ---
    QFont font("Segoe UI", 8);
    font.setWeight(QFont::Bold); // Đặt trọng lượng font (Đậm)
    painter.setFont(font);

    // --- LOGIC VẼ VẠCH (Giữ nguyên từ trước) ---

    // Ngưỡng hiển thị:
    bool drawHalfHour = (m_hourHeight >= 50);
    bool drawQuarterHour = (m_hourHeight >= 100);

    // Chuẩn bị các loại bút vẽ
    QPen solidPen(QColor("#cccccc"), 1, Qt::SolidLine); // Vạch giờ chính (nét liền)
    QPen dashedPen(QColor("#cccccc"), 1, Qt::DashLine); // Vạch 30 phút (nét đứt)
    QPen dottedPen(QColor("#dddddd"), 1, Qt::DotLine);  // Vạch 15/45 phút (nét chấm, mờ hơn)

    for (int i = 0; i < 24; ++i)
    {
        double y = i * m_hourHeight - m_scrollOffset;

        if (y < -m_hourHeight || y > height() + m_hourHeight)
            continue;

        // --- THÊM MỚI: Đảm bảo bút vẽ chữ luôn là màu đen ---
        // (Tránh bị ảnh hưởng bởi bút vẽ vạch kẻ của vòng lặp trước)
        painter.setPen(QColor("#000000"));

        // Vẽ chữ (07:00, 13:00, v.v.)
        QTime time(i, 0);
        QString timeString = time.toString("HH:mm");
        QRectF textRect(0, y - m_hourHeight, width() - 5, m_hourHeight);
        painter.drawText(textRect, Qt::AlignBottom | Qt::AlignRight, timeString);

        // 1. Vẽ vạch GIỜ CHÍNH (luôn luôn)
        painter.setPen(solidPen);
        painter.drawLine(QPointF(width() - 10, y), QPointF(width(), y));

        // 2. Logic vẽ vạch chi tiết (Giữ nguyên)
        if (drawQuarterHour) {
            // Vạch 15 phút (nét chấm)
            double y15 = y + (m_hourHeight * 0.25);
            painter.setPen(dottedPen);
            painter.drawLine(QPointF(width() - 5, y15), QPointF(width(), y15));

            // Vạch 30 phút (nét đứt, dài hơn 15)
            double y30 = y + (m_hourHeight * 0.5);
            painter.setPen(dashedPen);
            painter.drawLine(QPointF(width() - 7, y30), QPointF(width(), y30));

            // Vạch 45 phút (nét chấm)
            double y45 = y + (m_hourHeight * 0.75);
            painter.setPen(dottedPen);
            painter.drawLine(QPointF(width() - 5, y45), QPointF(width(), y45));

        } else if (drawHalfHour) {
            // Chỉ vẽ 30
            double y30 = y + (m_hourHeight / 2.0);
            painter.setPen(dashedPen);
            painter.drawLine(QPointF(width() - 5, y30), QPointF(width(), y30));
        }
    }
}
