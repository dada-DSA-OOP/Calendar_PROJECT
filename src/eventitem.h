#ifndef EVENTITEM_H
#define EVENTITEM_H

#include <QGraphicsRectItem>
#include <QString>
#include <QColor>
#include <QTime>
#include <QGraphicsSceneHoverEvent>
#include <QObject>

class EventItem : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
public:
    // Sửa lại hàm khởi tạo để nhận QObject parent
    EventItem(const QString &title, const QColor &color, int day,
              const QTime &startTime, const QTime &endTime, QGraphicsItem *parent = nullptr);

    // Sửa lại hàm updateGeometry để nhận thông tin cột phụ
    void updateGeometry(double dayWidth, double hourHeight, int subColumn = 0, int totalSubColumns = 1);
    // Thêm các hàm getter để thuật toán truy cập dữ liệu
    int day() const { return m_day; }
    QTime startTime() const { return m_startTime; }
    QTime endTime() const { return m_endTime; }
signals: // <-- Thêm section này
    void eventChanged(EventItem *item);

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    // Thêm khai báo override cho itemChange (Bắt ô sự kiện phải theo lưới, không được tự do)
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    // Thêm khai báo override cho mouseReleaseEvent (Kéo ô sự kiện thì sẽ cập nhật cả giá trị)
    // Thêm các override cho hover và mouse move
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    QString m_title;
    QColor m_color;
    int m_day;
    QTime m_startTime;
    QTime m_endTime;
    bool m_isResizing; // <-- Thêm biến trạng thái này
};

#endif // EVENTITEM_H
