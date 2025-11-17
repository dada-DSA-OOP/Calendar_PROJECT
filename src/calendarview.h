#ifndef CALENDARVIEW_H
#define CALENDARVIEW_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QDate>
#include <QDateTime>

class EventItem; // Forward declaration cho EventItem
class QTimer;    // Forward declaration cho QTimer

/**
 * @brief Lớp CalendarView chịu trách nhiệm hiển thị lịch dạng timeline (dòng thời gian).
 *
 * Lớp này kế thừa QGraphicsView và sử dụng QGraphicsScene để quản lý
 * và hiển thị các đối tượng EventItem (vốn là QGraphicsItem).
 * Nó xử lý việc vẽ lưới, sắp xếp các sự kiện chồng chéo,
 * và vẽ đường kẻ thời gian hiện tại.
 */
class CalendarView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit CalendarView(QWidget *parent = nullptr);

    // === 1. API CÔNG KHAI (PUBLIC API) ===
    // (Các hàm "ra lệnh" từ MainWindow)

    /**
     * @brief Thêm một EventItem* (đã được tạo) vào Scene.
     */
    void addEvent(EventItem *item);

    /**
     * @brief Xóa một EventItem* khỏi Scene.
     * @note Hàm này không 'delete' con trỏ, chỉ gỡ bỏ khỏi Scene.
     */
    void removeEvent(EventItem *item);

    /**
     * @brief Lấy chiều rộng (pixels) hiện tại của một cột ngày.
     */
    double getDayWidth() const;

    /**
     * @brief Lấy chiều cao (pixels) hiện tại của 1 giờ.
     */
    double getHourHeight() const { return m_hourHeight; }

    /**
     * @brief Lấy ngày Thứ Hai của tuần đang hiển thị.
     */
    QDate getMondayOfCurrentWeek() const { return m_currentMonday; }

    /**
     * @brief Lấy số ngày đang được hiển thị (1, 3, 5, hoặc 7).
     */
    int getNumberOfDays() const { return m_days; }

public slots:
    // === 2. CÁC SLOT CÔNG KHAI (PUBLIC SLOTS) ===
    // (Các hàm "phản ứng" với MainWindow)

    /**
     * @brief Cập nhật toàn bộ hiển thị cho một tuần/phạm vi ngày mới.
     * Đây là hàm cập nhật chính, được gọi khi đổi tuần, đổi múi giờ, hoặc lọc.
     * @param monday Ngày bắt đầu (thường là Thứ Hai) của phạm vi mới.
     */
    void updateViewForDateRange(const QDate &monday);

    /**
     * @brief Đặt số lượng ngày sẽ hiển thị trên view.
     * @param days Số ngày (ví dụ: 1, 3, 5, 7).
     */
    void setNumberOfDays(int days);

    /**
     * @brief Đặt tỉ lệ "zoom" dọc của lịch.
     * @param minutes Số phút tương ứng với 60px chiều cao (ví dụ: 60, 30, 15).
     */
    void setTimeScale(int minutes);

    /**
     * @brief Đặt chênh lệch múi giờ (so với UTC) để hiển thị thời gian chính xác.
     * @param offsetSeconds Số giây chênh lệch.
     */
    void setTimezoneOffset(int offsetSeconds);

    /**
     * @brief (Không còn dùng) Sắp xếp layout ban đầu.
     * @deprecated Được thay thế bởi updateViewForDateRange.
     */
    void performInitialLayout();

signals:
    // === 3. TÍN HIỆU (SIGNALS) ===
    // (Các tín hiệu "báo cáo" lên MainWindow)

    /**
     * @brief Phát ra khi một EventItem được click.
     * @param item Con trỏ tới EventItem đã được click.
     */
    void eventClicked(EventItem *item);

    /**
     * @brief Phát ra sau khi người dùng kéo (drag) một sự kiện đến vị trí mới.
     * @param item Con trỏ tới EventItem đã bị kéo.
     * @param newStartTime Thời gian bắt đầu (Local) mới.
     * @param newEndTime Thời gian kết thúc (Local) mới.
     */
    void eventDragged(EventItem *item, const QDateTime &newStartTime, const QDateTime &newEndTime);

protected:
    // === 4. HÀM OVERRIDE (PROTECTED EVENT HANDLERS) ===
    // (Được Qt gọi tự động)

    /**
     * @brief Vẽ nền (background) của QGraphicsView (vẽ *trước* các sự kiện).
     * Được dùng để vẽ các đường kẻ lưới (grid lines).
     */
    void drawBackground(QPainter *painter, const QRectF &rect) override;

    /**
     * @brief Vẽ tiền cảnh (foreground) của QGraphicsView (vẽ *sau* các sự kiện).
     * Được dùng để vẽ "đường kẻ thời gian hiện tại".
     */
    void drawForeground(QPainter *painter, const QRectF &rect) override;

    /**
     * @brief Được gọi khi kích thước của widget thay đổi.
     * Dùng để cập nhật lại sceneRect và tính toán lại layout.
     */
    void resizeEvent(QResizeEvent *event) override;

private slots:
    // === 5. CÁC SLOT NỘI BỘ (PRIVATE SLOTS) ===

    /**
     * @brief Sắp xếp lại các sự kiện chồng chéo cho một ngày cụ thể.
     * @param date Ngày cần sắp xếp.
     * @param dayIndex Chỉ số cột của ngày đó (0 = cột đầu tiên).
     */
    void relayoutEventsForDate(const QDate &date, int dayIndex);

    /**
     * @brief Nhận tín hiệu kéo (drag) nội bộ từ EventItem.
     * Mục đích là để "phát lại" (re-emit) tín hiệu này ra bên ngoài cho MainWindow.
     */
    void onInternalEventDragged(EventItem *item, const QDateTime &newStartTime, const QDateTime &newEndTime);

private:
    // === 6. HÀM TRỢ GIÚP (PRIVATE HELPERS) ===

    /**
     * @brief Cập nhật kích thước (chiều rộng, chiều cao) của QGraphicsScene
     * dựa trên kích thước viewport và m_hourHeight.
     */
    void updateSceneRect();

    // === 7. BIẾN THÀNH VIÊN (MEMBER VARIABLES) ===

    QGraphicsScene *m_scene; // Tấm "canvas" ảo chứa tất cả EventItem.
    QTimer *m_timer;         // Timer để cập nhật đường kẻ thời gian hiện tại (mỗi phút).

    int m_days;              // Số ngày đang hiển thị (1, 3, 5, 7).
    double m_hourHeight;     // Chiều cao (pixels) của 1 giờ.
    QDate m_currentMonday;   // Ngày bắt đầu của phạm vi đang hiển thị.
    int m_timezoneOffsetSeconds; // Chênh lệch múi giờ (so với UTC).
};

#endif // CALENDARVIEW_H
