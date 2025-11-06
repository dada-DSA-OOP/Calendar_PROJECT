#ifndef CALENDARVIEW_H
#define CALENDARVIEW_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QDate>
#include <QDateTime> // <-- THAY ĐỔI: Dùng QDateTime

class EventItem;
class QTimer; // <-- Thêm forward declaration cho QTimer

class CalendarView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit CalendarView(QWidget *parent = nullptr);
    // THAY ĐỔI: Sửa lại hàm addEvent
    void addEvent(const QString &title, const QColor &color, const QDateTime &startTime, const QDateTime &endTime);

    double getDayWidth() const;
    double getHourHeight() const { return m_hourHeight; }

    // THAY ĐỔI: Thêm hàm getter này để EventItem có thể truy cập
    QDate getMondayOfCurrentWeek() const { return m_currentMonday; }

    // --- THÊM HÀM NÀY VÀO ĐÂY ---
    int getNumberOfDays() const { return m_days; }

public slots:
    // THAY ĐỔI: Đổi tên và logic của hàm relayout
    void relayoutEventsForDate(const QDate &date, int dayIndex);
    void performInitialLayout();

    // THAY ĐỔI: Thêm slot để nhận biết tuần thay đổi
    void updateViewForDateRange(const QDate &monday);

    void setNumberOfDays(int days);

    void setTimeScale(int minutes); // Ví dụ: 30 (phút)

signals:

protected:
    void drawBackground(QPainter *painter, const QRectF &rect) override;
    void resizeEvent(QResizeEvent *event) override;
    void drawForeground(QPainter *painter, const QRectF &rect) override;

private:
    void updateSceneRect();

    QGraphicsScene *m_scene;
    int m_days;
    double m_hourHeight;
    QDate m_currentMonday; // <-- THAY ĐỔI: Thêm biến lưu ngày thứ Hai của tuần
    QTimer *m_timer; // <-- THÊM BIẾN TIMER
};

#endif // CALENDARVIEW_H
