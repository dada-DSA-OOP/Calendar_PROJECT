#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDate>
#include <QPushButton>

// Forward declaration để tránh include vòng lặp
class QLabel;
class DayHeader;
class CalendarView; // <-- THÊM DÒNG NÀY
class QPushButton;
class QCalendarWidget;

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
    void onNewEventClicked(); // <-- THÊM DÒNG NÀY
    void onDateSelectedFromPopup(const QDate &date); // <-- THÊM SLOT MỚI NÀY

private:
    Ui::MainWindow *ui;
    void updateCalendarDisplay(); // <-- Thêm hàm private helper

    QDate m_currentMonday;    // <-- Biến lưu ngày thứ Hai của tuần hiện tại
    QPushButton *m_dateNavButton;      // <-- Con trỏ tới nhãn hiển thị ngày tháng
    DayHeader *m_dayHeader;   // <-- Con trỏ tới header của lịch
    CalendarView *m_calendarView; // <-- VÀ THÊM DÒNG NÀY
    QPushButton *m_btnPrevWeek; // <-- THÊM DÒNG NÀY
    QPushButton *m_btnNextWeek; // <-- THÊM DÒNG NÀY
    QCalendarWidget *m_calendarPopup; // <-- THÊM BIẾN NÀY ĐỂ TRUY CẬP LỊCH POPUP

protected:
    void resizeEvent(QResizeEvent *event) override;

};
#endif // MAINWINDOW_H
