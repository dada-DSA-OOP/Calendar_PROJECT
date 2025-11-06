#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "settingsdialog.h"
#include <QMainWindow>
#include <QDate>
#include <QPushButton>
#include <QToolButton>

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

private:
    Ui::MainWindow *ui;
    void updateCalendarDisplay();

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

protected:
    void resizeEvent(QResizeEvent *event) override;

};
#endif // MAINWINDOW_H
