#ifndef CALENDARVIEW_H
#define CALENDARVIEW_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QDate>
#include <QDateTime>

class EventItem;
class QTimer;

class CalendarView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit CalendarView(QWidget *parent = nullptr);

    // THAY ĐỔI: Chuyển sang nhận EventItem*
    void addEvent(EventItem *item);

    // MỚI: Hàm để xóa
    void removeEvent(EventItem *item);

    double getDayWidth() const;
    double getHourHeight() const { return m_hourHeight; }
    QDate getMondayOfCurrentWeek() const { return m_currentMonday; }
    int getNumberOfDays() const { return m_days; }

public slots:
    void relayoutEventsForDate(const QDate &date, int dayIndex);
    void performInitialLayout();
    void updateViewForDateRange(const QDate &monday);
    void setNumberOfDays(int days);
    void setTimeScale(int minutes);
    void onInternalEventDragged(EventItem *item, const QDateTime &newStartTime, const QDateTime &newEndTime);
    void setTimezoneOffset(int offsetSeconds);

signals:
    void eventClicked(EventItem *item); // <-- MỚI: Tín hiệu báo cho MainWindow
    void eventDragged(EventItem *item, const QDateTime &newStartTime, const QDateTime &newEndTime);

protected:
    void drawBackground(QPainter *painter, const QRectF &rect) override;
    void resizeEvent(QResizeEvent *event) override;
    void drawForeground(QPainter *painter, const QRectF &rect) override;

private:
    void updateSceneRect();
    QGraphicsScene *m_scene;
    int m_days;
    double m_hourHeight;
    QDate m_currentMonday;
    QTimer *m_timer;
    int m_timezoneOffsetSeconds;
};

#endif // CALENDARVIEW_H
