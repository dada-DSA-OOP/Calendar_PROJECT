#ifndef DAYDETAILDIALOG_H
#define DAYDETAILDIALOG_H

#include <QDialog>
#include <QDate>

class QListWidget;
class EventItem; // Forward declaration

class DayDetailDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DayDetailDialog(const QDate &date, const QList<EventItem*> &events, QWidget *parent = nullptr);

private:
    QListWidget *m_listWidget;
};

#endif // DAYDETAILDIALOG_H
