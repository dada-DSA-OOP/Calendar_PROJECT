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
    void changeBackgroundImage(int index, const QString &imagePath);
    void setCalendarTransparency(bool transparent);

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

protected:
    void resizeEvent(QResizeEvent *event) override;

};
#endif // MAINWINDOW_H
