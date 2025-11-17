#include "monthviewwidget.h"
#include "daycellwidget.h"   // Lớp widget cho từng ô ngày
#include "daydetaildialog.h" // Dialog "chi tiết" khi click vào ô
#include "eventitem.h"
#include <QGridLayout>

// =================================================================================
// === 1. HÀM DỰNG (CONSTRUCTOR)
// =================================================================================

/**
 * @brief Hàm dựng của MonthViewWidget.
 * Khởi tạo một lưới cố định 6x7 (42 ô) gồm các DayCellWidget.
 */
MonthViewWidget::MonthViewWidget(QWidget *parent)
    : QWidget(parent)
    , m_timezoneOffsetSeconds(QDateTime::currentDateTime().offsetFromUtc())
{
    m_gridLayout = new QGridLayout(this);
    m_gridLayout->setContentsMargins(0, 0, 0, 0); // Không có lề
    m_gridLayout->setSpacing(0);                   // Không có khoảng cách

    // Tạo 42 ô (6 hàng x 7 cột)
    for (int i = 0; i < 42; ++i) {
        DayCellWidget *cell = new DayCellWidget;
        m_dayCells.append(cell); // Lưu con trỏ để truy cập sau
        m_gridLayout->addWidget(cell, i / 7, i % 7); // Thêm vào lưới

        // --- Kết nối tín hiệu (Signals) ---

        // (1) Kết nối khi click vào VÙNG TRỐNG của ô
        //     -> Mở dialog "Chi tiết ngày" (DayDetailDialog)
        connect(cell, &DayCellWidget::cellClicked, this, &MonthViewWidget::onCellClicked);

        // (2) Kết nối khi click vào MỘT SỰ KIỆN CỤ THỂ (QLabel) trong ô
        //     -> "Chuyển tiếp" (re-emit) tín hiệu này lên cho MainWindow
        connect(cell, &DayCellWidget::eventClicked, this, &MonthViewWidget::eventClicked);
    }

    setLayout(m_gridLayout);
}

// =================================================================================
// === 2. QUẢN LÝ SỰ KIỆN (PUBLIC API)
// =================================================================================

/**
 * @brief Thêm một EventItem* vào "nguồn sự thật" (m_allEvents) của View Tháng.
 * @note Hàm này chỉ cập nhật dữ liệu (Map), không cập nhật UI.
 * UI được cập nhật hàng loạt trong updateView().
 */
void MonthViewWidget::addEvent(EventItem *event)
{
    // Tính toán ngày hiển thị (Local) dựa trên múi giờ của view
    QDate displayDate = event->startTime().toOffsetFromUtc(m_timezoneOffsetSeconds).date();

    // Thêm sự kiện vào danh sách của ngày đó
    m_allEvents[displayDate].append(event);
}

/**
 * @brief Xóa một EventItem* khỏi "nguồn sự thật" (m_allEvents) VÀ
 * yêu cầu ô (cell) tương ứng cập nhật UI của nó.
 */
void MonthViewWidget::removeEvent(EventItem *event)
{
    if (!event) return;

    // Tính ngày hiển thị (Local)
    QDate displayDate = event->startTime().toOffsetFromUtc(m_timezoneOffsetSeconds).date();

    // 1. Xóa khỏi danh sách dữ liệu (Map)
    if (m_allEvents.contains(displayDate)) {
        m_allEvents[displayDate].removeAll(event);

        // 2. Tìm ô (cell) UI tương ứng
        for(DayCellWidget *cell : m_dayCells) {
            if (cell->date() == displayDate) {
                // 3. Yêu cầu ô đó tự xóa sự kiện khỏi UI của nó
                cell->removeEvent(event);
                break;
            }
        }
    }
}

/**
 * @brief HÀM QUAN TRỌNG: Vẽ lại toàn bộ lưới 42 ô.
 * Được gọi khi đổi tháng (Tới/Lui) hoặc khi múi giờ thay đổi.
 * @param date Một ngày bất kỳ trong tháng cần hiển thị.
 */
void MonthViewWidget::updateView(const QDate &date)
{
    // 1. Dọn dẹp UI cũ (xóa tất cả QLabel sự kiện)
    clearGrid();

    // 2. Tính toán ngày bắt đầu của lưới (Ngày Thứ Hai của tuần đầu tiên)
    QDate firstDayOfMonth = date.addDays(-(date.day() - 1)); // Ngày 1 của tháng
    int daysToMonday = firstDayOfMonth.dayOfWeek() - 1;    // Lùi về Thứ Hai
    QDate startDate = firstDayOfMonth.addDays(-daysToMonday); // Ngày bắt đầu (ô 0,0)

    // 3. (Quan trọng) Xây dựng lại Map sự kiện
    //    Cần làm điều này mỗi lần update để xử lý trường hợp
    //    thay đổi múi giờ (sự kiện có thể bị "nhảy" sang ngày khác)
    QMap<QDate, QList<EventItem*>> newAllEvents;
    for (const auto& list : m_allEvents.values()) {
        for (EventItem* event : list) {
            QDate displayDate = event->startTime().toOffsetFromUtc(m_timezoneOffsetSeconds).date();
            newAllEvents[displayDate].append(event);
        }
    }
    m_allEvents = newAllEvents; // Cập nhật "nguồn sự thật"

    // 4. Lặp qua 42 ô và điền dữ liệu
    for (int i = 0; i < 42; ++i) {
        QDate currentDate = startDate.addDays(i); // Ngày của ô hiện tại
        DayCellWidget *cell = m_dayCells[i];

        // 5. Đặt ngày và style (trong tháng / ngoài tháng)
        bool isCurrentMonth = (currentDate.month() == date.month());
        cell->setDate(currentDate, isCurrentMonth);

        // 6. Thêm các sự kiện (đã lọc) vào ô UI
        if (m_allEvents.contains(currentDate)) {
            for (EventItem *event : m_allEvents.value(currentDate)) {
                if (event && !event->isFilteredOut()) // Chỉ thêm nếu không bị lọc
                {
                    cell->addEvent(event);
                }
            }
        }
    }
}

/**
 * @brief Đặt múi giờ (do MainWindow gọi).
 */
void MonthViewWidget::setTimezoneOffset(int offsetSeconds)
{
    m_timezoneOffsetSeconds = offsetSeconds;
    // Yêu cầu vẽ lại toàn bộ, vì các sự kiện có thể đã "nhảy" ngày
    updateView(QDate::currentDate());
}

// =================================================================================
// === 3. SLOT NỘI BỘ (PRIVATE SLOTS)
// =================================================================================

/**
 * @brief Được gọi khi người dùng click vào *vùng trống* của một DayCellWidget.
 * Mở dialog "chi tiết" (dạng danh sách) cho ngày đó.
 */
void MonthViewWidget::onCellClicked(const QDate &date, const QList<EventItem*> &events)
{
    // Mở DayDetailDialog (hộp thoại danh sách)
    DayDetailDialog dialog(date, events, this);
    dialog.exec();
}

// =================================================================================
// === 4. HÀM TRỢ GIÚP (PRIVATE HELPERS)
// =================================================================================

/**
 * @brief Hàm dọn dẹp, yêu cầu tất cả 42 ô xóa UI sự kiện của chúng.
 */
void MonthViewWidget::clearGrid()
{
    for (DayCellWidget *cell : m_dayCells) {
        cell->clearEvents();
    }
}
