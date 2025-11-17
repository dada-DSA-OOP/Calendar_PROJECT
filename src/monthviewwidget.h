#ifndef MONTHVIEWWIDGET_H
#define MONTHVIEWWIDGET_H

#include <QWidget> // Lớp cơ sở (bắt buộc)
#include <QDate>   // Cần cho QDate và QMap
#include <QMap>    // Cần cho m_allEvents
#include <QList>   // (Thêm) Cần cho QList

// --- Forward Declarations (Khai báo trước) ---
// (Khai báo tên lớp để tránh include .h đầy đủ, tăng tốc độ biên dịch)
class DayCellWidget;
class EventItem;
class QGridLayout;

/**
 * @brief Lớp MonthViewWidget là widget chính cho "Chế độ xem Tháng".
 *
 * Lớp này quản lý một lưới (Grid) 6x7 gồm 42 đối tượng DayCellWidget.
 * Nó chịu trách nhiệm điền ngày và sự kiện vào các ô này,
 * và xử lý việc thay đổi múi giờ hoặc bộ lọc.
 */
class MonthViewWidget : public QWidget
{
    // Q_OBJECT là bắt buộc để sử dụng signals và slots
    Q_OBJECT

public:
    // === 1. HÀM DỰNG & API CÔNG KHAI ===
    // (Các hàm mà MainWindow sẽ gọi)

    explicit MonthViewWidget(QWidget *parent = nullptr);

    /**
     * @brief Thêm một sự kiện vào Map dữ liệu (m_allEvents) của view này.
     * (Không cập nhật UI ngay lập tức).
     */
    void addEvent(EventItem *event);

    /**
     * @brief Xóa một sự kiện khỏi Map dữ liệu (m_allEvents) VÀ
     * yêu cầu DayCellWidget tương ứng xóa nó khỏi UI.
     */
    void removeEvent(EventItem *event);

    /**
     * @brief Hàm cập nhật chính: Vẽ lại toàn bộ lưới 42 ô
     * dựa trên một ngày trong tháng.
     * @param date Một ngày bất kỳ trong tháng cần hiển thị.
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
     * @brief Phát ra khi người dùng click vào một *sự kiện cụ thể* (QLabel)
     * bên trong một DayCellWidget.
     * (Tín hiệu này được "chuyển tiếp" từ DayCellWidget).
     * @param item Con trỏ tới EventItem đã được click.
     */
    void eventClicked(EventItem *item);

private slots:
    // === 4. SLOT NỘI BỘ (PRIVATE SLOTS) ===
    // (Phản ứng với các DayCellWidget con)

    /**
     * @brief Được gọi khi người dùng click vào *vùng trống* của một DayCellWidget.
     * Mở dialog "chi tiết" (DayDetailDialog) cho ngày đó.
     */
    void onCellClicked(const QDate &date, const QList<EventItem*> &events);

private:
    // === 5. HÀM VÀ BIẾN NỘI BỘ (PRIVATE) ===

    /**
     * @brief Hàm dọn dẹp: Yêu cầu tất cả 42 ô DayCellWidget xóa UI sự kiện.
     */
    void clearGrid();

    // --- 5a. Con trỏ UI ---
    QGridLayout *m_gridLayout; // Layout lưới 6x7

    /**
     * @brief Danh sách chứa con trỏ tới 42 ô DayCellWidget
     * (để dễ dàng truy cập và lặp qua).
     */
    QList<DayCellWidget*> m_dayCells;

    // --- 5b. Biến Dữ liệu & Trạng thái ---
    /**
     * @brief "Nguồn sự thật" (Source of Truth) của riêng MonthViewWidget.
     * Ánh xạ một Ngày (QDate) tới Danh sách các sự kiện (EventItem*)
     * thuộc về ngày đó (theo múi giờ hiển thị).
     */
    QMap<QDate, QList<EventItem*>> m_allEvents;

    /**
     * @brief Múi giờ hiển thị hiện tại (tính bằng giây so với UTC).
     */
    int m_timezoneOffsetSeconds;
};

#endif // MONTHVIEWWIDGET_H
