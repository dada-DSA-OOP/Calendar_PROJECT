#include "timetableviewwidget.h"
#include "timetableslotwidget.h"
#include "eventitem.h"
#include <QGridLayout>
#include <QLabel>
#include <QDebug>

TimetableViewWidget::TimetableViewWidget(QWidget *parent)
    : QWidget(parent)
{
    m_gridLayout = new QGridLayout(this);
    m_gridLayout->setContentsMargins(0, 0, 0, 0);
    m_gridLayout->setSpacing(0);

    // --- ĐỊNH NGHĨA CÁC MỐC THỜI GIAN TIẾT HỌC ---
    // Tiết 1-5 (Sáng)
    m_slotStartTimes.append(QTime(7, 0));  // Tiết 1
    m_slotStartTimes.append(QTime(7, 50));  // Tiết 2 (7:45 + 5p break)
    m_slotStartTimes.append(QTime(8, 40));  // Tiết 3 (8:35 + 5p break)
    m_slotStartTimes.append(QTime(9, 45));  // Tiết 4 (9:25 + 20p break)
    m_slotStartTimes.append(QTime(10, 35)); // Tiết 5 (10:30 + 5p break)
    // Tiết 6-10 (Chiều)
    m_slotStartTimes.append(QTime(13, 0)); // Tiết 6
    m_slotStartTimes.append(QTime(13, 50)); // Tiết 7 (13:45 + 5p break)
    m_slotStartTimes.append(QTime(14, 40)); // Tiết 8 (14:35 + 5p break)
    m_slotStartTimes.append(QTime(15, 30)); // Tiết 9 (15:25 + 5p break)
    m_slotStartTimes.append(QTime(16, 20)); // Tiết 10 (16:15 + 5p break)
    // Giả định 1 tiết 45 phút
    // ---------------------------------------------

    // === TẠO CÁC HEADER (CỘT 0 VÀ HÀNG 0) ===

    // Cột 0: Header Tiết học
    QStringList slotNames = {
        "Tiết 1 (7:00)", "Tiết 2 (7:50)", "Tiết 3 (8:40)", "Tiết 4 (9:45)", "Tiết 5 (10:35)",
        "Tiết 6 (13:00)", "Tiết 7 (13:50)", "Tiết 8 (14:40)", "Tiết 9 (15:30)", "Tiết 10 (16:20)"
    };
    for (int i = 0; i < 10; ++i) {
        QLabel *slotHeader = new QLabel(slotNames[i]);
        slotHeader->setAlignment(Qt::AlignCenter);
        slotHeader->setStyleSheet("font-weight: bold; border: 1px solid #cccccc; padding: 8px;");
        m_gridLayout->addWidget(slotHeader, i + 1, 0);
    }

    // Hàng 0: Header Ngày
    QLocale viLocale(QLocale::Vietnamese);
    for (int j = 0; j < 6; ++j) { // Chỉ 6 ngày (T2 - T7)
        QLabel *dayHeader = new QLabel(viLocale.standaloneDayName(j + 1, QLocale::ShortFormat)); // T2, T3...
        dayHeader->setAlignment(Qt::AlignCenter);
        dayHeader->setStyleSheet("font-weight: bold; border: 1px solid #cccccc; padding: 8px;");
        m_dayHeaders[j] = dayHeader;
        m_gridLayout->addWidget(dayHeader, 0, j + 1);
    }

    // Góc trên bên trái
    m_gridLayout->addWidget(new QWidget, 0, 0);

    // === TẠO LƯỚI CÁC Ô TIẾT HỌC ===
    for (int i = 0; i < 10; ++i) { // 10 tiết (hàng)
        for (int j = 0; j < 6; ++j) { // 6 ngày (cột)
            m_slots[j][i] = new TimetableSlotWidget;
            m_gridLayout->addWidget(m_slots[j][i], i + 1, j + 1);

            // --- THÊM DÒNG KẾT NỐI NÀY ---
            // Kết nối tín hiệu của ô con với tín hiệu của widget này
            connect(m_slots[j][i], &TimetableSlotWidget::eventClicked,
                    this, &TimetableViewWidget::eventClicked);
            // ---------------------------------
        }
    }

    // Cho phép các ô co giãn
    for(int i = 1; i <= 10; ++i) m_gridLayout->setRowStretch(i, 1);
    for(int j = 1; j <= 6; ++j) m_gridLayout->setColumnStretch(j, 1);

    setLayout(m_gridLayout);
}

// MỚI: Thêm hàm removeEvent
void TimetableViewWidget::removeEvent(EventItem *event)
{
    if (!event) return;

    // SỬA LỖI: Phải tìm đúng ngày hiển thị của sự kiện
    QDate date = event->startTime().toOffsetFromUtc(m_timezoneOffsetSeconds).date();
    if (m_allEvents.contains(date)) {
        m_allEvents[date].removeAll(event);
    }
}

void TimetableViewWidget::addEvent(EventItem *event)
{
    // Lưu sự kiện
    // SỬA LỖI: Chuyển sang ngày hiển thị (display date)
    QDate displayDate = event->startTime().toOffsetFromUtc(m_timezoneOffsetSeconds).date();
    m_allEvents[displayDate].append(event);
}

void TimetableViewWidget::clearGrid()
{
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 6; ++j) {
            m_slots[j][i]->clearEvents();
        }
    }
}

/**
 * @brief (ĐÃ SỬA) Cập nhật lại toàn bộ TKB cho tuần
 */
void TimetableViewWidget::updateView(const QDate &monday)
{
    m_currentMonday = monday;
    clearGrid(); // Xóa tất cả sự kiện cũ khỏi các ô

    QMap<QDate, QList<EventItem*>> newAllEvents;

    for (const auto& list : m_allEvents.values()) {
        for (EventItem* event : list) {
            // Tính toán lại ngày hiển thị (displayDate) với múi giờ MỚI
            QDate displayDate = event->startTime().toOffsetFromUtc(m_timezoneOffsetSeconds).date();

            // Thêm vào bản đồ MỚI với đúng ngày
            newAllEvents[displayDate].append(event);
        }
    }
    m_allEvents = newAllEvents;

    QLocale viLocale(QLocale::Vietnamese);

    for (int j = 0; j < 6; ++j) { // 6 ngày (Cột, T2-T7)
        QDate currentDate = m_currentMonday.addDays(j);

        // Cập nhật header (ví dụ: T2 (14/10))
        QString headerText = QString("%1 (%2)")
                                 .arg(viLocale.standaloneDayName(currentDate.dayOfWeek(), QLocale::ShortFormat))
                                 .arg(currentDate.toString("d/M"));
        m_dayHeaders[j]->setText(headerText);

        // Lấy các sự kiện của ngày này
        if (m_allEvents.contains(currentDate)) {
            for (EventItem *event : m_allEvents.value(currentDate)) {
                if (!event || event->isFilteredOut()) {
                    continue; // Bỏ qua sự kiện đã bị lọc
                }

                QDateTime displayStart = event->startTime().toOffsetFromUtc(m_timezoneOffsetSeconds);
                QDateTime displayEnd = event->endTime().toOffsetFromUtc(m_timezoneOffsetSeconds);
                QTime eventStartTime = displayStart.time(); // <-- ĐÃ SỬA
                QTime eventEndTime = displayEnd.time(); // <-- ĐÃ SỬA

                // --- LOGIC MỚI: Duyệt qua tất cả 10 tiết học ---
                for (int i = 0; i < 10; ++i) { // 10 tiết (Hàng)

                    QTime slotStartTime = m_slotStartTimes[i];
                    // Giả định 1 tiết học kéo dài 45 phút
                    QTime slotEndTime = slotStartTime.addSecs(45 * 60);

                    // Điều kiện Overlap: (Bắt đầu sự kiện < Kết thúc Tiết) VÀ (Kết thúc sự kiện > Bắt đầu Tiết)
                    bool overlaps = (eventStartTime < slotEndTime) && (eventEndTime > slotStartTime);

                    if (overlaps) {
                        // Thêm sự kiện vào ô [ngày][tiết]
                        // Ví dụ: Sự kiện 7:00-11:30 sẽ được thêm vào
                        // ô (T2, Tiết 1), (T2, Tiết 2), (T2, Tiết 3), (T2, Tiết 4), (T2, Tiết 5)
                        m_slots[j][i]->addEvent(event);
                    }
                }
                // --- KẾT THÚC LOGIC MỚI ---
            }
        }
    }
}

void TimetableViewWidget::setTimezoneOffset(int offsetSeconds)
{
    m_timezoneOffsetSeconds = offsetSeconds;
    // Gọi hàm cập nhật của view này
    updateView(m_currentMonday);
}
