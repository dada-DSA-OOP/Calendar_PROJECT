#include "monthviewwidget.h"
#include "daycellwidget.h"
#include "daydetaildialog.h"
#include "eventitem.h"
#include <QGridLayout>

MonthViewWidget::MonthViewWidget(QWidget *parent)
    : QWidget(parent)
    , m_timezoneOffsetSeconds(QDateTime::currentDateTime().offsetFromUtc())
{
    m_gridLayout = new QGridLayout(this);
    m_gridLayout->setContentsMargins(0, 0, 0, 0);
    m_gridLayout->setSpacing(0);

    for (int i = 0; i < 42; ++i) { // 6 tuần * 7 ngày [cite: 2]
        DayCellWidget *cell = new DayCellWidget;
        m_dayCells.append(cell);
        m_gridLayout->addWidget(cell, i / 7, i % 7);

        // Giữ lại kết nối cũ
        connect(cell, &DayCellWidget::cellClicked, this, &MonthViewWidget::onCellClicked);

        // MỚI: Kết nối tín hiệu click của sự kiện
        // (Lưu ý: Bạn phải tự thêm tín hiệu eventClicked(EventItem*) vào DayCellWidget)
        connect(cell, &DayCellWidget::eventClicked, this, &MonthViewWidget::eventClicked);
    }

    setLayout(m_gridLayout);
}

void MonthViewWidget::addEvent(EventItem *event)
{
    QDate displayDate = event->startTime().toOffsetFromUtc(m_timezoneOffsetSeconds).date();
    m_allEvents[displayDate].append(event);
}

// MỚI: Hàm để xóa
void MonthViewWidget::removeEvent(EventItem *event)
{
    if (!event) return;

    QDate displayDate = event->startTime().toOffsetFromUtc(m_timezoneOffsetSeconds).date();
    if (m_allEvents.contains(displayDate)) {
        m_allEvents[displayDate].removeAll(event);

        // Tìm ô cell tương ứng và yêu cầu nó xóa
        for(DayCellWidget *cell : m_dayCells) {
            if (cell->date() == displayDate) { // So sánh với displayDate
                cell->removeEvent(event);
                break;
            }
        }
    }
}

void MonthViewWidget::updateView(const QDate &date)
{
    clearGrid();

    // 'date' truyền vào là Local, 'startDate' là Local
    QDate firstDayOfMonth = date.addDays(-(date.day() - 1));
    int daysToMonday = firstDayOfMonth.dayOfWeek() - 1;
    QDate startDate = firstDayOfMonth.addDays(-daysToMonday);

    QMap<QDate, QList<EventItem*>> newAllEvents;
    for (const auto& list : m_allEvents.values()) {
        for (EventItem* event : list) {
            QDate displayDate = event->startTime().toOffsetFromUtc(m_timezoneOffsetSeconds).date();
            newAllEvents[displayDate].append(event);
        }
    }
    m_allEvents = newAllEvents;

    for (int i = 0; i < 42; ++i) {
        QDate currentDate = startDate.addDays(i); // currentDate là Local
        DayCellWidget *cell = m_dayCells[i];

        bool isCurrentMonth = (currentDate.month() == date.month());
        cell->setDate(currentDate, isCurrentMonth);

        // Logic kiểm tra này giờ đã đúng
        if (m_allEvents.contains(currentDate)) {
            for (EventItem *event : m_allEvents.value(currentDate)) {
                if (event && !event->isFilteredOut())
                {
                    cell->addEvent(event);
                }
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
    DayDetailDialog dialog(date, events, this);
    dialog.exec();
}

void MonthViewWidget::setTimezoneOffset(int offsetSeconds)
{
    m_timezoneOffsetSeconds = offsetSeconds;
    // Tên hàm cập nhật của view này là updateView
    updateView(QDate::currentDate());
}
