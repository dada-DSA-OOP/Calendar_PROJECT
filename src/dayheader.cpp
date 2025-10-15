#include "dayheader.h"
#include <QPainter>
#include <QDateTime>
#include <QTextDocument>

DayHeader::DayHeader(QWidget *parent)
    : QWidget(parent), m_scrollOffset(0), m_dayWidth(100.0), m_days(7)
{
    // Đặt chiều cao cố định cho header
    setFixedHeight(60);
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

        QDate currentDate = m_monday.addDays(day);

        if (currentDate == today) {
            QPen topBorderPen(QColor("#0078d7"), 5);
            painter.setPen(topBorderPen);
            painter.drawLine(dayRect.topLeft(), dayRect.topRight());
        }

        // --- BẮT ĐẦU PHẦN THAY ĐỔI CHÍNH ---
        // 1. Chuẩn bị chuỗi HTML để định dạng
        QString dayName = QLocale("vi_VN").standaloneDayName(currentDate.dayOfWeek(), QLocale::ShortFormat).toUpper();
        QString dateNumber = currentDate.toString("d");
        // Dùng thẻ <b> để in đậm dayName
        QString htmlText = QString("<p style='color: #0078d7;'><b>%1</b> <br> %2</p>")
                               .arg(dayName, dateNumber);

        // 2. Thiết lập vùng vẽ chữ căn lề trái, có padding 10px
        QRectF textRect = dayRect.adjusted(10, 0, -10, 0);

        // 3. Dùng QTextDocument để vẽ
        painter.save(); // Lưu trạng thái painter

        QTextDocument doc;
        doc.setHtml(htmlText);
        doc.setDefaultFont(painter.font());
        doc.setTextWidth(textRect.width());

        // Căn giữa nội dung theo chiều dọc
        qreal yOffset = (textRect.height() - doc.size().height()) / 2.0;
        painter.translate(textRect.topLeft() + QPointF(0, yOffset));

        doc.drawContents(&painter);

        painter.restore(); // Khôi phục trạng thái painter
        // --- KẾT THÚC PHẦN THAY ĐỔI CHÍNH ---
    }
}
