#ifndef EVENTITEM_H
#define EVENTITEM_H

#include <QGraphicsRectItem>
#include <QString>
#include <QColor>
#include <QDateTime>
#include <QGraphicsSceneHoverEvent>
#include <QObject>
#include <QJsonObject>
#include "eventdialog.h" // <-- MỚI: Để sử dụng RecurrenceRule

class EventItem : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
public:
    // THAY ĐỔI: Hàm khởi tạo giờ sẽ nhận TẤT CẢ dữ liệu
    EventItem(const QString &title, const QColor &color,
              const QDateTime &startTime, const QDateTime &endTime,
              const QString &description, const QString &showAs,
              const QString &category, bool isAllDay,
              const EventDialog::RecurrenceRule &rule,
              const QString &eventType,
              const QJsonObject &extraData,
              QGraphicsItem *parent = nullptr);

    void updateGeometry(double dayWidth, double hourHeight, int dayIndex, int col, int totalCols, int displayOffsetSeconds);

    // Các hàm getter cũ
    QDateTime startTime() const { return m_startTime; }
    QDateTime endTime() const { return m_endTime; }
    QString title() const;
    QColor color() const;

    // MỚI: Các hàm getter cho dữ liệu mới
    QString description() const { return m_description; }
    QString showAsStatus() const { return m_showAs; }
    QString category() const { return m_category; }
    bool isAllDay() const { return m_isAllDay; }
    EventDialog::RecurrenceRule recurrenceRule() const { return m_rule; }

    // Các hàm setter cũ
    void setStartTime(const QDateTime &startTime);
    void setEndTime(const QDateTime &endTime);

    void setFiltered(bool filtered) { m_isFilteredOut = filtered; }
    bool isFilteredOut() const { return m_isFilteredOut; }

    QString eventType() const { return m_eventType; }
    QJsonObject extraData() const { return m_extraData; }


signals:
    void eventChanged(EventItem *item);
    void clicked(EventItem *item); // <-- MỚI: Tín hiệu khi item được nhấn
    // MỚI: Tín hiệu báo đã kéo thả xong
    void eventDragged(EventItem *item, const QDateTime &newStartTime, const QDateTime &newEndTime);

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    // Dữ liệu cũ
    QString m_title;
    QColor m_color;
    QDateTime m_startTime;
    QDateTime m_endTime;

    // Dữ liệu kéo/thả/resize
    bool m_isResizing;
    QGraphicsRectItem *m_ghostItem;
    QPointF m_dragStartOffset;

    // MỚI: Dữ liệu đầy đủ của sự kiện
    QString m_description;
    QString m_showAs;
    QString m_category;
    bool m_isAllDay;
    EventDialog::RecurrenceRule m_rule;

    // MỚI: Biến để phân biệt click và drag
    QPointF m_pressPos; // Vị trí nhấn chuột (trên scene)
    bool m_isMoving; // Cờ báo hiệu đang di chuyển

    bool m_isFilteredOut;

    void updateGhostPosition(QPointF newScenePos);

    QString m_eventType;
    QJsonObject m_extraData;
};

#endif // EVENTITEM_H
