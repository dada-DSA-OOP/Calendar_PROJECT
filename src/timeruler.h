#ifndef TIMERULER_H
#define TIMERULER_H

#include <QWidget> // Lớp cơ sở (bắt buộc)
#include <QTime>   // Cần cho QTime
#include <QDateTime> // (Thêm) Cần cho QDateTime

/**
 * @brief Lớp TimeRuler là widget "thước đo thời gian" dọc.
 *
 * Lớp này hiển thị các mốc thời gian (ví dụ: 09:00, 10:00)
 * ở bên trái của CalendarView. Nó cuộn đồng bộ theo chiều dọc
 * với CalendarView và có thể thay đổi "zoom" (m_hourHeight)
 * và định dạng (12/24h).
 */
class TimeRuler : public QWidget
{
    // Q_OBJECT là bắt buộc để sử dụng signals và slots
    Q_OBJECT

public:
    // === 1. HÀM DỰNG & API CÔNG KHAI ===
    // (Các hàm mà MainWindow/CalendarView sẽ gọi)

    explicit TimeRuler(QWidget *parent = nullptr);

    /**
     * @brief Đặt chiều cao (pixel) của 1 giờ.
     * Được gọi khi CalendarView thay đổi "Tỉ lệ thời gian" (zoom).
     */
    void setHourHeight(double height);

    /**
     * @brief Đặt định dạng thời gian 12h (AM/PM) hoặc 24h.
     * Được gọi từ SettingsDialog (thông qua MainWindow).
     */
    void set24HourFormat(bool is24Hour);

    /**
     * @brief Đặt chênh lệch múi giờ (so với UTC) để hiển thị nhãn UTC.
     * @param offsetSeconds Số giây chênh lệch.
     */
    void setTimezoneOffset(int offsetSeconds);

public slots:
    // === 2. CÁC SLOT CÔNG KHAI (PUBLIC SLOTS) ===
    // (Các hàm "phản ứng" với các widget khác)

    /**
     * @brief Được kết nối với thanh cuộn dọc (verticalScrollBar) của CalendarView.
     * @param y Vị trí cuộn (pixel) mới.
     */
    void setScrollOffset(int y);

protected:
    // === 3. HÀM SỰ KIỆN (PROTECTED EVENT HANDLERS) ===
    // (Các hàm override từ QWidget, được Qt gọi tự động)

    /**
     * @brief Hàm vẽ chính, chịu trách nhiệm vẽ các mốc thời gian,
     * đường kẻ, và nhãn múi giờ.
     */
    void paintEvent(QPaintEvent *event) override;

private:
    // === 4. BIẾN THÀNH VIÊN (MEMBER VARIABLES) ===

    int m_scrollOffset;    // Vị trí cuộn dọc hiện tại (tính bằng pixel).
    double m_hourHeight;     // Chiều cao (pixel) của MỘT giờ.
    bool m_use24HourFormat;  // Cờ 'true' = 24h, 'false' = 12h (AM/PM).
    int m_timezoneOffsetSeconds; // Chênh lệch múi giờ (so với UTC).
};

#endif // TIMERULER_H
