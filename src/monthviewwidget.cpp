#include "monthviewwidget.h"
#include "daycellwidget.h"
#include "daydetaildialog.h"
#include "eventitem.h"
#include <QGridLayout>

MonthViewWidget::MonthViewWidget(QWidget *parent)
    : QWidget(parent)
{
    m_gridLayout = new QGridLayout(this);
    m_gridLayout->setContentsMargins(0, 0, 0, 0);
    m_gridLayout->setSpacing(0);

    for (int i = 0; i < 42; ++i) { // 6 tuần * 7 ngày
        DayCellWidget *cell = new DayCellWidget;
        m_dayCells.append(cell);
        m_gridLayout->addWidget(cell, i / 7, i % 7);

        // Kết nối tín hiệu click
        connect(cell, &DayCellWidget::cellClicked, this, &MonthViewWidget::onCellClicked);
    }

    setLayout(m_gridLayout);
}

void MonthViewWidget::addEvent(EventItem *event)
{
    // Lưu sự kiện vào map
    m_allEvents[event->startTime().date()].append(event);

    // TODO: Cần có cơ chế cập nhật lại grid nếu tháng hiện tại thay đổi
    // Tạm thời, chúng ta giả định sự kiện được thêm trước khi 'updateView'
}

void MonthViewWidget::updateView(const QDate &date)
{
    clearGrid();

    // Tìm ngày đầu tiên của tháng
    QDate firstDayOfMonth = date.addDays(-(date.day() - 1));
    // Tìm ngày đầu tiên để bắt đầu vẽ (có thể là T2 của tuần trước)
    int daysToMonday = firstDayOfMonth.dayOfWeek() - 1;
    QDate startDate = firstDayOfMonth.addDays(-daysToMonday);

    // Lấp đầy 42 ô
    for (int i = 0; i < 42; ++i) {
        QDate currentDate = startDate.addDays(i);
        DayCellWidget *cell = m_dayCells[i];

        bool isCurrentMonth = (currentDate.month() == date.month());
        cell->setDate(currentDate, isCurrentMonth);

        // Thêm các sự kiện từ map vào ô
        if (m_allEvents.contains(currentDate)) {
            for (EventItem *event : m_allEvents.value(currentDate)) {
                cell->addEvent(event);
            }
        }
    }
}

void MonthViewWidget::clearGrid()
{
    for (DayCellWidget *cell : m_dayCells) {
        cell->clearEvents();
    }
}

void MonthViewWidget::onCellClicked(const QDate &date, const QList<EventItem*> &events)
{
    // Mở dialog chi tiết
    DayDetailDialog dialog(date, events, this);
    dialog.exec();
}
