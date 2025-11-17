#include "timetableviewwidget.h"
#include "timetableslotwidget.h" // Tái sử dụng widget ô TKB
#include "eventitem.h"
#include <QGridLayout>
#include <QLabel>
#include <QDebug>
#include <QLocale> // Cần để hiển thị tên Thứ (T2, T3...)

// =================================================================================
// === 1. HÀM DỰNG (CONSTRUCTOR)
// =================================================================================

/**
 * @brief Hàm dựng của TimetableViewWidget.
 * Khởi tạo một lưới (Grid) 10x6 (10 Tiết x 6 Ngày T2-T7).
 */
TimetableViewWidget::TimetableViewWidget(QWidget *parent)
    : QWidget(parent)
{
    m_gridLayout = new QGridLayout(this);
    m_gridLayout->setContentsMargins(0, 0, 0, 0);
    m_gridLayout->setSpacing(0); // Không có khoảng cách giữa các ô

    // --- ĐỊNH NGHĨA CÁC MỐC THỜI GIAN TIẾT HỌC (Logic "cứng") ---
    // (Đây là "business logic" của thời khóa biểu)
    // Tiết 1-5 (Sáng)
    m_slotStartTimes.append(QTime(7, 0));  // Tiết 1
    m_slotStartTimes.append(QTime(7, 50));  // Tiết 2
    m_slotStartTimes.append(QTime(8, 40));  // Tiết 3
    m_slotStartTimes.append(QTime(9, 45));  // Tiết 4 (Sau 20p ra chơi)
    m_slotStartTimes.append(QTime(10, 35)); // Tiết 5
    // Tiết 6-10 (Chiều)
    m_slotStartTimes.append(QTime(13, 0)); // Tiết 6
    m_slotStartTimes.append(QTime(13, 50)); // Tiết 7
    m_slotStartTimes.append(QTime(14, 40)); // Tiết 8
    m_slotStartTimes.append(QTime(15, 30)); // Tiết 9
    m_slotStartTimes.append(QTime(16, 20)); // Tiết 10
    // (Giả định mỗi tiết kéo dài 45 phút, được xử lý trong updateView)
    // ---------------------------------------------

    // === TẠO CÁC HEADER (CỘT 0 VÀ HÀNG 0) ===

    // 1. Cột 0: Header Hàng (Tiết học)
    QStringList slotNames = {
        "Tiết 1 (7:00)", "Tiết 2 (7:50)", "Tiết 3 (8:40)", "Tiết 4 (9:45)", "Tiết 5 (10:35)",
        "Tiết 6 (13:00)", "Tiết 7 (13:50)", "Tiết 8 (14:40)", "Tiết 9 (15:30)", "Tiết 10 (16:20)"
    };
    for (int i = 0; i < 10; ++i) { // i = 0 (Tiết 1) -> 9 (Tiết 10)
        QLabel *slotHeader = new QLabel(slotNames[i]);
        slotHeader->setAlignment(Qt::AlignCenter);
        slotHeader->setStyleSheet("font-weight: bold; border: 1px solid #cccccc; padding: 8px;");
        // Thêm vào hàng i+1 (vì hàng 0 là header ngày), cột 0
        m_gridLayout->addWidget(slotHeader, i + 1, 0);
    }

    // 2. Hàng 0: Header Cột (Ngày)
    QLocale viLocale(QLocale::Vietnamese); // Dùng Tiếng Việt
    for (int j = 0; j < 6; ++j) { // Chỉ 6 ngày (T2 - T7)
        // (j+1) vì QLocale::standaloneDayName(1) là Thứ Hai
        QLabel *dayHeader = new QLabel(viLocale.standaloneDayName(j + 1, QLocale::ShortFormat));
        dayHeader->setAlignment(Qt::AlignCenter);
        dayHeader->setStyleSheet("font-weight: bold; border: 1px solid #cccccc; padding: 8px;");
        m_dayHeaders[j] = dayHeader; // Lưu con trỏ để cập nhật ngày (ví dụ: "T2 (17/11)")
        // Thêm vào hàng 0, cột j+1 (vì cột 0 là header tiết)
        m_gridLayout->addWidget(dayHeader, 0, j + 1);
    }

    // 3. Ô góc (0, 0)
    m_gridLayout->addWidget(new QWidget, 0, 0);

    // === TẠO LƯỚI CÁC Ô TKB (10x6) ===
    for (int i = 0; i < 10; ++i) { // i = 0 (Tiết 1) -> 9 (Tiết 10)
        for (int j = 0; j < 6; ++j) { // j = 0 (T2) -> 5 (T7)
            // TÁI SỬ DỤNG TimetableSlotWidget
            m_slots[j][i] = new TimetableSlotWidget;
            m_gridLayout->addWidget(m_slots[j][i], i + 1, j + 1);

            // --- KỸ THUẬT "CHUYỂN TIẾP TÍN HIỆU" (Signal Forwarding) ---
            // Kết nối tín hiệu eventClicked() của Ô CON (slot)
            // với tín hiệu eventClicked() của WIDGET CHA NÀY (TimetableViewWidget).
            // -> Giúp MainWindow chỉ cần connect VỚI TimetableViewWidget
            //    thay vì connect với 60 ô con.
            connect(m_slots[j][i], &TimetableSlotWidget::eventClicked,
                    this, &TimetableViewWidget::eventClicked);
            // ---------------------------------
        }
    }

    // === CÀI ĐẶT CO GIÃN (STRETCH) ===
    for(int i = 1; i <= 10; ++i) m_gridLayout->setRowStretch(i, 1); // Các hàng tiết co giãn
    for(int j = 1; j <= 6; ++j) m_gridLayout->setColumnStretch(j, 1); // Các cột ngày co giãn

    setLayout(m_gridLayout);
}

// =================================================================================
// === 2. QUẢN LÝ SỰ KIỆN (PUBLIC API)
// =================================================================================

/**
 * @brief Thêm một EventItem* vào "nguồn sự thật" (m_allEvents) của View TKB.
 * @note Hàm này KHÔNG cập nhật UI. (updateView sẽ được gọi sau bởi MainWindow).
 */
void TimetableViewWidget::addEvent(EventItem *event)
{
    // Tính ngày hiển thị (Local) dựa trên múi giờ của view
    QDate displayDate = event->startTime().toOffsetFromUtc(m_timezoneOffsetSeconds).date();

    // Thêm sự kiện vào danh sách của ngày đó
    m_allEvents[displayDate].append(event);
}

/**
 * @brief Xóa một EventItem* khỏi "nguồn sự thật" (m_allEvents).
 * @note Hàm này KHÔNG cập nhật UI. (updateView sẽ được gọi sau).
 */
void TimetableViewWidget::removeEvent(EventItem *event)
{
    if (!event) return;

    // Tính ngày hiển thị (Local)
    QDate date = event->startTime().toOffsetFromUtc(m_timezoneOffsetSeconds).date();

    // Xóa khỏi Map dữ liệu
    if (m_allEvents.contains(date)) {
        m_allEvents[date].removeAll(event);
    }
}

// =================================================================================
// === 3. CẬP NHẬT GIAO DIỆN (UI LOGIC)
// =================================================================================

/**
 * @brief HÀM QUAN TRỌNG: Vẽ lại toàn bộ lưới TKB (Tiết).
 * Được gọi khi đổi tuần, đổi múi giờ, hoặc thêm/xóa/lọc sự kiện.
 * @param monday Ngày Thứ Hai của tuần cần hiển thị.
 */
void TimetableViewWidget::updateView(const QDate &monday)
{
    m_currentMonday = monday;
    if (!m_currentMonday.isValid()) return;

    // 1. Dọn dẹp UI cũ (xóa tất cả QLabel sự kiện khỏi 60 ô)
    clearGrid();

    // 2. (Quan trọng) Xây dựng lại Map sự kiện
    //    Cần làm điều này để xử lý thay đổi múi giờ
    //    (sự kiện có thể bị "nhảy" sang ngày khác).
    QMap<QDate, QList<EventItem*>> newAllEvents;
    for (const auto& list : m_allEvents.values()) {
        for (EventItem* event : list) {
            // Lấy ngày hiển thị (Local) dựa trên múi giờ *hiện tại*
            QDate displayDate = event->startTime().toOffsetFromUtc(m_timezoneOffsetSeconds).date();
            // Thêm vào bản đồ MỚI với đúng ngày
            newAllEvents[displayDate].append(event);
        }
    }
    m_allEvents = newAllEvents; // Cập nhật "nguồn sự thật"

    QLocale viLocale(QLocale::Vietnamese);

    // 3. Lặp qua 6 ngày (Cột, T2-T7)
    for (int j = 0; j < 6; ++j) { // j = 0 (T2) -> 5 (T7)
        QDate currentDate = m_currentMonday.addDays(j); // Ngày (T2, T3...)

        // 4. Cập nhật header ngày (ví dụ: "T2 (17/11)")
        QString headerText = QString("%1 (%2)")
                                 .arg(viLocale.standaloneDayName(currentDate.dayOfWeek(), QLocale::ShortFormat))
                                 .arg(currentDate.toString("d/M"));
        m_dayHeaders[j]->setText(headerText);

        // 5. Lấy các sự kiện của ngày hôm đó
        if (m_allEvents.contains(currentDate)) {
            for (EventItem *event : m_allEvents.value(currentDate)) {

                // Bỏ qua nếu sự kiện bị lọc
                if (!event || event->isFilteredOut()) {
                    continue;
                }

                // Lấy thời gian hiển thị (Local)
                QDateTime displayStart = event->startTime().toOffsetFromUtc(m_timezoneOffsetSeconds);
                QDateTime displayEnd = event->endTime().toOffsetFromUtc(m_timezoneOffsetSeconds);
                QTime eventStartTime = displayStart.time();
                QTime eventEndTime = displayEnd.time();

                // 6. === LOGIC GIAO NHAU (OVERLAP) ===
                // Lặp qua tất cả 10 tiết học (Hàng)
                for (int i = 0; i < 10; ++i) { // i = 0 (Tiết 1) -> 9 (Tiết 10)

                    QTime slotStartTime = m_slotStartTimes[i];
                    QTime slotEndTime = slotStartTime.addSecs(45 * 60); // Giả định Tiết học dài 45 phút

                    //                     // Điều kiện Giao nhau:
                    // (Bắt đầu sự kiện < Kết thúc Tiết) VÀ (Kết thúc sự kiện > Bắt đầu Tiết)
                    bool overlaps = (eventStartTime < slotEndTime) && (eventEndTime > slotStartTime);

                    if (overlaps) {
                        // Thêm sự kiện này vào ô [Thứ j][Tiết i]
                        // Ví dụ: Sự kiện 7:00-8:30 sẽ "chạm" vào Tiết 1 (7:00-7:45)
                        // và Tiết 2 (7:50-8:35), và sẽ được thêm vào cả 2 ô.
                        m_slots[j][i]->addEvent(event);
                    }
                }
                // --- KẾT THÚC LOGIC GIAO NHAU ---
            }
        }
    }
}

/**
 * @brief Hàm dọn dẹp, yêu cầu tất cả 60 ô (10x6) xóa UI sự kiện.
 */
void TimetableViewWidget::clearGrid()
{
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 6; ++j) {
            m_slots[j][i]->clearEvents();
        }
    }
}

// =================================================================================
// === 4. CÀI ĐẶT (SETTINGS)
// =================================================================================

/**
 * @brief Đặt múi giờ (do MainWindow gọi).
 */
void TimetableViewWidget::setTimezoneOffset(int offsetSeconds)
{
    m_timezoneOffsetSeconds = offsetSeconds;
    // Yêu cầu vẽ lại toàn bộ, vì các sự kiện có thể đã "nhảy" ngày
    // và thời gian bắt đầu/kết thúc (Local) của chúng đã thay đổi
    updateView(m_currentMonday);
}
