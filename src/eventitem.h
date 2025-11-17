#ifndef EVENTITEM_H
#define EVENTITEM_H

#include <QGraphicsRectItem> // Lớp cơ sở (để vẽ)
#include <QObject>           // Lớp cơ sở (để có signals/slots)
#include <QString>
#include <QColor>
#include <QDateTime>
#include <QGraphicsSceneHoverEvent> // Cần cho các sự kiện hover
#include <QJsonObject>              // Cần cho m_extraData
#include "eventdialog.h" // Cần để sử dụng EventDialog::RecurrenceRule

/**
 * @brief Lớp EventItem đại diện cho MỘT sự kiện đồ họa trên QGraphicsScene.
 *
 * Lớp này kế thừa từ QObject (để dùng signals/slots) và
 * QGraphicsRectItem (để vẽ và tương tác). Nó chứa toàn bộ dữ liệu
 * của một sự kiện và xử lý logic vẽ, click, kéo-thả (drag),
 * và thay đổi kích thước (resize).
 */
class EventItem : public QObject, public QGraphicsRectItem
{
    // Q_OBJECT là bắt buộc để sử dụng signals/slots
    Q_OBJECT

public:
    // === 1. HÀM DỰNG (CONSTRUCTOR) ===

    /**
     * @brief Hàm dựng chính, nhận tất cả dữ liệu của một sự kiện.
     * @param startTime Thời gian bắt đầu (LUÔN LÀ UTC).
     * @param endTime Thời gian kết thúc (LUÔN LÀ UTC).
     * @param eventType Loại sự kiện ("Sự kiện", "Cuộc họp", v.v.).
     * @param extraData Dữ liệu JSON chứa thông tin bổ sung (ví dụ: người tham gia).
     */
    EventItem(const QString &title, const QColor &color,
              const QDateTime &startTime, const QDateTime &endTime,
              const QString &description, const QString &showAs,
              const QString &category, bool isAllDay,
              const EventDialog::RecurrenceRule &rule,
              const QString &eventType,
              const QJsonObject &extraData,
              QGraphicsItem *parent = nullptr);

    // === 2. API CÔNG KHAI (PUBLIC API) ===

    /**
     * @brief Tính toán và cập nhật vị trí (X, Y) và kích thước (W, H) của item.
     * @param displayOffsetSeconds Múi giờ hiển thị (từ MainWindow)
     * để chuyển đổi startTime/endTime (UTC) sang giờ địa phương.
     */
    void updateGeometry(double dayWidth, double hourHeight, int dayIndex, int col, int totalCols, int displayOffsetSeconds);

    // --- Getters (Hàm lấy thông tin) ---
    QDateTime startTime() const { return m_startTime; } // Trả về UTC
    QDateTime endTime() const { return m_endTime; }     // Trả về UTC
    QString title() const;
    QColor color() const;
    QString description() const { return m_description; }
    QString showAsStatus() const { return m_showAs; }
    QString category() const { return m_category; }
    bool isAllDay() const { return m_isAllDay; }
    EventDialog::RecurrenceRule recurrenceRule() const { return m_rule; }
    QString eventType() const { return m_eventType; }
    QJsonObject extraData() const { return m_extraData; }
    bool isFilteredOut() const { return m_isFilteredOut; }

    // --- Setters (Hàm đặt thông tin) ---
    void setStartTime(const QDateTime &startTime); // (startTime phải là UTC)
    void setEndTime(const QDateTime &endTime);     // (endTime phải là UTC)
    /**
     * @brief Đặt cờ lọc (ẩn/hiện).
     * @param filtered 'true' nếu item nên bị ẩn, 'false' nếu nên hiển thị.
     */
    void setFiltered(bool filtered) { m_isFilteredOut = filtered; }

signals:
    // === 3. TÍN HIỆU (SIGNALS) ===
    // (Phát ra để báo cáo cho CalendarView / MainWindow)

    /**
     * @brief Phát ra khi một thay đổi (ví dụ: resize) đã hoàn tất.
     */
    void eventChanged(EventItem *item);

    /**
     * @brief Phát ra khi item được click (nhấn và thả mà không di chuyển).
     */
    void clicked(EventItem *item);

    /**
     * @brief Phát ra khi item được KÉO-THẢ (drag-and-drop) thành công.
     * @param newStartTime Thời gian bắt đầu (Local) MỚI (đã bắt dính).
     * @param newEndTime Thời gian kết thúc (Local) MỚI.
     */
    void eventDragged(EventItem *item, const QDateTime &newStartTime, const QDateTime &newEndTime);

protected:
    // === 4. HÀM SỰ KIỆN (PROTECTED EVENT HANDLERS) ===
    // (Các hàm override từ QGraphicsRectItem)

    /**
     * @brief Vẽ item (nền, viền, tiêu đề, thanh trạng thái).
     */
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    /**
     * @brief Được gọi ngay trước khi một thuộc tính (như vị trí) thay đổi.
     */
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

    /**
     * @brief Bắt sự kiện di chuột *bên trên* item (để đổi con trỏ chuột).
     */
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;

    /**
     * @brief Bắt sự kiện chuột *rời khỏi* item (để reset con trỏ chuột).
     */
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

    /**
     * @brief Bắt sự kiện *nhấn* chuột (để bắt đầu resize hoặc drag).
     */
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

    /**
     * @brief Bắt sự kiện *kéo* chuột (để xử lý logic resize hoặc drag).
     */
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

    /**
     * @brief Bắt sự kiện *thả* chuột (để hoàn tất resize, drag, hoặc xử lý click).
     */
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    // === 5. HÀM NỘI BỘ (PRIVATE HELPERS) ===

    /**
     * @brief Hàm trợ giúp để tính toán và cập nhật vị trí "bắt dính" (snapped)
     * cho m_ghostItem khi đang kéo.
     */
    void updateGhostPosition(QPointF newScenePos);

    // === 6. BIẾN THÀNH VIÊN (MEMBER VARIABLES) ===

    // --- 6a. Dữ liệu Sự kiện (Event Data) ---
    // (Đây là "Nguồn sự thật" (Source of Truth) của sự kiện này)
    QString m_title;
    QColor m_color;
    QDateTime m_startTime; // LƯU Ý: Luôn là UTC
    QDateTime m_endTime;   // LƯU Ý: Luôn là UTC
    QString m_description;
    QString m_showAs;
    QString m_category;
    bool m_isAllDay;
    EventDialog::RecurrenceRule m_rule;
    QString m_eventType;
    QJsonObject m_extraData; // Dữ liệu JSON (ví dụ: host, participants...)

    // --- 6b. Trạng thái Tương tác (Interaction State) ---
    bool m_isResizing;      // Cờ báo hiệu đang resize
    bool m_isMoving;        // Cờ báo hiệu đang kéo (drag)
    bool m_isFilteredOut;   // Cờ báo hiệu đang bị bộ lọc ẩn đi

    QPointF m_pressPos;     // Vị trí nhấn chuột (trên scene)
    QPointF m_dragStartOffset; // Vị trí nhấn chuột (so với góc item)

    QGraphicsRectItem *m_ghostItem; // Con trỏ tới item "ma" khi kéo
};

#endif // EVENTITEM_H
