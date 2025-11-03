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

    // Hàm để MainWindow gọi khi thêm sự kiện
    void addEvent(EventItem *event);

    // Hàm để MainWindow gọi khi chuyển tuần/tháng
    void updateView(const QDate &date);

private slots:
    void onCellClicked(const QDate &date, const QList<EventItem*> &events);

private:
    void clearGrid();
    QGridLayout *m_gridLayout;
    QList<DayCellWidget*> m_dayCells; // Danh sách 42 ô

    // Lưu trữ tất cả sự kiện để truy xuất nhanh
    QMap<QDate, QList<EventItem*>> m_allEvents;
};

#endif // MONTHVIEWWIDGET_H
