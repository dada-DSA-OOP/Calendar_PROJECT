#ifndef DAYCELLWIDGET_H
#define DAYCELLWIDGET_H

#include <QFrame>
#include <QDate>
#include <QList>

// --- Forward Declarations ---
// (Khai báo tên lớp để tránh include .h đầy đủ, tăng tốc độ biên dịch)
class QLabel;
class QVBoxLayout;
class EventItem;

/**
 * @brief Đại diện cho MỘT ô ngày trong MonthViewWidget (Chế độ xem Tháng).
 *
 * Lớp này chịu trách nhiệm hiển thị số ngày, danh sách các sự kiện,
 * và tự động chuyển sang chế độ "thu gọn" (hiển thị dấu chấm '●')
 * khi kích thước ô quá nhỏ.
 */
class DayCellWidget : public QFrame
{
    // BẮT BUỘC PHẢI CÓ để sử dụng signals và slots
    Q_OBJECT

public:
    // === 1. HÀM DỰNG VÀ API CÔNG KHAI ===
    // (Các hàm mà MonthViewWidget sẽ gọi)

    explicit DayCellWidget(QWidget *parent = nullptr);

    /**
     * @brief Đặt ngày cho ô và cập nhật style (ví dụ: ngày hôm nay, ngày mờ).
     * @param date Ngày của ô.
     * @param isCurrentMonth 'true' nếu ngày thuộc tháng đang xem, 'false' nếu mờ đi.
     */
    void setDate(const QDate &date, bool isCurrentMonth);

    /**
     * @brief Thêm một sự kiện vào ô (cả danh sách dữ liệu và hiển thị QLabel).
     */
    void addEvent(EventItem *event);

    /**
     * @brief Xóa một sự kiện cụ thể khỏi ô (cả dữ liệu và QLabel).
     */
    void removeEvent(EventItem *event);

    /**
     * @brief Xóa SẠCH tất cả sự kiện (dữ liệu và UI) khỏi ô.
     */
    void clearEvents();

    /**
     * @brief Trả về ngày mà ô này đang đại diện.
     */
    QDate date() const { return m_date; }

signals:
    // === 2. TÍN HIỆU (SIGNALS) ===
    // (Báo cáo sự kiện click lên cho MonthViewWidget)

    /**
     * @brief Phát ra khi người dùng click vào *vùng trống* của ô.
     * Dùng để tạo sự kiện mới.
     * @param date Ngày của ô đã được click.
     * @param events Danh sách các sự kiện hiện có trong ô.
     */
    void cellClicked(const QDate &date, const QList<EventItem*> &events);

    /**
     * @brief Phát ra khi người dùng click vào một *sự kiện cụ thể* (QLabel) trong ô.
     * Dùng để mở dialog chỉnh sửa sự kiện.
     * @param item Con trỏ tới EventItem đã được click.
     */
    void eventClicked(EventItem *item);

protected:
    // === 3. HÀM SỰ KIỆN (PROTECTED EVENT HANDLERS) ===
    // (Các hàm override từ QFrame/QWidget)

    /**
     * @brief Bắt sự kiện click chuột (thả chuột) lên *vùng trống* của ô.
     */
    void mouseReleaseEvent(QMouseEvent *event) override;

    /**
     * @brief Bắt sự kiện thay đổi kích thước ô.
     * Dùng để kích hoạt logic responsive (thu gọn).
     */
    void resizeEvent(QResizeEvent *event) override;

    /**
     * @brief Bắt sự kiện click chuột lên các *QLabel con* (các nhãn sự kiện).
     */
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    // === 4. HÀM VÀ BIẾN NỘI BỘ (PRIVATE) ===

    /**
     * @brief Logic responsive: Quyết định hiển thị danh sách hay dấu chấm.
     * @param newSize Kích thước mới của ô.
     */
    void updateEventDisplay(const QSize &newSize);

    // --- 4a. Con trỏ UI ---
    QLabel *m_dateLabel;            // Nhãn hiển thị số ngày (ví dụ: "15")
    QWidget *m_eventsContainer;     // Widget chứa layout các sự kiện
    QVBoxLayout *m_eventsLayout;    // Layout sắp xếp các nhãn sự kiện theo chiều dọc
    QLabel *m_compactEventsIndicator; // Dấu chấm '●' (indicator) khi thu gọn

    // --- 4b. Biến Trạng thái (State) ---
    QDate m_date;                       // Ngày mà ô này đại diện
    QList<EventItem*> m_events;         // Danh sách "dữ liệu" sự kiện của ô này

    /**
     * @brief Ngưỡng (bằng pixel) để chuyển sang chế độ thu gọn.
     * Nếu chiều cao ô < 75px, chế độ thu gọn sẽ được kích hoạt.
     */
    const int m_compactThreshold = 75;
};

#endif // DAYCELLWIDGET_H
