#ifndef EVENTDIALOG_H
#define EVENTDIALOG_H

#include <QDialog>
#include <QDateTime>
#include <QColor>
#include <QMap>

// Forward declarations
class QLineEdit;
class QDateEdit;
class QComboBox;
class QCheckBox;
class QDialogButtonBox;

class EventDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EventDialog(QWidget *parent = nullptr);

    // Các hàm để MainWindow lấy dữ liệu
    QString title() const;
    QDateTime startDateTime() const;
    QDateTime endDateTime() const;
    QColor categoryColor() const;
    // Chúng ta sẽ thêm getter cho Lời nhắc sau, giờ chỉ cần UI

private slots:
    void onAllDayToggled(bool checked);
    void onStartTimeChanged(const QString &text);

private:
    void setupUi();
    void populateTimeComboBox(QComboBox *comboBox);

    // UI elements
    QLineEdit *m_titleEdit;
    QDateEdit *m_startDateEdit;
    QComboBox *m_startTimeComboBox;
    QDateEdit *m_endDateEdit;
    QComboBox *m_endTimeComboBox;
    QCheckBox *m_allDayCheckBox;
    QComboBox *m_categoryComboBox;
    QDialogButtonBox *m_buttonBox;

    // Data
    QMap<QString, QColor> m_categories;
};

#endif // EVENTDIALOG_H
