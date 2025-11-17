#include "daydetaildialog.h"
#include "eventitem.h" // Cần để truy cập thông tin sự kiện (startTime, title, color)
#include <QVBoxLayout>
#include <QListWidget>
#include <QLabel>
#include <QDateTime> // Cần để chuyển đổi UTC sang LocalTime
#include <algorithm> // Cần cho std::sort

/**
 * @brief Hàm dựng của DayDetailDialog.
 * @param date Ngày được click (dùng để hiển thị trên tiêu đề cửa sổ).
 * @param events Danh sách các sự kiện của ngày đó (được truyền từ DayCellWidget).
 * @param parent Widget cha.
 */
DayDetailDialog::DayDetailDialog(const QDate &date, const QList<EventItem*> &events, QWidget *parent)
    : QDialog(parent)
{
    // Đặt tiêu đề cửa sổ theo ngày (ví dụ: "Thứ Hai, 17 Tháng 11 2025")
    setWindowTitle(date.toString("dddd, dd MMMM yyyy"));
    setMinimumSize(350, 250); // Đặt kích thước tối thiểu

    QVBoxLayout *layout = new QVBoxLayout(this);
    m_listWidget = new QListWidget; // Widget chính hiển thị danh sách

    // --- Sắp xếp sự kiện ---
    // 1. Sao chép danh sách sự kiện
    QList<EventItem*> sortedEvents = events;

    // 2. Sắp xếp danh sách sao chép dựa trên thời gian bắt đầu (startTime)
    //    (Luôn so sánh UTC để đảm bảo tính tuyệt đối)
    std::sort(sortedEvents.begin(), sortedEvents.end(), [](EventItem* a, EventItem* b) {
        return a->startTime() < b->startTime();
    });

    // --- Hiển thị sự kiện ---
    if (sortedEvents.isEmpty()) {
        m_listWidget->addItem("Không có sự kiện nào.");
    } else {
        // Lặp qua danh sách đã sắp xếp
        for (EventItem *event : sortedEvents) {

            // Chuyển đổi thời gian UTC (lưu trữ) sang LocalTime (hiển thị)
            // Lưu ý: .toLocalTime() dùng múi giờ HỆ THỐNG.
            // Để chính xác hơn, bạn nên truyền m_timezoneOffsetSeconds vào dialog
            // và dùng .toOffsetFromUtc(m_timezoneOffsetSeconds)
            QDateTime localStart = event->startTime().toLocalTime();
            QDateTime localEnd = event->endTime().toLocalTime();

            QString startTime = localStart.toString("HH:mm");
            QString endTime = localEnd.toString("HH:mm");
            QString title = event->title();

            // Tạo một item cho danh sách (ví dụ: "09:00 - 10:30  |  Họp nhóm")
            QListWidgetItem *item = new QListWidgetItem(QString("%1 - %2  |  %3").arg(startTime, endTime, title));

            // Lưu trữ màu sắc của sự kiện vào item
            // (Bạn có thể dùng QStyledItemDelegate để đọc màu này và vẽ nền cho item)
            item->setData(Qt::UserRole, QColor(event->color()));

            m_listWidget->addItem(item);
        }
    }

    layout->addWidget(m_listWidget);
    setLayout(layout);
}
