#include "sessionviewwidget.h"
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
 * @brief Hàm dựng của SessionViewWidget.
 * Khởi tạo một lưới (Grid) 2x6 (Sáng/Chiều x T2-T7).
 */
SessionViewWidget::SessionViewWidget(QWidget *parent)
    : QWidget(parent)
    , m_timezoneOffsetSeconds(QDateTime::currentDateTime().offsetFromUtc())
{
    m_gridLayout = new QGridLayout(this);
    m_gridLayout->setContentsMargins(0, 0, 0, 0);
    m_gridLayout->setSpacing(0); // Không có khoảng cách giữa các ô

    // === TẠO CÁC HEADER (HÀNG VÀ CỘT) ===

    // 1. Cột 0: Header Hàng (Buổi học)
    QStringList sessionNames = { "Sáng", "Chiều" };
    for (int i = 0; i < 2; ++i) { // i = 0 (Sáng), 1 (Chiều)
        QLabel *sessionHeader = new QLabel(sessionNames[i]);
        sessionHeader->setAlignment(Qt::AlignCenter);
        sessionHeader->setStyleSheet("font-weight: bold; border: 1px solid #cccccc; padding: 8px;");
        // Thêm vào hàng i+1 (vì hàng 0 là header ngày), cột 0
        m_gridLayout->addWidget(sessionHeader, i + 1, 0);
    }

    // 2. Hàng 0: Header Cột (Ngày)
    QLocale viLocale(QLocale::Vietnamese); // Dùng Tiếng Việt
    for (int j = 0; j < 6; ++j) { // Chỉ 6 ngày (T2 - T7)
        // (j+1) vì QLocale::standaloneDayName(1) là Thứ Hai
        m_dayHeaders[j] = new QLabel(viLocale.standaloneDayName(j + 1, QLocale::ShortFormat));
        m_dayHeaders[j]->setAlignment(Qt::AlignCenter);
        m_dayHeaders[j]->setStyleSheet("font-weight: bold; border: 1px solid #cccccc; padding: 8px;");
        // Thêm vào hàng 0, cột j+1 (vì cột 0 là header buổi)
        m_gridLayout->addWidget(m_dayHeaders[j], 0, j + 1);
    }

    // 3. Ô góc (0, 0)
    m_gridLayout->addWidget(new QWidget, 0, 0);

    // === TẠO LƯỚI CÁC Ô TKB (2x6) ===
    for (int i = 0; i < 2; ++i) { // i = 0 (Sáng), 1 (Chiều)
        for (int j = 0; j < 6; ++j) { // j = 0 (T2), ..., 5 (T7)
            // TÁI SỬ DỤNG TimetableSlotWidget
            m_sessionSlots[j][i] = new TimetableSlotWidget;
            m_gridLayout->addWidget(m_sessionSlots[j][i], i + 1, j + 1);

            // Kết nối tín hiệu click từ ô (slot)
            // "chuyển tiếp" (re-emit) lên cho MainWindow
            connect(m_sessionSlots[j][i], &TimetableSlotWidget::eventClicked, this, &SessionViewWidget::eventClicked);
        }
    }

    // === CÀI ĐẶT CO GIÃN (STRETCH) ===
    m_gridLayout->setRowStretch(1, 1); // Hàng "Sáng" co giãn
    m_gridLayout->setRowStretch(2, 1); // Hàng "Chiều" co giãn
    for(int j = 1; j <= 6; ++j) {
        m_gridLayout->setColumnStretch(j, 1); // Các cột T2-T7 co giãn
    }

    setLayout(m_gridLayout);
}

// =================================================================================
// === 2. QUẢN LÝ SỰ KIỆN (PUBLIC API)
// =================================================================================

/**
 * @brief Thêm một EventItem* vào "nguồn sự thật" (m_allEvents) của View Buổi.
 * @note Hàm này gọi updateView() ngay lập tức để làm mới (cách đơn giản).
 */
void SessionViewWidget::addEvent(EventItem *event)
{
    // Tính ngày hiển thị (Local)
    QDate displayDate = event->startTime().toOffsetFromUtc(m_timezoneOffsetSeconds).date();
    // Thêm vào Map dữ liệu
    m_allEvents[displayDate].append(event);

    // Cập nhật lại toàn bộ view
    // (Vì không biết sự kiện này thuộc T2, T3... nếu m_currentMonday không khớp)
    updateView(m_currentMonday);
}

/**
 * @brief Xóa một EventItem* khỏi "nguồn sự thật" (m_allEvents)
 * và cập nhật lại toàn bộ view.
 */
void SessionViewWidget::removeEvent(EventItem *event)
{
    if (!event) return;

    QDate displayDate = event->startTime().toOffsetFromUtc(m_timezoneOffsetSeconds).date();
    if (m_allEvents.contains(displayDate)) {
        m_allEvents[displayDate].removeAll(event);

        // Cập nhật lại toàn bộ view (cách đơn giản nhất)
        // (Cách hiệu quả hơn là tìm slot[j][i] tương ứng
        // và gọi slot->removeEvent(event))
        updateView(m_currentMonday);
    }
}

// =================================================================================
// === 3. CẬP NHẬT GIAO DIỆN (UI LOGIC)
// =================================================================================

/**
 * @brief HÀM QUAN TRỌNG: Vẽ lại toàn bộ lưới TKB (Buổi).
 * Được gọi khi đổi tuần, đổi múi giờ, hoặc thêm/xóa/lọc sự kiện.
 * @param monday Ngày Thứ Hai của tuần cần hiển thị.
 */
void SessionViewWidget::updateView(const QDate &monday)
{
    m_currentMonday = monday;
    if (!m_currentMonday.isValid()) return;

    // 1. Dọn dẹp UI cũ
    clearGrid();

    QLocale viLocale(QLocale::Vietnamese);

    // 2. Định nghĩa các mốc thời gian (cứng) cho các buổi
    const QTime sangStart(7, 0);   // 7:00 AM
    const QTime sangEnd(12, 0);    // 12:00 PM
    const QTime chieuStart(13, 0); // 1:00 PM
    const QTime chieuEnd(17, 30);  // 5:30 PM

    // 3. (Quan trọng) Xây dựng lại Map sự kiện
    //    Cần làm điều này để xử lý thay đổi múi giờ
    QMap<QDate, QList<EventItem*>> newAllEvents;
    for (const auto& list : m_allEvents.values()) {
        for (EventItem* event : list) {
            // Lấy ngày hiển thị (Local) dựa trên múi giờ *hiện tại*
            QDate displayDate = event->startTime().toOffsetFromUtc(m_timezoneOffsetSeconds).date();
            newAllEvents[displayDate].append(event);
        }
    }
    m_allEvents = newAllEvents; // Cập nhật "nguồn sự thật"

    // 4. Lặp qua 6 ngày (T2 - T7)
    for (int j = 0; j < 6; ++j) {
        QDate currentDate = m_currentMonday.addDays(j); // Ngày (T2, T3...)

        // 5. Cập nhật header ngày (ví dụ: "T2 (17/11)")
        QString headerText = QString("%1 (%2)")
                                 .arg(viLocale.standaloneDayName(currentDate.dayOfWeek(), QLocale::ShortFormat))
                                 .arg(currentDate.toString("d/M"));
        m_dayHeaders[j]->setText(headerText);

        // 6. Lấy các sự kiện của ngày hôm đó
        if (m_allEvents.contains(currentDate)) {
            for (EventItem *event : m_allEvents.value(currentDate)) {

                // Bỏ qua nếu sự kiện bị lọc
                if (!event || event->isFilteredOut()) { continue; }

                // Lấy thời gian hiển thị (Local)
                QDateTime displayStart = event->startTime().toOffsetFromUtc(m_timezoneOffsetSeconds);
                QDateTime displayEnd = event->endTime().toOffsetFromUtc(m_timezoneOffsetSeconds);
                QTime eventStartTime = displayStart.time();
                QTime eventEndTime = displayEnd.time();

                // 7. Logic phân loại Buổi:
                // (Sự kiện có "chạm" vào khung giờ Buổi Sáng không?)
                if ((eventStartTime < sangEnd) && (eventEndTime > sangStart)) {
                    m_sessionSlots[j][0]->addEvent(event); // Thêm vào ô [Thứ j][Sáng]
                }

                // (Sự kiện có "chạm" vào khung giờ Buổi Chiều không?)
                if ((eventStartTime < chieuEnd) && (eventEndTime > chieuStart)) {
                    m_sessionSlots[j][1]->addEvent(event); // Thêm vào ô [Thứ j][Chiều]
                }
                // (Lưu ý: Một sự kiện 11:00 - 14:00 sẽ xuất hiện ở cả 2 ô)
            }
        }
    }
}

/**
 * @brief Hàm dọn dẹp, yêu cầu tất cả 12 ô (2x6) xóa UI sự kiện.
 */
void SessionViewWidget::clearGrid()
{
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 6; ++j) {
            m_sessionSlots[j][i]->clearEvents();
        }
    }
}

// =================================================================================
// === 4. CÀI ĐẶT (SETTINGS)
// =================================================================================

/**
 * @brief Đặt múi giờ (do MainWindow gọi).
 */
void SessionViewWidget::setTimezoneOffset(int offsetSeconds)
{
    m_timezoneOffsetSeconds = offsetSeconds;
    // Yêu cầu vẽ lại toàn bộ, vì các sự kiện có thể đã "nhảy" ngày
    updateView(m_currentMonday);
}
