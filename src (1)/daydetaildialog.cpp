#include "daydetaildialog.h"
#include "eventitem.h" // Include file thật
#include <QVBoxLayout>
#include <QListWidget>
#include <QLabel>

DayDetailDialog::DayDetailDialog(const QDate &date, const QList<EventItem*> &events, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(date.toString("dddd, dd MMMM yyyy"));
    setMinimumSize(350, 250);

    QVBoxLayout *layout = new QVBoxLayout(this);
    m_listWidget = new QListWidget;

    // Sắp xếp sự kiện theo thời gian bắt đầu
    QList<EventItem*> sortedEvents = events;
    std::sort(sortedEvents.begin(), sortedEvents.end(), [](EventItem* a, EventItem* b) {
        return a->startTime() < b->startTime();
    });

    if (sortedEvents.isEmpty()) {
        m_listWidget->addItem("Không có sự kiện nào.");
    } else {
        for (EventItem *event : sortedEvents) {
            QString startTime = event->startTime().toString("HH:mm");
            QString endTime = event->endTime().toString("HH:mm");
            QString title = event->title();

            QListWidgetItem *item = new QListWidgetItem(QString("%1 - %2  |  %3").arg(startTime, endTime, title));
            item->setData(Qt::UserRole, QColor(event->color())); // Lưu màu
            m_listWidget->addItem(item);
        }
    }

    // Tùy chọn: Dùng delegate để vẽ màu cho đẹp hơn
    // (Bỏ qua cho đơn giản, bạn có thể thêm sau)

    layout->addWidget(m_listWidget);
    setLayout(layout);
}
