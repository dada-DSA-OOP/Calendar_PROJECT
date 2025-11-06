#include "daycellwidget.h"
#include "eventitem.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QDebug>

DayCellWidget::DayCellWidget(QWidget *parent)
    : QFrame(parent)
{
    setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
    setLineWidth(1);
    setStyleSheet("QFrame { border-top: 1px solid #cccccc; border-left: 1px solid #cccccc; }");

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(5, 5, 5, 5);
    layout->setSpacing(2);

    m_dateLabel = new QLabel("1");
    m_dateLabel->setAlignment(Qt::AlignTop | Qt::AlignRight);
    m_dateLabel->setStyleSheet("font-weight: 500;");

    // Container cho các sự kiện
    m_eventsContainer = new QWidget;
    m_eventsLayout = new QVBoxLayout(m_eventsContainer);
    m_eventsLayout->setContentsMargins(0, 0, 0, 0);
    m_eventsLayout->setSpacing(2);

    // --- BẮT ĐẦU THAY ĐỔI ---
    // 1. Tạo QLabel rỗng (không cần chữ "●")
    m_compactEventsIndicator = new QLabel("");
    m_compactEventsIndicator->setFixedSize(8, 8);

    m_compactEventsIndicator->setStyleSheet(
        "QLabel { "
        "  border-radius: 4px; "      // Nửa chiều rộng/cao
        "  background-color: #555555; " // Màu xám đậm mặc định
        "}"
    );
    m_compactEventsIndicator->setAlignment(Qt::AlignCenter);
    m_compactEventsIndicator->setVisible(false);

    layout->addWidget(m_dateLabel);
    layout->addWidget(m_eventsContainer, 1); // Cho phép layout sự kiện co giãn

    layout->addWidget(m_compactEventsIndicator, 1);

    setCursor(Qt::PointingHandCursor);

    // Đặt chiều cao tối thiểu để ô không bị bóp
    setMinimumHeight(60);
}

void DayCellWidget::setDate(const QDate &date, bool isCurrentMonth)
{
    m_date = date;
    m_dateLabel->setText(QString::number(date.day()));

    if (date == QDate::currentDate()) {
        m_dateLabel->setStyleSheet("font-weight: bold; color: #0078d7; padding: 2px 5px; background-color: #e5f3ff; border-radius: 4px;");
    } else if (isCurrentMonth) {
        m_dateLabel->setStyleSheet("font-weight: 500; color: black;");
    } else {
        // Ngày của tháng trước/sau
        m_dateLabel->setStyleSheet("font-weight: 400; color: #aaaaaa;");
    }

    clearEvents();
}

void DayCellWidget::addEvent(EventItem *event)
{
    m_events.append(event);

    // --- THÊM MỚI: Cập nhật màu cho hình tròn ---
    // Nếu đây là sự kiện ĐẦU TIÊN được thêm vào ngày
    if (m_events.size() == 1)
    {
        QColor firstEventColor = event->color();
        // Cập nhật lại màu nền của hình tròn
        m_compactEventsIndicator->setStyleSheet(
            QString("QLabel { "
                    "  border-radius: 4px; "
                    "  background-color: %1; "
                    "}")
                .arg(firstEventColor.name(QColor::HexRgb)) // Lấy mã màu (ví dụ: #a7d7f9)
            );
    }

    // Chỉ hiển thị tối đa 3-4 sự kiện, sau đó hiển thị "..."
    if (m_eventsLayout->count() < 3) {
        QString title = event->title();
        // Cắt bớt nếu quá dài
        if (title.length() > 20) {
            title = title.left(17) + "...";
        }

        QLabel *eventLabel = new QLabel(title);
        eventLabel->setStyleSheet(QString("background-color: %1; color: black; padding: 2px; border-radius: 3px; font-size: 8pt;")
                                      .arg(event->color().name()));
        m_eventsLayout->addWidget(eventLabel);
    } else if (m_eventsLayout->count() == 3) {
        QLabel *moreLabel = new QLabel("... thêm");
        moreLabel->setStyleSheet("font-size: 8pt; color: #555;");
        m_eventsLayout->addWidget(moreLabel);
    }
    // Nếu > 4 thì không làm gì cả
    updateEventDisplay(this->size());
}

void DayCellWidget::clearEvents()
{
    m_events.clear();
    m_compactEventsIndicator->setStyleSheet(
        "QLabel { "
        "  border-radius: 4px; "
        "  background-color: #555555; " // Trả về màu xám đậm mặc định
        "}"
    );
    QLayoutItem *child;
    while ((child = m_eventsLayout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    updateEventDisplay(this->size());
}

void DayCellWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit cellClicked(m_date, m_events);
    }
    QFrame::mouseReleaseEvent(event);
}

/**
 * @brief Được gọi mỗi khi widget thay đổi kích thước
 */
void DayCellWidget::resizeEvent(QResizeEvent *event)
{
    QFrame::resizeEvent(event); // Gọi hàm của lớp cha

    // Yêu cầu cập nhật hiển thị dựa trên kích thước mới
    updateEventDisplay(event->size());
}

/**
 * @brief Logic chính: Quyết định hiện danh sách đầy đủ hay chỉ hiện dấu chấm
 */
void DayCellWidget::updateEventDisplay(const QSize &newSize)
{
    bool hasEvents = !m_events.isEmpty();

    // Kiểm tra xem có nên vào chế độ "thu gọn" không
    // (Chiều cao nhỏ hơn 75px VÀ có sự kiện)
    bool isCompact = (newSize.height() < m_compactThreshold);

    // 1. Hiện/ẩn dấu chấm "●"
    // Chỉ hiện khi: (ở chế độ thu gọn VÀ có sự kiện)
    m_compactEventsIndicator->setVisible(isCompact && hasEvents);

    // 2. Hiện/ẩn danh sách sự kiện đầy đủ
    // Chỉ hiện khi: (KHÔNG ở chế độ thu gọn)
    m_eventsContainer->setVisible(!isCompact);
}
