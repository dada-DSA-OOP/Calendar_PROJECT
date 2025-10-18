#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDate>
#include <QPushButton>

// Forward declaration để tránh include vòng lặp
class QLabel;
class DayHeader;
class CalendarView;
class QPushButton;
class QCalendarWidget;
class QPropertyAnimation;

class SidePanel;
class FunnyTipWidget;

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

protected:
    void resizeEvent(QResizeEvent *event) override;

};
#endif // MAINWINDOW_H
