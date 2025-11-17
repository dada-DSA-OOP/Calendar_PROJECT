#include "eventdialog.h"

#include <QLineEdit>
#include <QDateEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QTextEdit>
#include <QColorDialog>
#include <QGroupBox>
#include <QTimeZone> // Cần để xử lý múi giờ
#include <QDebug>
#include <QCalendarWidget> // Cần để tùy chỉnh popup lịch
#include <QStackedWidget> // Cần cho form động
#include <QRadioButton>
#include <QSignalBlocker> // Cần để tránh tín hiệu không mong muốn khi setEventData

// =================================================================================
// === 1. HÀM DỰNG & CÀI ĐẶT UI (CONSTRUCTOR & UI SETUP)
// =================================================================================

/**
 * @brief Hàm dựng của EventDialog.
 */
EventDialog::EventDialog(QWidget *parent)
    : QDialog(parent)
    , m_timezoneOffsetSeconds(QDateTime::currentDateTime().offsetFromUtc()) // Lấy múi giờ hệ thống
{
    // Khởi tạo "Nguồn sự thật" (Source of Truth) cho các Thẻ/Tag và màu mặc định.
    // (ĐỒNG BỘ VỚI FILTER MENU TRONG MAINWINDOW)
    m_categories["Không"] = Qt::gray;
    m_categories["Đỏ"]    = QColor("#ff605d");
    m_categories["Cam"] = QColor("#ffa74f");
    m_categories["Vàng"] = QColor("#ffff7f");
    m_categories["Xanh lá"]  = QColor("#8cbb63");
    m_categories["Xanh dương"]   = QColor("#c8e2f1");
    m_categories["Tím"] = QColor("#e3dced");

    // Đặt màu mặc định
    m_selectedColor = m_categories.value("Không");

    // Gọi hàm xây dựng giao diện
    setupUi();
    setWindowTitle("Tạo/Chỉnh sửa sự kiện");
    setMinimumWidth(450);
}

/**
 * @brief Hàm chính xây dựng toàn bộ giao diện người dùng (UI) bằng code C++.
 */
void EventDialog::setupUi()
{
    // === 1. KHỞI TẠO CÁC WIDGET (Form cơ bản) ===
    m_titleEdit = new QLineEdit;
    m_allDayCheckBox = new QCheckBox("Cả ngày");
    m_showAsComboBox = new QComboBox;
    m_showAsComboBox->addItems({"Bận", "Rảnh", "Làm việc ở nơi khác", "Dự định", "Vắng mặt"});

    // -- Thời gian bắt đầu --
    m_startDateEdit = new QDateEdit(QDate::currentDate());
    m_startDateEdit->setCalendarPopup(true);
    m_startDateEdit->setDisplayFormat("dd/MM/yyyy");
    m_startTimeComboBox = new QComboBox;
    populateTimeComboBox(m_startTimeComboBox);
    m_startTimeComboBox->setEditable(true); // Cho phép người dùng nhập giờ lẻ

    // -- Thời gian kết thúc --
    m_endDateEdit = new QDateEdit(QDate::currentDate());
    m_endDateEdit->setCalendarPopup(true);
    m_endDateEdit->setDisplayFormat("dd/MM/yyyy");
    m_endTimeComboBox = new QComboBox;
    populateTimeComboBox(m_endTimeComboBox);
    m_endTimeComboBox->setEditable(true);

    // -- Thẻ/Tag (Category) --
    m_categoryComboBox = new QComboBox;
    // Thêm theo đúng thứ tự đã định nghĩa trong m_categories
    m_categoryComboBox->addItems(m_categories.keys());

    // -- Nút chọn màu tùy chỉnh --
    m_colorButton = new QPushButton;
    m_colorButton->setObjectName("colorButton");
    m_colorButton->setCursor(Qt::PointingHandCursor);
    updateColorButton(m_selectedColor); // Cập nhật màu ban đầu

    // -- Mô tả --
    m_descriptionEdit = new QTextEdit;
    m_descriptionEdit->setPlaceholderText("Thêm mô tả chi tiết cho sự kiện...");
    m_descriptionEdit->setMinimumHeight(80);

    // === 2. KHỞI TẠO KHU VỰC "LẶP LẠI" (Recurrence) ===
    m_recurrenceCheckBox = new QCheckBox("Lặp lại sự kiện");
    m_recurrenceWidget = new QWidget; // Widget container (để ẩn/hiện)
    m_recurrenceWidget->setObjectName("recurrenceWidget");
    QFormLayout *recurrenceLayout = new QFormLayout(m_recurrenceWidget);
    recurrenceLayout->setContentsMargins(0, 9, 0, 0);

    // -- Các CheckBox T2-CN --
    QHBoxLayout *weekdaysLayout = new QHBoxLayout;
    weekdaysLayout->setSpacing(5);
    QStringList weekdays = {"T2", "T3", "T4", "T5", "T6", "T7", "CN"};
    for (const QString &day : weekdays) {
        QCheckBox *cb = new QCheckBox(day);
        weekdaysLayout->addWidget(cb);
        m_weekdayCheckBoxes.append(cb); // Lưu lại con trỏ
    }
    weekdaysLayout->addStretch();

    // -- Ngày kết thúc lặp lại --
    m_recurrenceEndDateEdit = new QDateEdit(QDate::currentDate().addMonths(1));
    m_recurrenceEndDateEdit->setCalendarPopup(true);
    m_recurrenceEndDateEdit->setDisplayFormat("dd/MM/yyyy");
    m_recurrenceEndDateEdit->setMinimumDate(QDate::currentDate().addDays(1));

    recurrenceLayout->addRow("Lặp vào các thứ:", weekdaysLayout);
    recurrenceLayout->addRow("Cho đến ngày:", m_recurrenceEndDateEdit);
    m_recurrenceWidget->setVisible(false); // Ẩn đi lúc đầu

    // --- Cấu hình lịch popup (Tiếng Việt, Thứ 2 đầu tuần) ---
    auto configureCalendar = [](QDateEdit* dateEdit) {
        if (dateEdit->calendarPopup()) {
            QCalendarWidget* popupCalendar = dateEdit->calendarWidget();
            if (popupCalendar) {
                popupCalendar->setLocale(QLocale(QLocale::Vietnamese));
                popupCalendar->setFirstDayOfWeek(Qt::Monday);
            }
        }
    };
    configureCalendar(m_startDateEdit);
    configureCalendar(m_endDateEdit);
    configureCalendar(m_recurrenceEndDateEdit);

    // === 3. KHỞI TẠO KHU VỰC "FORM ĐỘNG" (Dynamic Form Stack) ===
    m_eventTypeComboBox = new QComboBox;
    m_eventTypeComboBox->addItem("Sự kiện");        // Index 0
    m_eventTypeComboBox->addItem("Cuộc họp");       // Index 1
    m_eventTypeComboBox->addItem("Học tập");         // Index 2
    m_eventTypeComboBox->addItem("Ngày lễ");         // Index 3
    m_eventTypeComboBox->addItem("Cuộc hẹn");       // Index 4

    m_extraDataStack = new QStackedWidget;
    m_extraDataStack->addWidget(new QWidget); // Index 0: Trang trống
    m_extraDataStack->addWidget(createMeetingPage());   // Index 1
    m_extraDataStack->addWidget(createStudyPage());     // Index 2
    m_extraDataStack->addWidget(createHolidayPage());    // Index 3
    m_extraDataStack->addWidget(createAppointmentPage());// Index 4

    // === 4. KHỞI TẠO CÁC NÚT BẤM (OK, Cancel, Delete) ===
    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    m_deleteButton = m_buttonBox->addButton("Xóa", QDialogButtonBox::DestructiveRole);
    m_deleteButton->setVisible(false); // Chỉ hiện khi chỉnh sửa

    // Đặt tên ObjectName để định kiểu QSS
    this->setObjectName("EventDialog");
    m_buttonBox->button(QDialogButtonBox::Ok)->setText("Lưu");
    m_buttonBox->button(QDialogButtonBox::Ok)->setObjectName("okButton");
    m_buttonBox->button(QDialogButtonBox::Cancel)->setObjectName("cancelButton");
    m_deleteButton->setObjectName("deleteButton");

    // === 5. ĐẶT GIÁ TRỊ MẶC ĐỊNH (Smart Defaults) ===
    QTime now = QTime::currentTime();
    int minutes = (now.hour() * 60 + now.minute() + 15) / 30 * 30; // Làm tròn lên 30p gần nhất
    QTime startTime = QTime(minutes / 60, minutes % 60);
    m_startTimeComboBox->setCurrentText(startTime.toString("HH:mm"));
    m_endTimeComboBox->setCurrentText(startTime.addSecs(3600).toString("HH:mm")); // +1 giờ

    // === 6. SẮP XẾP LAYOUT (Layout Assembly) ===
    QHBoxLayout *startLayout = new QHBoxLayout;
    startLayout->addWidget(m_startDateEdit);
    startLayout->addWidget(m_startTimeComboBox);

    QHBoxLayout *endLayout = new QHBoxLayout;
    endLayout->addWidget(m_endDateEdit);
    endLayout->addWidget(m_endTimeComboBox);

    // Form layout chính
    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow("Tiêu đề:", m_titleEdit);
    formLayout->addRow("Loại sự kiện:", m_eventTypeComboBox);
    formLayout->addRow("Bắt đầu:", startLayout);
    formLayout->addRow("Kết thúc:", endLayout);
    formLayout->addRow("", m_allDayCheckBox);
    formLayout->addRow("Trạng thái:", m_showAsComboBox);
    formLayout->addRow("Thẻ/Tag:", m_categoryComboBox);
    formLayout->addRow("Màu sắc:", m_colorButton);
    formLayout->addRow("", m_recurrenceCheckBox);
    formLayout->addRow(m_recurrenceWidget); // Thêm widget container
    formLayout->addRow("Mô tả:", m_descriptionEdit);

    // Layout tổng của Dialog
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(formLayout);     // Form cơ bản
    mainLayout->addWidget(m_extraDataStack); // Form động (Cuộc họp, Học tập, v.v.)
    mainLayout->addWidget(m_buttonBox);      // Các nút bấm
    setLayout(mainLayout);

    // === 7. KẾT NỐI TÍN HIỆU (SIGNALS & SLOTS) ===
    connect(m_allDayCheckBox, &QCheckBox::toggled, this, &EventDialog::onAllDayToggled);
    connect(m_colorButton, &QPushButton::clicked, this, &EventDialog::onColorButtonClicked);
    connect(m_categoryComboBox, &QComboBox::currentTextChanged, this, &EventDialog::onCategoryChanged);
    connect(m_recurrenceCheckBox, &QCheckBox::toggled, m_recurrenceWidget, &QWidget::setVisible);

    // Kết nối nút Lưu/Xóa/Hủy
    connect(m_buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked, this, &EventDialog::onSaveClicked);
    connect(m_deleteButton, &QPushButton::clicked, this, &EventDialog::onDeleteClicked);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    // Kết nối ComboBox Loại sự kiện -> QStackedWidget
    connect(m_eventTypeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            m_extraDataStack, &QStackedWidget::setCurrentIndex);
    // Kết nối ComboBox Loại sự kiện -> Slot tự động chọn Tag
    connect(m_eventTypeComboBox, &QComboBox::currentTextChanged, this, &EventDialog::onEventTypeChanged);

    // Logic validation: Ngày kết thúc không được trước ngày bắt đầu
    connect(m_startDateEdit, &QDateEdit::dateChanged, this, [this](const QDate &date){
        m_endDateEdit->setMinimumDate(date);
        if (m_endDateEdit->date() < date) {
            m_endDateEdit->setDate(date);
        }
        // Ngày kết thúc lặp lại cũng phải sau ngày bắt đầu
        m_recurrenceEndDateEdit->setMinimumDate(date.addDays(1));
        if (m_recurrenceEndDateEdit->date() <= date) {
            m_recurrenceEndDateEdit->setDate(date.addMonths(1));
        }
    });
}

// =================================================================================
// === 2. HÀM "SET" CÔNG KHAI (PUBLIC SETTERS)
// =================================================================================

/**
 * @brief Nhận múi giờ từ MainWindow.
 */
void EventDialog::setTimezoneOffset(int offsetSeconds)
{
    m_timezoneOffsetSeconds = offsetSeconds;
}

/**
 * @brief Điền dữ liệu của một EventItem* hiện có vào dialog (chế độ Chỉnh sửa).
 */
void EventDialog::setEventData(const QString &title, const QDateTime &start, const QDateTime &end,
                               const QColor &color, const QString &description,
                               const QString &showAs, const QString &category,
                               bool isAllDay, const RecurrenceRule &rule,
                               const QString &eventType,
                               const QJsonObject &extraData)
{
    // Hiển thị nút "Xóa" vì đây là chế độ chỉnh sửa
    m_deleteButton->setVisible(true);

    // 1. Điền thông tin cơ bản
    m_titleEdit->setText(title);
    m_descriptionEdit->setText(description);

    // Chuyển đổi thời gian UTC (lưu trữ) sang Local (hiển thị)
    QDateTime displayStart = start.toOffsetFromUtc(m_timezoneOffsetSeconds);
    QDateTime displayEnd = end.toOffsetFromUtc(m_timezoneOffsetSeconds);

    m_startDateEdit->setDate(displayStart.date());
    m_startTimeComboBox->setCurrentText(displayStart.toString("HH:mm"));
    m_endDateEdit->setDate(displayEnd.date());
    m_endTimeComboBox->setCurrentText(displayEnd.toString("HH:mm"));

    m_allDayCheckBox->setChecked(isAllDay);
    onAllDayToggled(isAllDay);

    m_showAsComboBox->setCurrentText(showAs);
    m_categoryComboBox->setCurrentText(category);

    m_selectedColor = color;
    updateColorButton(color);

    // 2. Điền thông tin lặp lại
    m_recurrenceCheckBox->setChecked(rule.isRecurrent);
    m_recurrenceWidget->setVisible(rule.isRecurrent);
    if (rule.isRecurrent) {
        m_recurrenceEndDateEdit->setDate(rule.endDate);
        for (int i = 0; i < m_weekdayCheckBoxes.size(); ++i) {
            Qt::DayOfWeek day = static_cast<Qt::DayOfWeek>(i + 1);
            m_weekdayCheckBoxes[i]->setChecked(rule.days.contains(day));
        }
    }

    // 3. Điền thông tin form động (QStackedWidget)

    // CHẶN TÍN HIỆU: Ngăn m_eventTypeComboBox phát tín hiệu 'onEventTypeChanged'
    // (vì chúng ta muốn giữ Tag/Màu gốc, không bị ghi đè tự động)
    {
        QSignalBlocker blocker(m_eventTypeComboBox);
        m_eventTypeComboBox->setCurrentText(eventType);
    }

    // Chuyển trang trong StackedWidget
    int index = m_eventTypeComboBox->currentIndex();
    m_extraDataStack->setCurrentIndex(index);
    QWidget *currentPage = m_extraDataStack->widget(index);

    if (!currentPage) {
        qWarning() << "EventDialog::setEventData: Trang không hợp lệ, index:" << index;
        return; // Thoát an toàn
    }

    // Điền dữ liệu vào trang con tương ứng
    switch (index) {
    case 0: // Sự kiện (trống)
        break;
    case 1: // Cuộc họp
        currentPage->findChild<QLineEdit*>("hostEdit")->setText(extraData["host"].toString());
        currentPage->findChild<QLineEdit*>("participantsEdit")->setText(extraData["participants"].toString());
        currentPage->findChild<QComboBox*>("statusComboBox")->setCurrentText(extraData["meetingStatus"].toString());
        break;
    case 2: // Học tập
        currentPage->findChild<QComboBox*>("studyMethodCombo")->setCurrentText(extraData["studyMethod"].toString());
        currentPage->findChild<QLineEdit*>("subjectEdit")->setText(extraData["subject"].toString());
        break;
    case 3: // Ngày lễ
    {
        QFormLayout* layout = qobject_cast<QFormLayout*>(currentPage->findChild<QGroupBox*>()->layout());
        if (!layout) break;
        QString scope = extraData["holidayScope"].toString("Quốc tế");
        currentPage->findChild<QComboBox*>("holidayScopeCombo")->setCurrentText(scope);
        QLineEdit* customEdit = currentPage->findChild<QLineEdit*>("customHolidayEdit");
        QWidget* label = layout->labelForField(customEdit);
        bool visible = (scope == "Tùy chỉnh");
        customEdit->setVisible(visible);
        if (label) label->setVisible(visible);
        if (visible) customEdit->setText(extraData["customHolidayName"].toString());
    }
    break;
    case 4: // Cuộc hẹn
    {
        QFormLayout* layout = qobject_cast<QFormLayout*>(currentPage->findChild<QGroupBox*>()->layout());
        if (!layout) break;
        currentPage->findChild<QLineEdit*>("locationEdit")->setText(extraData["location"].toString());
        currentPage->findChild<QCheckBox*>("privateCheck")->setChecked(extraData["isPrivate"].toBool());
        QString type = extraData["appointmentType"].toString("Khác");
        currentPage->findChild<QComboBox*>("appointmentTypeCombo")->setCurrentText(type);
        QLineEdit* customEdit = currentPage->findChild<QLineEdit*>("customAppointmentEdit");
        QWidget* label = layout->labelForField(customEdit);
        bool visible = (type == "Khác");
        customEdit->setVisible(visible);
        if (label) label->setVisible(visible);
        if (visible) customEdit->setText(extraData["customAppointmentType"].toString());
    }
    break;
    default:
        break;
    }
}

/**
 * @brief Đặt thời gian bắt đầu mới (thường dùng khi kéo-thả).
 */
void EventDialog::setNewStartDateTime(const QDateTime &start)
{
    // Chuyển sang múi giờ hiển thị
    QDateTime displayStart = start.toOffsetFromUtc(m_timezoneOffsetSeconds);

    if (m_startDateEdit) m_startDateEdit->setDate(displayStart.date());
    if (m_startTimeComboBox) m_startTimeComboBox->setCurrentText(displayStart.toString("HH:mm"));
}

/**
 * @brief Đặt thời gian kết thúc mới (thường dùng khi kéo-thả).
 */
void EventDialog::setNewEndDateTime(const QDateTime &end)
{
    // Chuyển sang múi giờ hiển thị
    QDateTime displayEnd = end.toOffsetFromUtc(m_timezoneOffsetSeconds);

    if (m_endDateEdit) m_endDateEdit->setDate(displayEnd.date());
    if (m_endTimeComboBox) m_endTimeComboBox->setCurrentText(displayEnd.toString("HH:mm"));
}

/**
 * @brief Đặt quy tắc lặp lại mới (thường dùng khi kéo-thả sự kiện lặp).
 */
void EventDialog::setNewRecurrenceRule(const EventDialog::RecurrenceRule &rule)
{
    if (!m_recurrenceCheckBox || !m_recurrenceWidget || !m_recurrenceEndDateEdit) return;

    m_recurrenceCheckBox->setChecked(rule.isRecurrent);
    m_recurrenceWidget->setVisible(rule.isRecurrent);

    if (rule.isRecurrent) {
        m_recurrenceEndDateEdit->setDate(rule.endDate);
        for (int i = 0; i < m_weekdayCheckBoxes.size(); ++i) {
            Qt::DayOfWeek day = static_cast<Qt::DayOfWeek>(i + 1);
            m_weekdayCheckBoxes[i]->setChecked(rule.days.contains(day));
        }
    }
}

/**
 * @brief Đặt loại sự kiện (dùng khi tạo sự kiện mới từ menu).
 */
void EventDialog::setEventType(const QString &eventType)
{
    m_eventTypeComboBox->setCurrentText(eventType);
    // Tín hiệu 'currentTextChanged' sẽ tự động được phát ra,
    // gọi slot onEventTypeChanged() để tự động chọn Tag.
}

// =================================================================================
// === 3. HÀM "GET" CÔNG KHAI (PUBLIC GETTERS)
// =================================================================================

QString EventDialog::title() const {
    return m_titleEdit->text();
}

QString EventDialog::description() const {
    return m_descriptionEdit->toPlainText();
}

/**
 * @brief Lấy QDateTime bắt đầu (đã bao gồm thông tin múi giờ Local).
 * MainWindow sẽ chịu trách nhiệm chuyển đổi nó sang UTC để lưu trữ.
 */
QDateTime EventDialog::startDateTime() const
{
    QDate date = m_startDateEdit->date();
    if (m_allDayCheckBox->isChecked()) {
        return QDateTime(date, QTime(0, 0, 0), QTimeZone(m_timezoneOffsetSeconds));
    }
    QTime time = QTime::fromString(m_startTimeComboBox->currentText(), "HH:mm");
    return QDateTime(date, time, QTimeZone(m_timezoneOffsetSeconds));
}

/**
 * @brief Lấy QDateTime kết thúc (đã bao gồm thông tin múi giờ Local).
 */
QDateTime EventDialog::endDateTime() const
{
    QDate date = m_endDateEdit->date();
    if (m_allDayCheckBox->isChecked()) {
        return QDateTime(date, QTime(23, 59, 59), QTimeZone(m_timezoneOffsetSeconds));
    }
    QTime time = QTime::fromString(m_endTimeComboBox->currentText(), "HH:mm");
    return QDateTime(date, time, QTimeZone(m_timezoneOffsetSeconds));
}

QColor EventDialog::eventColor() const {
    return m_selectedColor; // Luôn trả về màu từ nút chọn màu
}

bool EventDialog::isAllDay() const {
    return m_allDayCheckBox->isChecked();
}

/**
 * @brief Thu thập và trả về quy tắc lặp lại từ form.
 */
EventDialog::RecurrenceRule EventDialog::recurrenceRule() const {
    RecurrenceRule rule;
    rule.isRecurrent = m_recurrenceCheckBox->isChecked();
    if (!rule.isRecurrent) return rule; // Trả về quy tắc rỗng nếu không lặp

    rule.endDate = m_recurrenceEndDateEdit->date();
    // Qt::DayOfWeek: Monday=1, ..., Sunday=7
    for (int i = 0; i < m_weekdayCheckBoxes.size(); ++i) {
        if (m_weekdayCheckBoxes[i]->isChecked()) {
            rule.days.append(static_cast<Qt::DayOfWeek>(i + 1));
        }
    }
    return rule;
}

QString EventDialog::showAsStatus() const {
    return m_showAsComboBox->currentText();
}

/**
 * @brief Lấy kết quả (Lưu hay Xóa) sau khi dialog đóng.
 */
EventDialog::EditResult EventDialog::getEditResult() const
{
    return m_editResult;
}

/**
 * @brief Lấy Thẻ/Tag (Category) dựa trên MÀU SẮC đã chọn.
 * Nếu màu là tùy chỉnh, trả về "Không".
 */
QString EventDialog::category() const {
    // Tìm xem m_selectedColor (từ nút màu) có khớp với
    // một trong các màu category chuẩn không.
    for (auto it = m_categories.constBegin(); it != m_categories.constEnd(); ++it) {
        if (it.value() == m_selectedColor) {
            return it.key(); // Trả về tên (ví dụ: "Đỏ")
        }
    }
    // Nếu là màu tùy chỉnh (không khớp), trả về "Không"
    return "Không";
}

QString EventDialog::getEventType() const
{
    return m_eventTypeComboBox->currentText();
}

/**
 * @brief Thu thập dữ liệu từ QStackedWidget (form động) và đóng gói thành QJsonObject.
 */
QJsonObject EventDialog::getExtraData() const
{
    QJsonObject data;
    int index = m_eventTypeComboBox->currentIndex();
    QWidget *currentPage = m_extraDataStack->widget(index);
    if (!currentPage) return data;

    // Dựa trên trang nào, thu thập dữ liệu từ các widget con
    switch (index) {
    case 1: // Cuộc họp
        data["host"] = currentPage->findChild<QLineEdit*>("hostEdit")->text();
        data["participants"] = currentPage->findChild<QLineEdit*>("participantsEdit")->text();
        data["meetingStatus"] = currentPage->findChild<QComboBox*>("statusComboBox")->currentText();
        break;
    case 2: // Học tập
        data["studyMethod"] = currentPage->findChild<QComboBox*>("studyMethodCombo")->currentText();
        data["subject"] = currentPage->findChild<QLineEdit*>("subjectEdit")->text();
        break;
    case 3: // Ngày lễ
        data["holidayScope"] = currentPage->findChild<QComboBox*>("holidayScopeCombo")->currentText();
        if (data["holidayScope"].toString() == "Tùy chỉnh") {
            data["customHolidayName"] = currentPage->findChild<QLineEdit*>("customHolidayEdit")->text();
        }
        break;
    case 4: // Cuộc hẹn
        data["location"] = currentPage->findChild<QLineEdit*>("locationEdit")->text();
        data["isPrivate"] = currentPage->findChild<QCheckBox*>("privateCheck")->isChecked();
        data["appointmentType"] = currentPage->findChild<QComboBox*>("appointmentTypeCombo")->currentText();
        if (data["appointmentType"].toString() == "Khác") {
            data["customAppointmentType"] = currentPage->findChild<QLineEdit*>("customAppointmentEdit")->text();
        }
        break;
    default: // Sự kiện (Không có dữ liệu thêm)
        break;
    }
    return data;
}

// =================================================================================
// === 4. HÀM TẠO TRANG (PRIVATE PAGE CREATION HELPERS)
// =================================================================================

/**
 * @brief Tạo trang widget cho "Chi tiết Cuộc họp".
 * @return Con trỏ QWidget chứa form.
 */
QWidget* EventDialog::createMeetingPage()
{
    QWidget *page = new QWidget;
    QGroupBox *meetingGroup = new QGroupBox("Chi tiết Cuộc họp");
    QFormLayout *layout = new QFormLayout(meetingGroup);

    QLineEdit *hostEdit = new QLineEdit;
    hostEdit->setObjectName("hostEdit"); // Đặt tên để findChild

    QLineEdit *participantsEdit = new QLineEdit;
    participantsEdit->setPlaceholderText("Phân cách bằng dấu phẩy (,)");
    participantsEdit->setObjectName("participantsEdit");

    QComboBox *statusComboBox = new QComboBox;
    statusComboBox->addItems({"Dự kiến", "Đã xác nhận", "Đã hủy"});
    statusComboBox->setObjectName("statusComboBox");

    layout->addRow("Người chủ trì:", hostEdit);
    layout->addRow("Người tham gia:", participantsEdit);
    layout->addRow("Trạng thái:", statusComboBox);

    QVBoxLayout *pageLayout = new QVBoxLayout(page);
    pageLayout->addWidget(meetingGroup);
    return page;
}

/**
 * @brief Tạo trang widget cho "Chi tiết Học tập".
 * @return Con trỏ QWidget chứa form.
 */
QWidget* EventDialog::createStudyPage()
{
    QWidget *page = new QWidget;
    QGroupBox *groupBox = new QGroupBox("Chi tiết Học tập");
    QFormLayout *layout = new QFormLayout(groupBox);

    QComboBox *methodCombo = new QComboBox;
    methodCombo->addItems({"Trực tiếp", "Trực tuyến", "Học thêm", "Tự học"});
    methodCombo->setObjectName("studyMethodCombo");

    QLineEdit *subjectEdit = new QLineEdit;
    subjectEdit->setPlaceholderText("Ví dụ: Giải tích 1");
    subjectEdit->setObjectName("subjectEdit");

    layout->addRow("Cách thức:", methodCombo);
    layout->addRow("Tên môn học:", subjectEdit);

    QVBoxLayout *pageLayout = new QVBoxLayout(page);
    pageLayout->addWidget(groupBox);
    return page;
}

/**
 * @brief Tạo trang widget cho "Chi tiết Ngày lễ".
 * @return Con trỏ QWidget chứa form.
 */
QWidget* EventDialog::createHolidayPage()
{
    QWidget *page = new QWidget;
    QGroupBox *groupBox = new QGroupBox("Chi tiết Ngày lễ");
    QFormLayout *layout = new QFormLayout(groupBox);

    QComboBox *scopeCombo = new QComboBox;
    scopeCombo->addItems({"Quốc tế", "Quốc gia", "Tôn giáo", "Tùy chỉnh"});
    scopeCombo->setObjectName("holidayScopeCombo");
    layout->addRow("Ảnh hưởng:", scopeCombo);

    // Trường "Tùy chỉnh" (ẩn ban đầu)
    QLineEdit *customHolidayEdit = new QLineEdit;
    customHolidayEdit->setPlaceholderText("Ví dụ: Tự thưởng...");
    customHolidayEdit->setObjectName("customHolidayEdit");
    layout->addRow("Tùy chỉnh:", customHolidayEdit);

    // Lấy con trỏ tới nhãn và trường vừa thêm
    int newRowIndex = layout->rowCount() - 1;
    QLayoutItem *labelItem = layout->itemAt(newRowIndex, QFormLayout::LabelRole);
    QLayoutItem *fieldItem = layout->itemAt(newRowIndex, QFormLayout::FieldRole);

    // Ẩn cả hai
    labelItem->widget()->setVisible(false);
    fieldItem->widget()->setVisible(false);

    // Kết nối tín hiệu: Khi chọn "Tùy chỉnh" -> Hiện
    connect(scopeCombo, &QComboBox::currentTextChanged, this, [labelItem, fieldItem](const QString &text) {
        bool visible = (text == "Tùy chỉnh");
        labelItem->widget()->setVisible(visible);
        fieldItem->widget()->setVisible(visible);
    });

    QVBoxLayout *pageLayout = new QVBoxLayout(page);
    pageLayout->addWidget(groupBox);
    return page;
}

/**
 * @brief Tạo trang widget cho "Chi tiết Cuộc hẹn".
 * @return Con trỏ QWidget chứa form.
 */
QWidget* EventDialog::createAppointmentPage()
{
    QWidget *page = new QWidget;
    QGroupBox *groupBox = new QGroupBox("Chi tiết Cuộc hẹn");
    QFormLayout *layout = new QFormLayout(groupBox);

    QLineEdit *locationEdit = new QLineEdit;
    locationEdit->setPlaceholderText("Ví dụ: Quán cà phê XYZ");
    locationEdit->setObjectName("locationEdit");

    QCheckBox *privateCheck = new QCheckBox("Đây là cuộc hẹn riêng tư");
    privateCheck->setObjectName("privateCheck");

    QComboBox *typeCombo = new QComboBox;
    typeCombo->addItems({"Hẹn hò", "Gặp mặt", "Học nhóm", "Đi chơi", "Khác"});
    typeCombo->setObjectName("appointmentTypeCombo");

    layout->addRow("Địa điểm:", locationEdit);
    layout->addRow("Kiểu:", typeCombo);

    // Trường "Tùy chỉnh" (ẩn ban đầu)
    QLineEdit *customAppointmentEdit = new QLineEdit;
    customAppointmentEdit->setPlaceholderText("Ví dụ: Phỏng vấn...");
    customAppointmentEdit->setObjectName("customAppointmentEdit");
    layout->addRow("Tùy chỉnh:", customAppointmentEdit);

    // Lấy con trỏ tới nhãn và trường
    int newRowIndex = layout->rowCount() - 1;
    QLayoutItem *labelItem = layout->itemAt(newRowIndex, QFormLayout::LabelRole);
    QLayoutItem *fieldItem = layout->itemAt(newRowIndex, QFormLayout::FieldRole);
    labelItem->widget()->setVisible(false);
    fieldItem->widget()->setVisible(false);

    // Kết nối tín hiệu: Khi chọn "Khác" -> Hiện
    connect(typeCombo, &QComboBox::currentTextChanged, this, [labelItem, fieldItem](const QString &text) {
        bool visible = (text == "Khác");
        labelItem->widget()->setVisible(visible);
        fieldItem->widget()->setVisible(visible);
    });

    layout->addRow("", privateCheck);

    QVBoxLayout *pageLayout = new QVBoxLayout(page);
    pageLayout->addWidget(groupBox);
    return page;
}

// =================================================================================
// === 5. SLOT NỘI BỘ (PRIVATE SLOTS)
// =================================================================================

/**
 * @brief Được gọi khi nhấn nút "Lưu".
 * Thực hiện validation (kiểm tra dữ liệu) trước khi chấp nhận.
 */
void EventDialog::onSaveClicked()
{
    // --- Validation (Kiểm tra dữ liệu) ---
    if (m_titleEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Thiếu thông tin", "Vui lòng nhập tiêu đề cho sự kiện.");
        return;
    }
    if (!m_allDayCheckBox->isChecked() && startDateTime() >= endDateTime()) {
        QMessageBox::warning(this, "Thời gian không hợp lệ", "Thời gian kết thúc phải sau thời gian bắt đầu.");
        return;
    }
    if (m_recurrenceCheckBox->isChecked()) {
        bool oneDayChecked = false;
        for (QCheckBox *cb : m_weekdayCheckBoxes) {
            if (cb->isChecked()) { oneDayChecked = true; break; }
        }
        if (!oneDayChecked) {
            QMessageBox::warning(this, "Lặp lại không hợp lệ", "Vui lòng chọn ít nhất một ngày trong tuần để lặp lại.");
            return;
        }
        if (m_recurrenceEndDateEdit->date() <= m_startDateEdit->date()) {
            QMessageBox::warning(this, "Lặp lại không hợp lệ", "Ngày kết thúc lặp lại phải sau ngày bắt đầu sự kiện.");
            return;
        }
    }
    // ------------------------------------

    // Nếu mọi thứ hợp lệ
    m_editResult = EditResult::Save; // Đặt cờ kết quả
    accept(); // Đóng dialog với kết quả QDialog::Accepted
}

/**
 * @brief Được gọi khi nhấn nút "Xóa".
 */
void EventDialog::onDeleteClicked()
{
    auto reply = QMessageBox::warning(this, "Xác nhận xóa",
                                      "Bạn có chắc chắn muốn xóa sự kiện này?",
                                      QMessageBox::Yes | QMessageBox::No,
                                      QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        m_editResult = EditResult::Delete; // Đặt cờ kết quả
        accept(); // Đóng dialog
    }
}

/**
 * @brief Được gọi khi tick/bỏ tick "Cả ngày".
 * Vô hiệu hóa/Kích hoạt ComboBox chọn giờ.
 */
void EventDialog::onAllDayToggled(bool checked)
{
    m_startTimeComboBox->setDisabled(checked);
    m_endTimeComboBox->setDisabled(checked);
}

/**
 * @brief Được gọi khi nhấn nút chọn màu.
 * Mở QColorDialog, sau đó cập nhật ngược lại m_categoryComboBox.
 */
void EventDialog::onColorButtonClicked()
{
    QColor color = QColorDialog::getColor(m_selectedColor, this, "Chọn màu cho sự kiện");
    if (color.isValid()) {
        m_selectedColor = color; // Cập nhật màu "thật"
        updateColorButton(color);

        // Logic "Ngược": Cập nhật ComboBox dựa trên màu
        QString matchingCategory;
        for (auto it = m_categories.constBegin(); it != m_categories.constEnd(); ++it) {
            if (it.value() == m_selectedColor) {
                matchingCategory = it.key();
                break;
            }
        }

        if (!matchingCategory.isEmpty()) {
            m_categoryComboBox->setCurrentText(matchingCategory); // Khớp (ví dụ: "Đỏ")
        } else {
            m_categoryComboBox->setCurrentText("Không"); // Màu tùy chỉnh
        }
    }
}

/**
 * @brief Được gọi khi ComboBox Thẻ/Tag (Category) thay đổi.
 * Cập nhật Nút màu theo màu mặc định của Tag.
 */
void EventDialog::onCategoryChanged(const QString &categoryName)
{
    m_selectedColor = m_categories.value(categoryName); // Lấy màu từ Map
    updateColorButton(m_selectedColor); // Cập nhật nút màu
}

/**
 * @brief Được gọi khi ComboBox "Loại sự kiện" thay đổi.
 * Tự động chọn một Thẻ/Tag (và màu) mặc định cho phù hợp.
 */
void EventDialog::onEventTypeChanged(const QString &text)
{
    if (text == "Cuộc họp") {
        m_categoryComboBox->setCurrentText("Xanh dương");
    } else if (text == "Học tập") {
        m_categoryComboBox->setCurrentText("Vàng");
    } else if (text == "Ngày lễ") {
        m_categoryComboBox->setCurrentText("Xanh lá");
    } else if (text == "Cuộc hẹn") {
        m_categoryComboBox->setCurrentText("Đỏ");
    } else { // "Sự kiện"
        m_categoryComboBox->setCurrentText("Không");
    }
    // Slot 'onCategoryChanged' sẽ tự động được gọi, cập nhật nút màu
}

/**
 * @brief Tự động cập nhật giờ kết thúc (chưa được kết nối trong setupUi).
 */
void EventDialog::onStartTimeChanged(const QString &text)
{
    QTime startTime = QTime::fromString(text, "HH:mm");
    if (startTime.isValid()) {
        QTime endTime = startTime.addSecs(3600); // Tự động +1 giờ
        m_endTimeComboBox->setCurrentText(endTime.toString("HH:mm"));
    }
}

// =================================================================================
// === 6. HÀM TRỢ GIÚP NỘI BỘ (PRIVATE HELPERS)
// =================================================================================

/**
 * @brief Đổ danh sách các mốc thời gian (cách nhau 30 phút) vào ComboBox.
 */
void EventDialog::populateTimeComboBox(QComboBox *comboBox)
{
    for (int hour = 0; hour < 24; ++hour) {
        for (int minute = 0; minute < 60; minute += 30) {
            QTime time(hour, minute);
            comboBox->addItem(time.toString("HH:mm"));
        }
    }
}

/**
 * @brief Cập nhật QSS cho nút chọn màu (m_colorButton).
 */
void EventDialog::updateColorButton(const QColor &color)
{
    // Tính toán màu chữ (đen hoặc trắng) để tương phản
    QColor textColor = (color.redF() * 0.299 + color.greenF() * 0.587 + color.blueF() * 0.114) > 0.5
                           ? QColor(0,0,0) : QColor(255,255,255);

    QString style = QString("background-color: %1; color: %2; border: 1px solid %3;")
                        .arg(color.name())
                        .arg(textColor.name())
                        .arg(color.darker(120).name());

    m_colorButton->setStyleSheet(style);
    m_colorButton->setText(color.name().toUpper()); // Hiển thị mã hex
}
