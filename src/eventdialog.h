#ifndef EVENTDIALOG_H
#define EVENTDIALOG_H

#include <QDialog>
#include <QDateTime>
#include <QColor>
#include <QMap>
#include <QList>
#include <QJsonObject>

// Forward declarations
class QLineEdit;
class QDateEdit;
class QComboBox;
class QCheckBox;
class QDialogButtonBox;
class QTextEdit;
class QPushButton;
class QGroupBox;
class QStackedWidget;

class EventDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EventDialog(QWidget *parent = nullptr);

    // Struct để chứa quy tắc lặp lại
    struct RecurrenceRule {
        bool isRecurrent = false;
        QList<Qt::DayOfWeek> days; // T2=1, T3=2, ..., CN=7
        QDate endDate;
    };

    void setEventData(const QString &title, const QDateTime &start, const QDateTime &end,
                      const QColor &color, const QString &description,
                      const QString &showAs, const QString &category,
                      bool isAllDay, const RecurrenceRule &rule,
                      const QString &eventType,
                      const QJsonObject &extraData);

    QString getEventType() const;
    QJsonObject getExtraData() const;

    // Các hàm để MainWindow lấy dữ liệu
    QString title() const;
    QString description() const;
    QDateTime startDateTime() const;
    QDateTime endDateTime() const;
    QColor eventColor() const;
    RecurrenceRule recurrenceRule() const;
    bool isAllDay() const;
    QString showAsStatus() const;
    QString category() const;
    enum class EditResult { Cancel, Save, Delete };
    EditResult getEditResult() const;
    void setEventType(const QString &eventType);

    // MỚI: Các hàm setter cho thời gian
    void setNewStartDateTime(const QDateTime &start);
    void setNewEndDateTime(const QDateTime &end);

    void setNewRecurrenceRule(const RecurrenceRule &rule);
    void setTimezoneOffset(int offsetSeconds);

private slots:
    void onAllDayToggled(bool checked);
    void onStartTimeChanged(const QString &text);
    void onColorButtonClicked();
    void onCategoryChanged(const QString &categoryName);
    void onSaveClicked();
    void onDeleteClicked();
    void onEventTypeChanged(const QString &text);

private:
    void setupUi();
    void populateTimeComboBox(QComboBox *comboBox);
    void updateColorButton(const QColor &color);

    QWidget* createMeetingPage();
    QWidget* createStudyPage();
    QWidget* createHolidayPage();
    QWidget* createAppointmentPage();

    // UI elements
    QLineEdit *m_titleEdit;
    QDateEdit *m_startDateEdit;
    QComboBox *m_startTimeComboBox;
    QDateEdit *m_endDateEdit;
    QComboBox *m_endTimeComboBox;
    QCheckBox *m_allDayCheckBox;
    QComboBox *m_showAsComboBox;
    QComboBox *m_categoryComboBox;
    QPushButton *m_colorButton;
    QTextEdit *m_descriptionEdit;

    // --- Recurrence Widgets ---
    QCheckBox *m_recurrenceCheckBox;
    QWidget *m_recurrenceWidget;
    QList<QCheckBox*> m_weekdayCheckBoxes;
    QDateEdit *m_recurrenceEndDateEdit;

    QDialogButtonBox *m_buttonBox;

    // Data
    QMap<QString, QColor> m_categories;
    QColor m_selectedColor;

    EditResult m_editResult; // Biến lưu trữ kết quả (Lưu, Xóa, Hủy)
    QPushButton *m_deleteButton;

    // Biến lưu múi giờ hiển thị (tính bằng giây)
    int m_timezoneOffsetSeconds;

    QComboBox *m_eventTypeComboBox;
    QStackedWidget *m_extraDataStack;

    bool m_isMeeting;
};

#endif // EVENTDIALOG_H
