#ifndef TIMETABLESLOTWIDGET_H
#define TIMETABLESLOTWIDGET_H

#include <QFrame> // Lớp cơ sở (bắt buộc)
#include <QList>   // Cần cho m_events

// --- Forward Declarations (Khai báo trước) ---
// (Khai báo tên lớp để tránh include .h đầy đủ, tăng tốc độ biên dịch)
class QVBoxLayout;
class EventItem;

/**
 * @brief Lớp TimetableSlotWidget là một QFrame đại diện cho MỘT Ô
 * trong lưới Thời khóa biểu (TKB).
 *
 * Lớp này được TÁI SỬ DỤNG bởi cả TimetableViewWidget (cho các tiết)
 * và SessionViewWidget (cho các buổi Sáng/Chiều).
 * Nó chịu trách nhiệm hiển thị một danh sách rút gọn các sự kiện
 * và phát tín hiệu khi một sự kiện được click.
 */
class TimetableSlotWidget : public QFrame
{
    // Q_OBJECT là bắt buộc để sử dụng signals và slots
    Q_OBJECT

public:
    // === 1. HÀM DỰNG & API CÔNG KHAI ===
    // (Các hàm mà view cha sẽ gọi)

    explicit TimetableSlotWidget(QWidget *parent = nullptr);

    /**
     * @brief Thêm một sự kiện vào ô này.
     * (Tạo một QLabel rút gọn và thêm vào m_eventsLayout).
     */
    void addEvent(EventItem *event);

    /**
     * @brief Xóa SẠCH tất cả sự kiện (chỉ UI) khỏi ô.
     * Được gọi bởi view cha khi update.
     */
    void clearEvents();

signals:
    // === 2. TÍN HIỆU (SIGNALS) ===
    // (Báo cáo sự kiện click lên cho view cha)

    /**
     * @brief Phát ra khi người dùng click vào một *sự kiện cụ thể* (QLabel) trong ô.
     * @param item Con trỏ tới EventItem đã được click.
     */
    void eventClicked(EventItem *item);

protected:
    // === 3. HÀM SỰ KIỆN (PROTECTED EVENT HANDLERS) ===
    // (Các hàm override từ QFrame/QWidget)

    /**
     * @brief Bắt sự kiện click chuột lên các *QLabel con* (các nhãn sự kiện)
     * mà nó quản lý.
     */
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    // === 4. BIẾN THÀNH VIÊN (MEMBER VARIABLES) ===

    /**
     * @brief Layout dọc, sắp xếp các nhãn sự kiện (QLabel) từ trên xuống.
     */
    QVBoxLayout *m_eventsLayout;

    /**
     * @brief Danh sách "dữ liệu" tham khảo, chứa con trỏ tới các sự kiện
     * thuộc về ô này.
     */
    QList<EventItem*> m_events;
};

#endif // TIMETABLESLOTWIDGET_H
