#include "daycellwidget.h"
#include "eventitem.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QDebug>
#include <QEvent>
#include <QVariant>

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

    // --- Cập nhật màu cho hình tròn ---
    if (m_events.size() == 1)
    {
        QColor firstEventColor = event->color();
        m_compactEventsIndicator->setStyleSheet(
            QString("QLabel { "
                    "  border-radius: 4px; "
                    "  background-color: %1; "
                    "}")
                .arg(firstEventColor.name(QColor::HexRgb))
            );
    }

    if (m_eventsLayout->count() < 3) {
        QString title = event->title();
        if (title.length() > 20) {
            title = title.left(17) + "...";
        }

        QLabel *eventLabel = new QLabel(title);
        eventLabel->setStyleSheet(QString("background-color: %1; color: black; padding: 2px; border-radius: 3px; font-size: 8pt;")
                                      .arg(event->color().name()));

        // --- CẬP NHẬT ---
        eventLabel->setCursor(Qt::PointingHandCursor); // (1) Đổi con trỏ chuột

        // (2) Lưu con trỏ EventItem vào QLabel
        eventLabel->setProperty("eventItem", QVariant::fromValue(event));

        // (3) Cài đặt bộ lọc sự kiện để bắt click
        eventLabel->installEventFilter(this);
        // ---------------

        m_eventsLayout->addWidget(eventLabel);
    } else if (m_eventsLayout->count() == 3) {
        QLabel *moreLabel = new QLabel("... thêm");
        moreLabel->setStyleSheet("font-size: 8pt; color: #555;");
        m_eventsLayout->addWidget(moreLabel);
    }
    // Nếu > 4 thì không làm gì cả
    updateEventDisplay(this->size());
}

// MỚI: Triển khai hàm removeEvent
void DayCellWidget::removeEvent(EventItem *event)
{
    if (!event || !m_events.contains(event)) return;

    // 1. Xóa khỏi danh sách
    m_events.removeAll(event);

    // 2. Xóa tất cả các widget QLabel khỏi layout
    QLayoutItem *child;
    while ((child = m_eventsLayout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    // 3. Chạy lại logic addEvent cho các sự kiện còn lại
    // (Đây là cách đơn giản và an toàn nhất để đảm bảo UI đúng)
    QList<EventItem*> remainingEvents = m_events;
    m_events.clear(); // Xóa list tạm thời

    for (EventItem* evt : remainingEvents) {
        addEvent(evt); // Gọi lại addEvent, nó sẽ tự điền lại m_events
    }

    // 4. Cập nhật lại màu sắc (nếu cần)
    if (m_events.isEmpty()) {
        // Trả về màu mặc định nếu không còn sự kiện
        m_compactEventsIndicator->setStyleSheet(
            "QLabel { border-radius: 4px; background-color: #555555; }"
            );
    } else {
        // Lấy màu của sự kiện đầu tiên mới
        QColor firstEventColor = m_events.first()->color();
        m_compactEventsIndicator->setStyleSheet(
            QString("QLabel { border-radius: 4px; background-color: %1; }")
                .arg(firstEventColor.name(QColor::HexRgb))
            );
    }

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

// MỚI: Triển khai hàm eventFilter
bool DayCellWidget::eventFilter(QObject *watched, QEvent *event)
{
    // 1. Chỉ quan tâm đến sự kiện nhấn chuột trái
    if (event->type() == QEvent::MouseButtonRelease)
    {
        // 2. Kiểm tra xem có phải là QLabel (eventLabel) không
        if (qobject_cast<QLabel*>(watched))
        {
            // 3. Lấy lại con trỏ EventItem* mà chúng ta đã lưu
            QVariant eventData = watched->property("eventItem");
            if (eventData.isValid()) {
                EventItem *item = eventData.value<EventItem*>();
                if (item) {
                    // 4. Phát tín hiệu eventClicked
                    emit eventClicked(item);
                    return true; // Báo rằng chúng ta đã xử lý sự kiện
                }
            }
        }
    }

    // Chuyển tiếp các sự kiện khác (ví dụ: click vào "... thêm")
    return QFrame::eventFilter(watched, event);
}
