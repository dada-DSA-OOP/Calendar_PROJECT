#ifndef SESSIONVIEWWIDGET_H
#define SESSIONVIEWWIDGET_H

#include <QWidget> // Lớp cơ sở (bắt buộc)
#include <QDate>   // Cần cho QDate và QMap
#include <QMap>    // Cần cho m_allEvents
#include <QTime>   // Cần cho logic phân loại buổi

// --- Forward Declarations (Khai báo trước) ---
class QGridLayout;
class EventItem;
class TimetableSlotWidget; // Tái sử dụng widget ô TKB
class QLabel;

/**
 * @brief Lớp SessionViewWidget là widget cho "Chế độ xem TKB theo Buổi".
 *
 * Lớp này quản lý một lưới (Grid) 2x6 (Sáng/Chiều x T2-T7).
 * Nó tái sử dụng TimetableSlotWidget cho từng ô
 * và phân loại các sự kiện vào "Sáng" hoặc "Chiều"
 * dựa trên thời gian bắt đầu/kết thúc của chúng.
 */
class SessionViewWidget : public QWidget
{
    // Q_OBJECT là bắt buộc để sử dụng signals và slots
    Q_OBJECT

public:
    // === 1. HÀM DỰNG & API CÔNG KHAI ===
    // (Các hàm mà MainWindow sẽ gọi)

    explicit SessionViewWidget(QWidget *parent = nullptr);

    /**
     * @brief Thêm một sự kiện vào Map dữ liệu (m_allEvents) của view này.
     * (Cập nhật UI ngay lập tức bằng cách gọi updateView).
     */
    void addEvent(EventItem *event);

    /**
     * @brief Xóa một sự kiện khỏi Map dữ liệu (m_allEvents) VÀ
     * cập nhật lại toàn bộ UI.
     */
    void removeEvent(EventItem *event);

    /**
     * @brief Hàm cập nhật chính: Vẽ lại toàn bộ lưới TKB (Buổi)
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
     * @brief Hàm dọn dẹp: Yêu cầu tất cả 12 ô (2x6) xóa UI sự kiện.
     */
    void clearGrid();

    // --- 4a. Con trỏ UI ---
    QGridLayout *m_gridLayout; // Layout lưới chính

    /**
     * @brief Mảng 2D (6 cột, 2 hàng) chứa con trỏ tới 12 ô TKB.
     * Truy cập: m_sessionSlots[Ngày][Buổi]
     * (ví dụ: m_sessionSlots[0][0] = Thứ 2, Sáng)
     */
    TimetableSlotWidget* m_sessionSlots[6][2]; // [T2-T7][Sáng-Chiều]

    /**
     * @brief Mảng chứa con trỏ tới 6 nhãn header (T2, T3, ... T7).
     */
    QLabel* m_dayHeaders[6];

    // --- 4b. Biến Dữ liệu & Trạng thái ---
    QDate m_currentMonday; // Ngày Thứ Hai của tuần đang hiển thị

    /**
     * @brief "Nguồn sự thật" (Source of Truth) của riêng SessionViewWidget.
     * Ánh xạ một Ngày (QDate) tới Danh sách các sự kiện (EventItem*)
     * thuộc về ngày đó (theo múi giờ hiển thị).
     */
    QMap<QDate, QList<EventItem*>> m_allEvents;

    /**
     * @brief Múi giờ hiển thị hiện tại (tính bằng giây so với UTC).
     */
    int m_timezoneOffsetSeconds;
};

#endif // SESSIONVIEWWIDGET_H
