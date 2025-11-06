#include "dayheader.h"
#include <QPainter>
#include <QDateTime>
#include <QTextDocument>
#include <QStyleOption>

DayHeader::DayHeader(QWidget *parent)
    : QWidget(parent), m_scrollOffset(0), m_dayWidth(0), m_days(7), m_rightMargin(0)
{
    // Đặt chiều cao cố định cho header
    setFixedHeight(60);
    m_monday = QDate::currentDate().addDays(-(QDate::currentDate().dayOfWeek() - 1));
}

void DayHeader::setRightMargin(int margin)
{
    m_rightMargin = margin;

    // Tính toán lại m_dayWidth ngay lập tức
    double effectiveWidth = (double)width() - m_rightMargin;
    if (m_days > 0) {
        m_dayWidth = effectiveWidth / m_days;
    } else {
        m_dayWidth = 0;
    }
    update(); // Yêu cầu vẽ lại
}

void DayHeader::updateDates(const QDate &monday)
{
    m_currentMonday = monday;
    update(); // Kích hoạt paintEvent
}

void DayHeader::setScrollOffset(int x)
{
    m_scrollOffset = x;
    update(); // Yêu cầu vẽ lại
}

void DayHeader::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    // Đoạn code này yêu cầu widget vẽ nền theo stylesheet trước
    QStyleOption opt;
    opt.initFrom(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
    painter.setRenderHint(QPainter::Antialiasing);

    // THAY ĐỔI: Bỏ Bold ở đây, vì HTML sẽ xử lý việc in đậm
    painter.setFont(QFont("Segoe UI", 9));

    QDate today = QDate::currentDate();

    for (int day = 0; day < m_days; ++day) {
        double x = day * m_dayWidth - m_scrollOffset;
        QRectF dayRect(x, 0, m_dayWidth, height());

        if (dayRect.right() < 0 || dayRect.left() > width()) {
            continue;
        }

        if (day > 0) {
            painter.setPen(QColor("#AAAAAA"));
            painter.drawLine(QPointF(x, 0), QPointF(x, height()));
        }

        // --- SỬA LỖI 1: m_monday -> m_currentMonday ---
        QDate currentDate = m_currentMonday.addDays(day);

        // --- SỬA LỖI 2: 'today' đã được định nghĩa bên ngoài ---
        if (currentDate == today) {
            QPen topBorderPen(QColor("#0078d7"), 5);
            painter.setPen(topBorderPen);
            painter.drawLine(dayRect.topLeft(), dayRect.topRight());
        }

        // --- BẮT ĐẦU PHẦN THAY ĐỔI ---

        // 1. Lấy tên thứ đầy đủ (ví dụ: "Thứ 2")
        QString dayName = QLocale("vi_VN").standaloneDayName(currentDate.dayOfWeek(), QLocale::ShortFormat).toUpper();

        // 2. Lấy ngày tháng theo định dạng (d/M)
        QString dateString = currentDate.toString("d");

        // 3. Sửa lại HTML
        // - Căn giữa toàn bộ text (text-align: center)
        // - Giảm kích thước font của ngày tháng (12pt)
        QString htmlText = QString(
                               "<div style='color: #0078d7; font-family: Segoe UI; text-align: left;'>"
                               "<span style='font-size: 11pt; font-weight: 600;'>%1</span>" // Tên thứ: "THỨ 2"
                               "<br>"
                               "<span style='font-size: 12pt; font-weight: 400;'>%2</span>" // Ngày: "(3/11)"
                               "</div>"
                               ).arg(dayName, dateString);

        // 4. Căn lề text (bỏ 10px lề trái/phải cũ)
        QRectF textRect = dayRect.adjusted(0, 0, 0, 0);

        // 5. Dùng QTextDocument để vẽ
        painter.save();

        QTextDocument doc;
        doc.setHtml(htmlText);
        doc.setDefaultFont(painter.font());
        doc.setTextWidth(textRect.width()); // Cho phép text căn giữa

        // Căn giữa nội dung theo chiều dọc
        qreal yOffset = (textRect.height() - doc.size().height()) / 2.0;
        painter.translate(textRect.topLeft() + QPointF(0, yOffset));

        doc.drawContents(&painter);

        painter.restore();
    }
}

void DayHeader::setNumberOfDays(int days)
{
    if (days > 0) {
        m_days = days;

        // SỬA LẠI PHẦN TÍNH TOÁN: Trừ đi lề phải
        double effectiveWidth = (double)width() - m_rightMargin;
        m_dayWidth = effectiveWidth / m_days;

        update();
    }
}

void DayHeader::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    // SỬA LẠI PHẦN TÍNH TOÁN: Trừ đi lề phải
    double effectiveWidth = (double)width() - m_rightMargin;

    if (m_days > 0) {
        m_dayWidth = effectiveWidth / m_days;
    } else {
        m_dayWidth = 0;
    }
}
