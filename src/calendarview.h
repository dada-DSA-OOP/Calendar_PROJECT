#ifndef CALENDARVIEW_H
#define CALENDARVIEW_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QDate>
#include <QTime>

class EventItem;

class CalendarView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit CalendarView(QWidget *parent = nullptr);
    void addEvent(const QString &title, const QColor &color, int day, const QTime &startTime, const QTime &endTime);
    double getDayWidth() const;
    double getHourHeight() const { return m_hourHeight; } // <-- Thêm dòng này

public slots: // <-- Thêm section này
    void relayoutDay(EventItem *changedItem);
    void performInitialLayout(); // <-- Thêm khai báo hàm này

signals: // <-- Thêm section này
    void viewResized(); // <-- Thêm tín hiệu này

protected:
    void drawBackground(QPainter *painter, const QRectF &rect) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void updateSceneRect();

    QGraphicsScene *m_scene;
    int m_days;          // Số ngày hiển thị (ví dụ: 7)
    double m_hourHeight; // Chiều cao của một giờ
    bool m_initialLayoutDone; // <-- Thêm biến cờ này
};

#endif // CALENDARVIEW_H
