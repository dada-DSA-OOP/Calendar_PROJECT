#include "timetableslotwidget.h"
#include "eventitem.h"
#include <QVBoxLayout>
#include <QLabel>

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
        if (title.length() > 25) { // Cắt bớt nếu quá dài
            title = title.left(22) + "...";
        }

        QLabel *eventLabel = new QLabel(title);
        eventLabel->setStyleSheet(QString("background-color: %1; color: black; padding: 3px; border-radius: 3px; font-size: 8pt;")
                                      .arg(event->color().name()));
        m_eventsLayout->addWidget(eventLabel);
    } else if (m_eventsLayout->count() == 2) {
        QLabel *moreLabel = new QLabel("... thêm");
        moreLabel->setStyleSheet("font-size: 8pt; color: #555;");
        m_eventsLayout->addWidget(moreLabel);
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
