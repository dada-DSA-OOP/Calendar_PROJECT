#ifndef TIMETABLEVIEWWIDGET_H
#define TIMETABLEVIEWWIDGET_H

#include <QWidget> // Lớp cơ sở (bắt buộc)
#include <QDate>   // Cần cho QDate và QMap
#include <QMap>    // Cần cho m_allEvents
#include <QTime>   // Cần cho m_slotStartTimes
#include <QList>   // (Thêm) Cần cho QList

// --- Forward Declarations (Khai báo trước) ---
// (Khai báo tên lớp để tránh include .h đầy đủ, tăng tốc độ biên dịch)
class QGridLayout;
class EventItem;
class TimetableSlotWidget;
class QLabel;

/**
 * @brief Lớp TimetableViewWidget là widget cho "Chế độ xem TKB theo Tiết".
 *
 * Lớp này quản lý một lưới (Grid) 10x6 (10 Tiết x 6 Ngày T2-T7).
 * Nó tái sử dụng TimetableSlotWidget cho từng ô
 * và phân loại các sự kiện vào các Tiết học (slots)
 * dựa trên logic giao nhau (overlap) về thời gian.
 */
class TimetableViewWidget : public QWidget
{
    // Q_OBJECT là bắt buộc để sử dụng signals và slots
    Q_OBJECT

public:
    // === 1. HÀM DỰNG & API CÔNG KHAI ===
    // (Các hàm mà MainWindow sẽ gọi)

    explicit TimetableViewWidget(QWidget *parent = nullptr);

    /**
     * @brief Thêm một sự kiện vào Map dữ liệu (m_allEvents) của view này.
     * (Không cập nhật UI ngay lập tức).
     */
    void addEvent(EventItem *event);

    /**
     * @brief Xóa một sự kiện khỏi Map dữ liệu (m_allEvents).
     * (Không cập nhật UI ngay lập tức).
     */
    void removeEvent(EventItem *event);

    /**
     * @brief Hàm cập nhật chính: Vẽ lại toàn bộ lưới TKB (Tiết)
     * dựa trên ngày Thứ Hai của tuần.
     * @param date Ngày Thứ Hai của tuần cần hiển thị.
     */
    void updateView(const QDate &date);

public slots:
    // === 2. CÁC SLOT CÔNG KHAI (PUBLIC SLOTS) ===
    // (Các hàm "phản ứng" với MainWindow)

    /**
     * @brief Đặt chênh lệch múi giờ (so với UTC) để hiển thị thời gian chính xác.
     * @param offsetSeconds Số giây chênh lệch.
     */
    void setTimezoneOffset(int offsetSeconds);

signals:
    // === 3. TÍN HIỆU (SIGNALS) ===
    // (Báo cáo sự kiện click lên cho MainWindow)

    /**
     * @brief Phát ra khi người dùng click vào một *sự kiện cụ thể*
     * bên trong một TimetableSlotWidget.
     * (Tín hiệu này được "chuyển tiếp" từ TimetableSlotWidget).
     * @param item Con trỏ tới EventItem đã được click.
     */
    void eventClicked(EventItem *item);

private:
    // === 4. HÀM VÀ BIẾN NỘI BỘ (PRIVATE) ===

    /**
     * @brief Hàm dọn dẹp: Yêu cầu tất cả 60 ô (10x6) xóa UI sự kiện.
     */
    void clearGrid();

    // --- 4a. Con trỏ UI ---
    QGridLayout *m_gridLayout; // Layout lưới chính

    /**
     * @brief Mảng 2D (6 cột, 10 hàng) chứa con trỏ tới 60 ô TKB.
     * Truy cập: m_slots[Ngày][Tiết]
     * (ví dụ: m_slots[0][0] = Thứ 2, Tiết 1)
     */
    TimetableSlotWidget* m_slots[6][10]; // [T2-T7][Tiết 1-10]

    /**
     * @brief Mảng chứa con trỏ tới 6 nhãn header (T2, T3, ... T7).
     */
    QLabel* m_dayHeaders[6];

    // --- 4b. Biến Dữ liệu & Trạng thái ---
    QDate m_currentMonday; // Ngày Thứ Hai của tuần đang hiển thị

    /**
     * @brief "Nguồn sự thật" (Source of Truth) của riêng TimetableViewWidget.
     * Ánh xạ một Ngày (QDate) tới Danh sách các sự kiện (EventItem*)
     * thuộc về ngày đó (theo múi giờ hiển thị).
     */
    QMap<QDate, QList<EventItem*>> m_allEvents;

    /**
     * @brief Danh sách các mốc thời gian "cứng" (hard-coded)
     * định nghĩa giờ bắt đầu của 10 tiết học.
     */
    QList<QTime> m_slotStartTimes;

    /**
     * @brief Múi giờ hiển thị hiện tại (tính bằng giây so với UTC).
     */
    int m_timezoneOffsetSeconds;
};

#endif // TIMETABLEVIEWWIDGET_H
