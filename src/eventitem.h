#ifndef EVENTITEM_H
#define EVENTITEM_H

#include <QGraphicsRectItem>
#include <QString>
#include <QColor>
#include <QDateTime> // <-- THAY ĐỔI: Dùng QDateTime thay cho QTime
#include <QGraphicsSceneHoverEvent>
#include <QObject>

class EventItem : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
public:
    // THAY ĐỔI: Sửa lại hàm khởi tạo để nhận QDateTime
    EventItem(const QString &title, const QColor &color,
              const QDateTime &startTime, const QDateTime &endTime, QGraphicsItem *parent = nullptr);

    void updateGeometry(double dayWidth, double hourHeight, int subColumn = 0, int totalSubColumns = 1);

    // THAY ĐỔI: Sửa lại các hàm getter
    QDateTime startTime() const { return m_startTime; }
    QDateTime endTime() const { return m_endTime; }

    // THAY ĐỔI: Thêm hàm setter để CalendarView có thể cập nhật
    void setStartTime(const QDateTime &startTime);
    void setEndTime(const QDateTime &endTime);


signals:
    void eventChanged(EventItem *item);

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    QString m_title;
    QColor m_color;
    // THAY ĐỔI: Bỏ m_day, thay bằng QDateTime
    QDateTime m_startTime;
    QDateTime m_endTime;
    bool m_isResizing;
    QGraphicsRectItem *m_ghostItem; // Vật phẩm mờ khi kéo
    QPointF m_dragStartOffset;      // Vị trí nhấn chuột so với góc item
    void updateGhostPosition(QPointF newScenePos);
};

#endif // EVENTITEM_H
