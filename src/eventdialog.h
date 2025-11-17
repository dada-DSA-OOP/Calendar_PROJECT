#ifndef EVENTDIALOG_H
#define EVENTDIALOG_H

#include <QDialog>
#include <QDateTime> // Cần cho QDateTime
#include <QColor>    // Cần cho QColor
#include <QMap>      // Cần cho m_categories
#include <QList>     // Cần cho QList
#include <QJsonObject> // Cần cho getExtraData / setEventData

// --- Forward Declarations (Khai báo trước) ---
// (Khai báo tên lớp để tránh include .h đầy đủ, tăng tốc độ biên dịch)
class QLineEdit;
class QDateEdit;
class QComboBox;
class QCheckBox;
class QDialogButtonBox;
class QTextEdit;
class QPushButton;
class QGroupBox;
class QStackedWidget;

/**
 * @brief Hộp thoại (Dialog) chính để Tạo và Chỉnh sửa sự kiện.
 *
 * Lớp này chịu trách nhiệm hiển thị form nhập liệu,
 * bao gồm các form động (dynamic forms) cho các loại sự kiện khác nhau
 * (Cuộc họp, Học tập, v.v.) và xử lý logic validation cơ bản.
 */
class EventDialog : public QDialog
{
    Q_OBJECT

public:
    // === 1. CÁC KIỂU DỮ LIỆU CÔNG KHAI (PUBLIC TYPES) ===

    /**
     * @brief Cấu trúc lưu trữ quy tắc lặp lại của một sự kiện.
     */
    struct RecurrenceRule {
        bool isRecurrent = false;     // Sự kiện có lặp lại không?
        QList<Qt::DayOfWeek> days;    // Danh sách ngày lặp (T2=1, ..., CN=7)
        QDate endDate;                // Ngày kết thúc lặp
    };

    /**
     * @brief Enum xác định hành động của người dùng khi đóng dialog.
     */
    enum class EditResult {
        Cancel, // Người dùng nhấn Hủy hoặc đóng cửa sổ
        Save,   // Người dùng nhấn Lưu
        Delete  // Người dùng nhấn Xóa
    };

    // === 2. HÀM DỰNG (CONSTRUCTOR) ===

    explicit EventDialog(QWidget *parent = nullptr);

    // === 3. HÀM "SET" CÔNG KHAI (PUBLIC SETTERS) ===
    // (Dùng để điền dữ liệu vào dialog)

    /**
     * @brief Điền toàn bộ dữ liệu của một sự kiện hiện có vào dialog (chế độ Chỉnh sửa).
     */
    void setEventData(const QString &title, const QDateTime &start, const QDateTime &end,
                      const QColor &color, const QString &description,
                      const QString &showAs, const QString &category,
                      bool isAllDay, const RecurrenceRule &rule,
                      const QString &eventType,
                      const QJsonObject &extraData);

    /**
     * @brief Đặt loại sự kiện (thường dùng khi tạo sự kiện mới).
     */
    void setEventType(const QString &eventType);

    /**
     * @brief Đặt múi giờ hiển thị (tính bằng giây so với UTC).
     */
    void setTimezoneOffset(int offsetSeconds);

    /**
     * @brief Đặt thời gian bắt đầu (dùng khi kéo-thả sự kiện).
     */
    void setNewStartDateTime(const QDateTime &start);

    /**
     * @brief Đặt thời gian kết thúc (dùng khi kéo-thả sự kiện).
     */
    void setNewEndDateTime(const QDateTime &end);

    /**
     * @brief Đặt quy tắc lặp lại (dùng khi kéo-thả sự kiện lặp).
     */
    void setNewRecurrenceRule(const RecurrenceRule &rule);

    // === 4. HÀM "GET" CÔNG KHAI (PUBLIC GETTERS) ===
    // (Dùng để lấy dữ liệu từ dialog sau khi đóng)

    QString title() const;
    QString description() const;
    QDateTime startDateTime() const;
    QDateTime endDateTime() const;
    QColor eventColor() const;
    RecurrenceRule recurrenceRule() const;
    bool isAllDay() const;
    QString showAsStatus() const;
    QString category() const;
    QString getEventType() const;
    QJsonObject getExtraData() const;
    EditResult getEditResult() const;

private slots:
    // === 5. SLOT NỘI BỘ (PRIVATE SLOTS) ===
    // (Phản ứng với các tương tác UI)

    void onAllDayToggled(bool checked);        // Khi tick "Cả ngày"
    void onStartTimeChanged(const QString &text); // Khi thay đổi giờ bắt đầu
    void onColorButtonClicked();               // Khi nhấn nút chọn màu
    void onCategoryChanged(const QString &categoryName); // Khi đổi Thẻ/Tag
    void onSaveClicked();                      // Khi nhấn "Lưu"
    void onDeleteClicked();                    // Khi nhấn "Xóa"
    void onEventTypeChanged(const QString &text); // Khi đổi "Loại sự kiện"

private:
    // === 6. HÀM TRỢ GIÚP NỘI BỘ (PRIVATE HELPERS) ===

    /**
     * @brief Hàm chính xây dựng toàn bộ UI (widget, layout, connect).
     */
    void setupUi();

    /**
     * @brief Đổ danh sách 00:00, 00:30, 01:00... vào ComboBox chọn giờ.
     */
    void populateTimeComboBox(QComboBox *comboBox);

    /**
     * @brief Cập nhật style QSS (màu nền, màu chữ) cho nút chọn màu.
     */
    void updateColorButton(const QColor &color);

    // --- Các hàm tạo trang cho QStackedWidget ---
    QWidget* createMeetingPage();
    QWidget* createStudyPage();
    QWidget* createHolidayPage();
    QWidget* createAppointmentPage();

    // === 7. BIẾN THÀNH VIÊN (MEMBER VARIABLES) ===

    // --- 7a. Các Widget UI chính ---
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
    QDialogButtonBox *m_buttonBox;
    QPushButton *m_deleteButton;

    // --- 7b. Các Widget UI Lặp lại (Recurrence) ---
    QCheckBox *m_recurrenceCheckBox;
    QWidget *m_recurrenceWidget; // Container cho form lặp lại
    QList<QCheckBox*> m_weekdayCheckBoxes; // Danh sách 7 checkbox (T2-CN)
    QDateEdit *m_recurrenceEndDateEdit;

    // --- 7c. Các Widget UI Form động ---
    QComboBox *m_eventTypeComboBox; // ComboBox (Sự kiện, Cuộc họp,...)
    QStackedWidget *m_extraDataStack; // Stack chứa các form chi tiết

    // --- 7d. Biến Dữ liệu & Trạng thái ---
    QMap<QString, QColor> m_categories; // Map ("Đỏ" -> QColor("#ff605d"))
    QColor m_selectedColor;             // Màu "thật" đang được chọn
    EditResult m_editResult;            // Lưu kết quả (Lưu, Xóa, Hủy)
    int m_timezoneOffsetSeconds;        // Múi giờ đang hiển thị
    bool m_isMeeting;                   // (Biến này có vẻ đã lỗi thời, không còn dùng)
};

#endif // EVENTDIALOG_H
