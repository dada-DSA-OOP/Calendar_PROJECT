#include "sessionviewwidget.h"
#include "timetableslotwidget.h" // Tái sử dụng
#include "eventitem.h"
#include <QGridLayout>
#include <QLabel>
#include <QDebug>

SessionViewWidget::SessionViewWidget(QWidget *parent)
    : QWidget(parent)
{
    m_gridLayout = new QGridLayout(this);
    m_gridLayout->setContentsMargins(0, 0, 0, 0);
    m_gridLayout->setSpacing(0);

    // === TẠO CÁC HEADER ===

    // Cột 0: Header Buổi học
    QStringList sessionNames = { "Sáng", "Chiều" };
    for (int i = 0; i < 2; ++i) {
        QLabel *sessionHeader = new QLabel(sessionNames[i]);
        sessionHeader->setAlignment(Qt::AlignCenter);
        sessionHeader->setStyleSheet("font-weight: bold; border: 1px solid #cccccc; padding: 8px;");
        m_gridLayout->addWidget(sessionHeader, i + 1, 0);
    }

    // Hàng 0: Header Ngày
    QLocale viLocale(QLocale::Vietnamese);
    for (int j = 0; j < 6; ++j) { // Chỉ 6 ngày (T2 - T7)
        m_dayHeaders[j] = new QLabel(viLocale.standaloneDayName(j + 1, QLocale::ShortFormat));
        m_dayHeaders[j]->setAlignment(Qt::AlignCenter);
        m_dayHeaders[j]->setStyleSheet("font-weight: bold; border: 1px solid #cccccc; padding: 8px;");
        m_gridLayout->addWidget(m_dayHeaders[j], 0, j + 1);
    }

    m_gridLayout->addWidget(new QWidget, 0, 0); // Góc

    // === TẠO LƯỚI CÁC Ô BUỔI HỌC ===
    for (int i = 0; i < 2; ++i) { // 2 buổi (hàng)
        for (int j = 0; j < 6; ++j) { // 6 ngày (cột)
            m_sessionSlots[j][i] = new TimetableSlotWidget; // Tái sử dụng
            m_gridLayout->addWidget(m_sessionSlots[j][i], i + 1, j + 1);
        }
    }

    // Co giãn
    m_gridLayout->setRowStretch(1, 1); // Hàng Sáng
    m_gridLayout->setRowStretch(2, 1); // Hàng Chiều
    for(int j = 1; j <= 6; ++j) m_gridLayout->setColumnStretch(j, 1);

    setLayout(m_gridLayout);
}

void SessionViewWidget::addEvent(EventItem *event)
{
    m_allEvents[event->startTime().date()].append(event);
    updateView(m_currentMonday);
}

void SessionViewWidget::clearGrid()
{
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 6; ++j) {
            m_sessionSlots[j][i]->clearEvents();
        }
    }
}

void SessionViewWidget::updateView(const QDate &monday)
{
    m_currentMonday = monday;
    if (!m_currentMonday.isValid()) return;

    clearGrid();
    QLocale viLocale(QLocale::Vietnamese);

    // Định nghĩa các mốc thời gian Buổi học
    const QTime sangStart(7, 0);
    const QTime sangEnd(12, 0);
    const QTime chieuStart(13, 0);
    const QTime chieuEnd(17, 30); // 17:30

    for (int j = 0; j < 6; ++j) { // 6 ngày (Cột, T2-T7)
        QDate currentDate = m_currentMonday.addDays(j);

        QString headerText = QString("%1 (%2)")
                                 .arg(viLocale.standaloneDayName(currentDate.dayOfWeek(), QLocale::ShortFormat))
                                 .arg(currentDate.toString("d/M"));
        m_dayHeaders[j]->setText(headerText);

        if (m_allEvents.contains(currentDate)) {
            for (EventItem *event : m_allEvents.value(currentDate)) {

                QTime eventStartTime = event->startTime().time();
                QTime eventEndTime = event->endTime().time();

                // Kiểm tra overlap với Buổi Sáng
                if ((eventStartTime < sangEnd) && (eventEndTime > sangStart)) {
                    m_sessionSlots[j][0]->addEvent(event);
                }

                // Kiểm tra overlap với Buổi Chiều
                if ((eventStartTime < chieuEnd) && (eventEndTime > chieuStart)) {
                    m_sessionSlots[j][1]->addEvent(event);
                }
            }
        }
    }
}
