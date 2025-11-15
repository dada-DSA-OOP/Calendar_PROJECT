#include "timetableslotwidget.h"
#include "eventitem.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QEvent>
#include <QVariant>

TimetableSlotWidget::TimetableSlotWidget(QWidget *parent)
    : QFrame(parent)
{
    setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
    setLineWidth(1);
    // Đặt màu nền và viền cho ô
    setStyleSheet("QFrame { border-top: 1px solid #cccccc; border-left: 1px solid #cccccc; }");

    m_eventsLayout = new QVBoxLayout(this);
    m_eventsLayout->setContentsMargins(4, 4, 4, 4);
    m_eventsLayout->setSpacing(2);
    m_eventsLayout->addStretch(1);

    // Đảm bảo ô có thể co giãn
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMinimumHeight(50); // Chiều cao tối thiểu cho 1 tiết
}

void TimetableSlotWidget::addEvent(EventItem *event)
{
    m_events.append(event);

    // Chỉ hiển thị 1-2 sự kiện
    if (m_eventsLayout->count() < 2) {
        QString title = event->title();

        QLabel *eventLabel = new QLabel(title);
        eventLabel->setStyleSheet(QString("background-color: %1; color: black; padding: 3px; border-radius: 3px; font-size: 8pt;")
                                      .arg(event->color().name()));

        eventLabel->setWordWrap(true);

        // --- CẬP NHẬT ---
        eventLabel->setCursor(Qt::PointingHandCursor); // (1) Đổi con trỏ chuột

        // (2) Lưu con trỏ EventItem vào QLabel
        eventLabel->setProperty("eventItem", QVariant::fromValue(event));

        // (3) Cài đặt bộ lọc sự kiện để bắt click
        eventLabel->installEventFilter(this);
        // ---------------

        m_eventsLayout->insertWidget(m_eventsLayout->count() - 1, eventLabel);

    } else if (m_eventsLayout->count() == 2) {
        QLabel *moreLabel = new QLabel("... thêm");
        moreLabel->setStyleSheet("font-size: 8pt; color: #555;");

        m_eventsLayout->insertWidget(m_eventsLayout->count() - 1, moreLabel);
    }
}

void TimetableSlotWidget::clearEvents()
{
    m_events.clear();
    QLayoutItem *child;
    while ((child = m_eventsLayout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }
}

// MỚI: Triển khai hàm eventFilter
bool TimetableSlotWidget::eventFilter(QObject *watched, QEvent *event)
{
    // 1. Chỉ quan tâm đến sự kiện nhấn chuột trái
    if (event->type() == QEvent::MouseButtonRelease)
    {
        // 2. Kiểm tra xem đối tượng có phải là QLabel không (dù không bắt buộc)
        if (qobject_cast<QLabel*>(watched))
        {
            // 3. Lấy lại con trỏ EventItem* mà chúng ta đã lưu
            QVariant eventData = watched->property("eventItem");
            if (eventData.isValid()) {
                EventItem *item = eventData.value<EventItem*>();
                if (item) {
                    // 4. Phát tín hiệu
                    emit eventClicked(item);
                    return true; // Báo rằng chúng ta đã xử lý sự kiện
                }
            }
        }
    }

    // Chuyển tiếp các sự kiện khác
    return QFrame::eventFilter(watched, event);
}
