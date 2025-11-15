#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "settingsdialog.h"
#include <QMainWindow>
#include <QDate>
#include <QPushButton>
#include <QToolButton>
#include <QAction>
#include "eventdialog.h" // Cần cho EventDialog, EditResult, RecurrenceRule
#include "eventitem.h"   // Cần cho EventItem*

// Forward declaration để tránh include vòng lặp
class QLabel;
class DayHeader;
class CalendarView;
class QPushButton;
class QCalendarWidget;
class QPropertyAnimation;
class SidePanel;
class FunnyTipWidget;
class QStackedWidget;
class SettingsDialog;
class TimeRuler;
class MonthViewWidget;
class TimetableViewWidget;
class SessionViewWidget;
class QListWidget;
class QTextEdit;
class QListWidgetItem;
class EventDialog;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots: // <-- Thêm private slots
    void showPreviousWeek();
    void showNextWeek();
    void showToday();
    void onNewEventClicked();
    void onNewMeetingClicked();

    void onNewStudyClicked();      // Slot cho "Học tập"
    void onNewHolidayClicked();    // Slot cho "Ngày lễ"
    void onNewAppointmentClicked(); // Slot cho "Cuộc hẹn"

    void onDateSelectedFromPopup(const QDate &date);
    void toggleHelpPanel();
    void toggleTipsPanel();
    void toggleSupportPanel();
    void toggleFeedbackPanel();
    void openSettingsDialog();
    void changeBackgroundImage(int index, const QString &imagePath, const QColor &color);
    void setCalendarTransparency(bool transparent);
    void onDisplayDaysChanged(int days);
    void showWorkWeek();
    void showFullWeek();
    void showMonthView();
    void showTimetableView();
    void showSessionView();
    void onTimeScaleChanged(int minutes);
    void onPrintToPdf();
    void onExportData();
    void onImportData();
    void onFilterChanged();

    // MỚI: Slot để nhận tín hiệu từ các View
    void onEventItemClicked(EventItem *item);

    // --- THÊM CÁC SLOT MỚI CHO TO-DO LIST ---
    void onAddTodoItem();
    void onTodoItemChanged(int state);
    void onTodoItemDeleted();
    void saveData(); // <-- Slot để lưu dữ liệu (có thể gọi từ nhiều nơi)

    // MỚI: Slot xử lý sự kiện kéo thả
    void onEventItemDragged(EventItem *item, const QDateTime &newStartTime, const QDateTime &newEndTime);

    void applyTimeSettings(); // Slot để áp dụng cài đặt

private:
    Ui::MainWindow *ui;
    void updateCalendarDisplay();

    void applyFilters();

    QMenu *m_filterMenu; // Con trỏ tới menu Bộ lọc

    // Lưu trữ các action để lọc
    QList<QAction*> m_categoryActions;
    QList<QAction*> m_statusActions;
    // (Thêm các list QAction* khác cho "Cuộc họp", "Lặp lại"...)

    // Lưu trữ trạng thái lọc (dùng QSet để truy cập nhanh)
    QSet<QString> m_visibleCategories;
    QSet<QString> m_visibleStatuses;

    QList<QAction*> m_recurrenceActions; // Lưu 2 action "Đơn" và "Chuỗi"
    QSet<QString> m_visibleRecurrenceTypes; // Lưu trạng thái ("Đơn", "Chuỗi")

    QList<QAction*> m_meetingStatusActions;
    QSet<QString> m_visibleMeetingStatuses;

    // Bộ lọc Loại sự kiện chính
    QList<QAction*> m_eventTypeActions;
    QSet<QString> m_visibleEventTypes;

    // Bộ lọc con cho Học tập
    QList<QAction*> m_studyMethodActions;
    QSet<QString> m_visibleStudyMethods;

    // Bộ lọc con cho Ngày lễ
    QList<QAction*> m_holidayScopeActions;
    QSet<QString> m_visibleHolidayScopes;

    // Bộ lọc con cho Cuộc hẹn
    QList<QAction*> m_appointmentTypeActions;
    QSet<QString> m_visibleAppointmentTypes;
    QList<QAction*> m_appointmentPrivacyActions;
    QSet<QString> m_visibleAppointmentPrivacy;

    QDate m_currentMonday;    // <-- Biến lưu ngày thứ Hai của tuần hiện tại
    QPushButton *m_dateNavButton;      // <-- Con trỏ tới nhãn hiển thị ngày tháng
    DayHeader *m_dayHeader;   // <-- Con trỏ tới header của lịch
    CalendarView *m_calendarView; // <-- VÀ THÊM DÒNG NÀY
    QPushButton *m_btnPrevWeek; // <-- THÊM DÒNG NÀY
    QPushButton *m_btnNextWeek; // <-- THÊM DÒNG NÀY
    QCalendarWidget *m_calendarPopup; // <-- THÊM BIẾN NÀY ĐỂ TRUY CẬP LỊCH POPUP
    QStackedWidget *m_viewStack;   // Thay thế cho m_calendarView trong layout
    MonthViewWidget *m_monthView;  // Widget xem tháng mới
    QWidget *m_topBar;
    SidePanel *m_helpPanel;
    SidePanel *m_tipsPanel;
    FunnyTipWidget *m_funnyTipWidget;
    SidePanel *m_supportPanel;
    SidePanel *m_feedbackPanel;
    QWidget *m_sidebarCalendar = nullptr;
    QToolButton *m_btnSidebarToggle = nullptr;
    bool m_sidebarVisible = false;
    QStackedWidget *m_toolbarStack;
    TimetableViewWidget *m_timetableView;
    TimeRuler *m_timeRuler;
    QWidget *m_calendarCorner;
    SessionViewWidget *m_sessionView;
    QToolButton *m_btnTimeScale;

    bool m_use24HourFormat;
    int m_timezoneOffsetSeconds;

    void loadSettings();
    void saveSettings(SettingsDialog *dialog); // Sửa để nhận dialog

    // MỚI: Các hàm trợ giúp
    void addEventFromDialog(EventDialog &dialog);
    void removeEventFromViews(EventItem *item);
    EventItem* createEventItemFromDialog(EventDialog &dialog, const QDateTime &start, const QDateTime &end);

    // Con trỏ tới các widget trong To-Do List
    QTextEdit *m_noteInput;
    QListWidget *m_todoList;

    // Đường dẫn file save
    QString m_saveFilePath;

    // Nguồn sự thật duy nhất (Single Source of Truth)
    QList<EventItem*> m_allEventItems;

    // Hàm trợ giúp cho Save/Load
    void initSavePath();
    void loadData();
    void loadEvents(const QJsonArray &eventsArray);
    void loadTodos(const QJsonArray &todosArray);

    // Hàm trợ giúp tái cấu trúc
    void addTodoItem(const QString &text, bool completed); // <--- Hàm này sẽ thay thế lambda cũ
    QJsonObject serializeEvent(EventItem *item) const;

    // --- THÊM CÁC HÀM TRỢ GIÚP MỚI CHO VIỆC SỬA/XÓA CHUỖI ---
    void updateSingleEvent(EventItem *oldItem, EventDialog &dialog);
    void updateEventSeries(EventItem *oldItem, EventDialog &dialog);
    void deleteSingleEvent(EventItem *item);
    void deleteEventSeries(EventItem *item);

    void recreateEventSeries(EventDialog &dialog, QDate seriesStartDate);

    QString m_settingsFilePath;

    int m_currentBackgroundIndex;
    QString m_currentImagePath;
    QColor m_currentSolidColor;
    bool m_isCalendarTransparent;

    // Chuyển đổi từ Local (nhập liệu) sang UTC (lưu trữ)
    QDateTime convertToStorageTime(const QDateTime &localTime) const { return localTime.toUTC(); }
    QDateTime convertToDisplayTime(const QDateTime &utcTime) const { return utcTime.toOffsetFromUtc(m_timezoneOffsetSeconds); }

protected:
    void resizeEvent(QResizeEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

};
#endif // MAINWINDOW_H
