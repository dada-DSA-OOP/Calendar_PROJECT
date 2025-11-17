#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDate>
#include <QList> // Thêm QList để dùng QSet
#include <QSet>  // Thêm QSet

// --- BAO GỒM CÁC TỆP ĐỊNH NGHĨA QUAN TRỌNG ---
// Các tệp này được include đầy đủ (không forward declare)
// vì chúng ta cần định nghĩa đầy đủ của các kiểu dữ liệu này
#include "settingsdialog.h"
#include "eventdialog.h"
#include "eventitem.h"

// --- FORWARD DECLARATIONS ---
// Tiết kiệm thời gian biên dịch bằng cách chỉ khai báo "tên" của lớp
class QLabel;
class DayHeader;
class CalendarView;
class QPushButton;
class QToolButton;
class QAction;
class QCalendarWidget;
class QPropertyAnimation;
class SidePanel;
class FunnyTipWidget;
class QStackedWidget;
class TimeRuler;
class MonthViewWidget;
class TimetableViewWidget;
class SessionViewWidget;
class QListWidget;
class QTextEdit;
class QListWidgetItem;
class QMenu;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    // === 1. HÀM DỰNG & HÀM HỦY ===
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    // === 2. HÀM OVERRIDE TỪ QT (SỰ KIỆN) ===
    void resizeEvent(QResizeEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    // === 3. CÁC SLOT GIAO DIỆN (UI SLOTS) ===

    // --- 3a. Slots Điều hướng & Chuyển View ---
    void showPreviousWeek();
    void showNextWeek();
    void showToday();
    void onDateSelectedFromPopup(const QDate &date);
    void onDisplayDaysChanged(int days);
    void showWorkWeek();
    void showFullWeek();
    void showMonthView();
    void showTimetableView();
    void showSessionView();

    // --- 3b. Slots Toolbar (Tạo mới, Cài đặt, Panels) ---
    void onNewEventClicked();
    void onNewMeetingClicked();
    void onNewStudyClicked();
    void onNewHolidayClicked();
    void onNewAppointmentClicked();
    void openSettingsDialog();
    void onTimeScaleChanged(int minutes);
    void toggleHelpPanel();
    void toggleTipsPanel();
    void toggleSupportPanel();
    void toggleFeedbackPanel();

    // --- 3c. Slots Tương tác Lịch ---
    void onEventItemClicked(EventItem *item);
    void onEventItemDragged(EventItem *item, const QDateTime &newStartTime, const QDateTime &newEndTime);

    // --- 3d. Slots Dữ liệu, Lọc & To-Do ---
    void onFilterChanged();
    void onPrintToPdf();
    void onExportData();
    void onImportData();
    void onAddTodoItem();
    void onTodoItemChanged(int state);
    void onTodoItemDeleted();
    void saveData(); // (Được gọi bởi các hàm khác)
    void applyTimeSettings();

private:
    Ui::MainWindow *ui; // Con trỏ do Qt Designer tạo ra

    // === 4. HÀM LOGIC TRỢ GIÚP (HELPER FUNCTIONS) ===

    // --- 4a. Logic Hiển thị & Cập nhật UI ---
    /**
     * @brief Hàm "bộ não" cập nhật tất cả các view dựa trên m_currentMonday và view hiện tại.
     */
    void updateCalendarDisplay();
    /**
     * @brief Lặp qua m_allEventItems và ẩn/hiện chúng dựa trên QSet bộ lọc.
     */
    void applyFilters();
    /**
     * @brief Áp dụng ảnh nền hoặc màu nền cho QMainWindow.
     */
    void changeBackgroundImage(int index, const QString &imagePath, const QColor &color);
    /**
     * @brief Bật/tắt thuộc tính động [transparent="true"] cho các view lịch.
     */
    void setCalendarTransparency(bool transparent);

    // --- 4b. Logic Quản lý Sự kiện (CRUD) ---
    /**
     * @brief Thêm sự kiện (đơn hoặc lặp) từ EventDialog đã được chấp nhận.
     */
    void addEventFromDialog(EventDialog &dialog);
    /**
     * @brief Tạo một EventItem* mới, lưu vào m_allEventItems, và trả về con trỏ.
     */
    EventItem* createEventItemFromDialog(EventDialog &dialog, const QDateTime &start, const QDateTime &end);
    /**
     * @brief Xóa một EventItem* khỏi m_allEventItems và tất cả các view.
     */
    void removeEventFromViews(EventItem *item);
    /**
     * @brief Xử lý logic "Sửa chỉ sự kiện này" (tách sự kiện khỏi chuỗi).
     */
    void updateSingleEvent(EventItem *oldItem, EventDialog &dialog);
    /**
     * @brief Xử lý logic "Sửa tất cả sự kiện trong chuỗi".
     */
    void updateEventSeries(EventItem *oldItem, EventDialog &dialog);
    /**
     * @brief Xử lý logic "Xóa chỉ sự kiện này".
     */
    void deleteSingleEvent(EventItem *item);
    /**
     * @brief Xử lý logic "Xóa tất cả sự kiện trong chuỗi".
     */
    void deleteEventSeries(EventItem *item);
    /**
     * @brief Hàm trợ giúp cho updateEventSeries, tạo lại chuỗi từ ngày bắt đầu mới.
     */
    void recreateEventSeries(EventDialog &dialog, QDate seriesStartDate);

    // --- 4c. Logic Lưu/Tải (Persistence) ---
    /**
     * @brief Xác định đường dẫn lưu file data.json và settings.json.
     */
    void initSavePath();
    /**
     * @brief Tải toàn bộ dữ liệu từ data.json (gọi loadEvents, loadTodos).
     */
    void loadData();
    /**
     * @brief Tải mảng "events" từ JSON vào m_allEventItems.
     */
    void loadEvents(const QJsonArray &eventsArray);
    /**
     * @brief Tải mảng "todos" từ JSON vào m_todoList.
     */
    void loadTodos(const QJsonArray &todosArray);
    /**
     * @brief Tải cài đặt từ settings.json.
     */
    void loadSettings();
    /**
     * @brief Lưu cài đặt hiện tại vào settings.json.
     */
    void saveSettings(SettingsDialog *dialog);
    /**
     * @brief Chuyển một EventItem* thành một QJsonObject để lưu.
     */
    QJsonObject serializeEvent(EventItem *item) const;

    // --- 4d. Logic Tiện ích & Múi giờ ---
    /**
     * @brief Hàm trợ giúp để tạo một item widget trong m_todoList.
     */
    void addTodoItem(const QString &text, bool completed);
    /**
     * @brief Chuyển đổi QDateTime từ múi giờ Local (nhập liệu) sang UTC (lưu trữ).
     */
    QDateTime convertToStorageTime(const QDateTime &localTime) const { return localTime.toUTC(); }
    /**
     * @brief Chuyển đổi QDateTime từ UTC (lưu trữ) sang múi giờ Local (hiển thị).
     */
    QDateTime convertToDisplayTime(const QDateTime &utcTime) const { return utcTime.toOffsetFromUtc(m_timezoneOffsetSeconds); }


    // === 5. BIẾN THÀNH VIÊN (MEMBER VARIABLES) ===

    // --- 5a. Con trỏ UI (Giao diện chính) ---
    QStackedWidget *m_toolbarStack; // Stack chứa các thanh công cụ (Home, View, Help)
    QStackedWidget *m_viewStack;    // Stack chứa các view lịch (Tuần, Tháng, TKB)
    QWidget *m_topBar;              // Thanh widget chứa Tabs và Toolbar Stack
    QToolButton *m_btnSidebarToggle;// Nút 3 gạch
    QWidget *m_sidebarCalendar;     // Panel bên trái (lịch nhỏ & To-Do)
    QPushButton *m_dateNavButton;   // Nút "Tháng 11, 2025"
    QPushButton *m_btnPrevWeek;     // Nút điều hướng Lùi
    QPushButton *m_btnNextWeek;     // Nút điều hướng Tới
    QCalendarWidget *m_calendarPopup; // Lịch popup khi nhấn vào m_dateNavButton
    QToolButton *m_btnTimeScale;    // Nút "Tỉ lệ thời gian"

    // --- 5b. Con trỏ UI (Các View Lịch) ---
    CalendarView *m_calendarView;       // View timeline (Ngày/Tuần)
    MonthViewWidget *m_monthView;       // View tháng
    TimetableViewWidget *m_timetableView; // View TKB (Tiết)
    SessionViewWidget *m_sessionView;   // View TKB (Buổi)

    // --- 5c. Con trỏ UI (Các thành phần Lịch) ---
    DayHeader *m_dayHeader;         // Header (T2, T3, T4...)
    TimeRuler *m_timeRuler;         // Thước giờ (0:00, 1:00...)
    QWidget *m_calendarCorner;      // Góc trên bên trái (trên TimeRuler)

    // --- 5d. Con trỏ UI (Panel bên & To-Do) ---
    SidePanel *m_helpPanel;
    SidePanel *m_tipsPanel;
    SidePanel *m_supportPanel;
    SidePanel *m_feedbackPanel;
    FunnyTipWidget *m_funnyTipWidget; // Mẹo vui
    QTextEdit *m_noteInput;           // Ô nhập To-Do
    QListWidget *m_todoList;          // Danh sách To-Do

    // --- 5e. Trạng thái Bộ lọc (Filter State) ---
    QMenu *m_filterMenu; // Con trỏ menu "Bộ lọc" (dùng chung)
    // Loại sự kiện
    QList<QAction*> m_eventTypeActions;
    QSet<QString> m_visibleEventTypes;
    // (Học tập)
    QList<QAction*> m_studyMethodActions;
    QSet<QString> m_visibleStudyMethods;
    // (Ngày lễ)
    QList<QAction*> m_holidayScopeActions;
    QSet<QString> m_visibleHolidayScopes;
    // (Cuộc hẹn)
    QList<QAction*> m_appointmentTypeActions;
    QSet<QString> m_visibleAppointmentTypes;
    QList<QAction*> m_appointmentPrivacyActions;
    QSet<QString> m_visibleAppointmentPrivacy;
    // (Cuộc họp)
    QList<QAction*> m_meetingStatusActions;
    QSet<QString> m_visibleMeetingStatuses;
    // (Category/Tag)
    QList<QAction*> m_categoryActions;
    QSet<QString> m_visibleCategories;
    // (Trạng thái Rảnh/Bận)
    QList<QAction*> m_statusActions;
    QSet<QString> m_visibleStatuses;
    // (Lặp lại)
    QList<QAction*> m_recurrenceActions;
    QSet<QString> m_visibleRecurrenceTypes;

    // --- 5f. Trạng thái Chung (General State) ---
    /**
     * @brief Ngày "mỏ neo" (anchor) cho việc hiển thị.
     * Ở view Tuần/Ngày/TKB: Là ngày đầu tiên được hiển thị.
     * Ở view Tháng: Là ngày Thứ Hai của tuần đầu tiên của tháng.
     */
    QDate m_currentMonday;
    bool m_sidebarVisible;
    bool m_use24HourFormat;         // Định dạng 12/24 giờ
    int m_timezoneOffsetSeconds;    // Múi giờ (so với UTC)

    // --- 5g. Trạng thái Nền (Background State) ---
    int m_currentBackgroundIndex;   // ID của ảnh nền
    QString m_currentImagePath;     // Đường dẫn ảnh tùy chỉnh
    QColor m_currentSolidColor;     // Màu nền tùy chỉnh
    bool m_isCalendarTransparent;   // Cài đặt độ trong suốt

    // --- 5h. Dữ liệu & Lưu trữ (Data & Persistence) ---
    /**
     * @brief NGUỒN SỰ THẬT DUY NHẤT (Single Source of Truth).
     * Tất cả sự kiện của lịch được lưu ở đây.
     */
    QList<EventItem*> m_allEventItems;
    QString m_saveFilePath;         // Đường dẫn file data.json
    QString m_settingsFilePath;     // Đường dẫn file settings.json
};
#endif // MAINWINDOW_H
