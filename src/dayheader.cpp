#include "dayheader.h"
#include <QPainter>
#include <QDateTime>     // Dùng cho QDate, QLocale
#include <QTextDocument> // Dùng để vẽ HTML
#include <QStyleOption>  // Dùng để vẽ nền theo QSS
#include <QPaintEvent>   // (Bao gồm đầy đủ)

// =================================================================================
// === 1. HÀM DỰNG (CONSTRUCTOR)
// =================================================================================

DayHeader::DayHeader(QWidget *parent)
    : QWidget(parent)
    , m_scrollOffset(0) // Độ cuộn ngang (tính bằng pixel)
    , m_dayWidth(0)     // Chiều rộng (pixel) của 1 cột ngày
    , m_days(7)         // Số ngày hiển thị (mặc định 7)
    , m_rightMargin(0)  // Lề phải (dành cho thanh cuộn dọc)
{
    // Cố định chiều cao của header là 60px
    setFixedHeight(60);
    // Khởi tạo ngày "mỏ neo" ban đầu
    m_monday = QDate::currentDate().addDays(-(QDate::currentDate().dayOfWeek() - 1));
    m_currentMonday = m_monday; // (m_currentMonday sẽ được MainWindow cập nhật)
}

// =================================================================================
// === 2. HÀM VẼ CHÍNH (PAINT EVENT)
// =================================================================================

/**
 * @brief Hàm vẽ chính, được Qt gọi bất cứ khi nào widget cần được vẽ lại
 * (ví dụ: khi gọi update(), resize(), hoặc cuộn).
 */
void DayHeader::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    // 1. Vẽ nền (background) theo stylesheet (QSS)
    // (Điều này cho phép QSS như #dayHeaderWidget { background-color: ... } hoạt động)
    QStyleOption opt;
    opt.initFrom(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);

    painter.setRenderHint(QPainter::Antialiasing); // Vẽ mượt mà

    QDate today = QDate::currentDate();

    // 2. Lặp qua từng ngày để vẽ
    for (int day = 0; day < m_days; ++day) {

        // Tính toán vị trí X của cột ngày,
        // **trừ đi m_scrollOffset** để cuộn đồng bộ với CalendarView
        double x = day * m_dayWidth - m_scrollOffset;

        // Tạo hình chữ nhật cho cột ngày này
        QRectF dayRect(x, 0, m_dayWidth, height());

        // Bỏ qua việc vẽ nếu cột ngày nằm hoàn toàn bên ngoài tầm nhìn
        if (dayRect.right() < 0 || dayRect.left() > width()) {
            continue;
        }

        // 3. Vẽ đường kẻ dọc phân cách các ngày (trừ ngày đầu tiên)
        if (day > 0) {
            painter.setPen(QColor("#AAAAAA")); // Màu xám nhạt
            painter.drawLine(QPointF(x, 0), QPointF(x, height()));
        }

        // 4. Lấy ngày hiện tại (ví dụ: T2, T3, T4...)
        QDate currentDate = m_currentMonday.addDays(day);

        // 5. Vẽ đường kẻ màu xanh lam ở trên cùng nếu là "hôm nay"
        if (currentDate == today) {
            QPen topBorderPen(QColor("#0078d7"), 5); // Dày 5px
            painter.setPen(topBorderPen);
            painter.drawLine(dayRect.topLeft(), dayRect.topRight());
        }

        // 6. Chuẩn bị text HTML để vẽ
        // Lấy tên thứ (ví dụ: "THỨ 2")
        QString dayName = QLocale("vi_VN").standaloneDayName(currentDate.dayOfWeek(), QLocale::ShortFormat).toUpper();
        // Lấy ngày (ví dụ: "17")
        QString dateString = currentDate.toString("d");

        // Tạo chuỗi HTML
        // (Lưu ý: Bạn đã sửa font-weight thành 600 và 400,
        // và text-align: left, điều này là chính xác)
        QString htmlText = QString(
                               "<div style='color: #0078d7; font-family: Segoe UI; text-align: left;'>"
                               "<span style='font-size: 11pt; font-weight: 600;'>%1</span>" // Tên thứ
                               "<br>"
                               "<span style='font-size: 12pt; font-weight: 400;'>%2</span>" // Ngày
                               "</div>"
                               ).arg(dayName, dateString);

        // 7. Dùng QTextDocument để vẽ HTML (cách này tốt hơn QPainter::drawText)
        painter.save(); // Lưu trạng thái painter

        QTextDocument doc;
        doc.setHtml(htmlText);
        doc.setDefaultFont(painter.font());
        doc.setTextWidth(dayRect.width()); // Giới hạn chiều rộng văn bản

        // Căn giữa nội dung HTML theo chiều dọc
        qreal yOffset = (dayRect.height() - doc.size().height()) / 2.0;
        painter.translate(dayRect.topLeft() + QPointF(0, yOffset));

        doc.drawContents(&painter); // Vẽ HTML

        painter.restore(); // Khôi phục trạng thái painter
    }
}

// =================================================================================
// === 3. HÀM SỰ KIỆN VÀ CẬP NHẬT (SLOTS & HELPERS)
// =================================================================================

/**
 * @brief Được MainWindow gọi để cập nhật ngày bắt đầu (ví dụ: khi nhấn Tới/Lui).
 * @param monday Ngày bắt đầu mới (thường là Thứ Hai).
 */
void DayHeader::updateDates(const QDate &monday)
{
    m_currentMonday = monday;
    update(); // Yêu cầu vẽ lại (kích hoạt paintEvent)
}

/**
 * @brief Slot này được kết nối với thanh cuộn ngang của CalendarView.
 * @param x Vị trí pixel cuộn ngang mới.
 */
void DayHeader::setScrollOffset(int x)
{
    m_scrollOffset = x;
    update(); // Yêu cầu vẽ lại để cuộn đồng bộ
}

/**
 * @brief Được MainWindow gọi khi số ngày hiển thị thay đổi (1, 3, 5, 7).
 * Tính toán lại chiều rộng của một cột ngày.
 */
void DayHeader::setNumberOfDays(int days)
{
    if (days > 0) {
        m_days = days;
        // Tính toán lại chiều rộng 1 ngày
        double effectiveWidth = (double)width() - m_rightMargin;
        m_dayWidth = (m_days > 0) ? (effectiveWidth / m_days) : 0;
        update(); // Yêu cầu vẽ lại
    }
}

/**
 * @brief Được MainWindow gọi để báo cho Header biết thanh cuộn dọc đang chiếm bao nhiêu pixel.
 * Điều này đảm bảo các cột ngày của Header thẳng hàng với các cột của CalendarView.
 * @param margin Chiều rộng (pixels) của thanh cuộn dọc.
 */
void DayHeader::setRightMargin(int margin)
{
    m_rightMargin = margin;
    // Tính toán lại chiều rộng 1 ngày
    double effectiveWidth = (double)width() - m_rightMargin;
    m_dayWidth = (m_days > 0) ? (effectiveWidth / m_days) : 0;
    update(); // Yêu cầu vẽ lại
}

/**
 * @brief Được Qt gọi khi kích thước của widget (cửa sổ) thay đổi.
 * Tính toán lại chiều rộng của một cột ngày.
 */
void DayHeader::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event); // Gọi hàm của lớp cha

    // Tính toán lại chiều rộng 1 ngày
    double effectiveWidth = (double)width() - m_rightMargin;
    m_dayWidth = (m_days > 0) ? (effectiveWidth / m_days) : 0;

    // (Không cần gọi update() ở đây, vì Qt sẽ tự động kích hoạt paintEvent sau khi resize)
}
