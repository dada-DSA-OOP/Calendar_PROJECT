#include "dayheader.h"
#include <QPainter>
#include <QDateTime>

DayHeader::DayHeader(QWidget *parent)
    : QWidget(parent), m_scrollOffset(0), m_dayWidth(100.0), m_days(7)
{
    // Đặt chiều cao cố định cho header
    setFixedHeight(60);
    // Đặt màu nền
    setStyleSheet("background-color: #f5f5f5; border-bottom: 1px solid #dcdcdc;");
    m_monday = QDate::currentDate().addDays(-(QDate::currentDate().dayOfWeek() - 1));
}

void DayHeader::updateDates(const QDate &monday)
{
    m_monday = monday;
    update(); // Vẽ lại với ngày mới
}

void DayHeader::setScrollOffset(int x)
{
    m_scrollOffset = x;
    update(); // Yêu cầu vẽ lại
}

void DayHeader::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setFont(QFont("Segoe UI", 8));
    painter.setPen(Qt::gray);

    for (int day = 0; day < m_days; ++day) {
        // Tính toán vị trí x, có bù trừ theo thanh cuộn
        double x = day * m_dayWidth - m_scrollOffset;

        QRectF dayRect(x, 0, m_dayWidth, height());
        if (dayRect.right() < 0 || dayRect.left() > width()) {
            continue; // Bỏ qua vẽ nếu nằm ngoài màn hình
        }

        // Vẽ đường phân cách
        if (day > 0) {
            painter.setPen(Qt::lightGray);
            painter.drawLine(QPointF(x, 10), QPointF(x, height() - 10));
        }

        // Định dạng và vẽ chữ
        QDate currentDate = m_monday.addDays(day); // SỬA LẠI DÒNG NÀY
        QString dayName = QLocale("vi_VN").standaloneDayName(currentDate.dayOfWeek(), QLocale::ShortFormat).toUpper();
        QString dateText = dayName + " " + currentDate.toString("d/M");

        painter.setPen(Qt::black);
        painter.drawText(dayRect, Qt::AlignCenter, dateText);
    }
}
