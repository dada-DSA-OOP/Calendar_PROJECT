#ifndef DAYHEADER_H
#define DAYHEADER_H

#include <QWidget> // Lớp cơ sở (bắt buộc)
#include <QDate>   // Cần cho kiểu dữ liệu QDate

/**
 * @brief Lớp DayHeader là widget thanh tiêu đề ngang.
 *
 * Lớp này hiển thị các ngày trong tuần (ví dụ: "THỨ 2 / 17")
 * và cuộn đồng bộ theo chiều ngang với CalendarView.
 */
class DayHeader : public QWidget
{
    // Q_OBJECT là bắt buộc để sử dụng signals và public slots
    Q_OBJECT

public:
    // === 1. HÀM DỰNG (CONSTRUCTOR) ===
    explicit DayHeader(QWidget *parent = nullptr);

public slots:
    // === 2. CÁC SLOT CÔNG KHAI (PUBLIC SLOTS) ===
    // (Các hàm "ra lệnh" từ MainWindow)

    /**
     * @brief Được gọi bởi CalendarView để đồng bộ cuộn ngang.
     * @param x Vị trí cuộn (pixel) mới.
     */
    void setScrollOffset(int x);

    /**
     * @brief Cập nhật ngày bắt đầu (thường là Thứ Hai) để vẽ.
     * @param monday Ngày bắt đầu của phạm vi hiển thị.
     */
    void updateDates(const QDate& monday);

    /**
     * @brief Đặt số lượng ngày sẽ hiển thị trên header.
     * @param days Số ngày (ví dụ: 1, 3, 5, 7).
     */
    void setNumberOfDays(int days);

    /**
     * @brief Đặt lề phải (right margin) cho header.
     * Dùng để căn chỉnh header với CalendarView khi thanh cuộn dọc
     * của CalendarView xuất hiện (chiếm chỗ).
     * @param margin Chiều rộng (pixel) của lề.
     */
    void setRightMargin(int margin);

protected:
    // === 3. HÀM SỰ KIỆN (PROTECTED EVENT HANDLERS) ===
    // (Các hàm override từ QWidget, được Qt gọi tự động)

    /**
     * @brief Hàm vẽ chính, chịu trách nhiệm vẽ các tiêu đề ngày,
     * đường kẻ, và làm nổi bật ngày hôm nay.
     */
    void paintEvent(QPaintEvent *event) override;

    /**
     * @brief Được gọi khi kích thước widget thay đổi.
     * Dùng để tính toán lại m_dayWidth (chiều rộng của một cột ngày).
     */
    void resizeEvent(QResizeEvent *event) override;

private:
    // === 4. BIẾN THÀNH VIÊN (MEMBER VARIABLES) ===

    int m_scrollOffset;    // Vị trí cuộn ngang hiện tại (tính bằng pixel).
    double m_dayWidth;     // Chiều rộng (pixel) của MỘT cột ngày.
    int m_days;            // Số lượng ngày đang hiển thị (1, 3, 5, 7).
    int m_rightMargin;     // Kích thước lề phải (để trừ đi thanh cuộn).

    QDate m_monday;        // Ngày Thứ Hai (dùng để khởi tạo, có thể không cần thiết).
    QDate m_currentMonday; // Ngày bắt đầu (anchor) của phạm vi đang được vẽ.
};

#endif // DAYHEADER_H
