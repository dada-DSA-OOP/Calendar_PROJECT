#ifndef MONTHVIEWWIDGET_H
#define MONTHVIEWWIDGET_H

#include <QWidget>
#include <QDate>
#include <QMap>

class DayCellWidget;
class EventItem;
class QGridLayout;

class MonthViewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MonthViewWidget(QWidget *parent = nullptr);

    void addEvent(EventItem *event);
    void updateView(const QDate &date);

    // MỚI: Hàm để xóa
    void removeEvent(EventItem *event);

public slots:
    void setTimezoneOffset(int offsetSeconds);

signals:
    // MỚI: Tín hiệu báo cho MainWindow
    void eventClicked(EventItem *item);

private slots:
    void onCellClicked(const QDate &date, const QList<EventItem*> &events);

private:
    void clearGrid();
    QGridLayout *m_gridLayout;
    QList<DayCellWidget*> m_dayCells;
    QMap<QDate, QList<EventItem*>> m_allEvents;
    int m_timezoneOffsetSeconds;
};

#endif // MONTHVIEWWIDGET_H
