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
#include <QTimeZone>
#include <QDebug>
#include <QCalendarWidget>
#include <QStackedWidget>
#include <QRadioButton>
#include <QSignalBlocker>

EventDialog::EventDialog(QWidget *parent)
    : QDialog(parent)
    , m_timezoneOffsetSeconds(QDateTime::currentDateTime().offsetFromUtc())
{
    // Khởi tạo dữ liệu cho danh mục (ĐỒNG BỘ VỚI FILTER MENU)
    m_categories["Không"] = Qt::gray;
    m_categories["Đỏ"]    = QColor("#ff605d"); // (Màu đỏ)
    m_categories["Cam"] = QColor("#ffa74f"); // (Màu cam)
    m_categories["Vàng"] = QColor("#ffff7f"); // (Màu vàng)
    m_categories["Xanh lá"]  = QColor("#8cbb63"); // (Màu xanh lá)
    m_categories["Xanh dương"]   = QColor("#c8e2f1"); // (Màu xanh dương)
    m_categories["Tím"] = QColor("#e3dced"); // (Màu tím)

    // Đặt màu mặc định là "Không"
    m_selectedColor = m_categories.value("Không");

    setupUi();
    setWindowTitle("Tạo/Chỉnh sửa sự kiện");
    setMinimumWidth(450);
}

void EventDialog::setTimezoneOffset(int offsetSeconds)
{
    m_timezoneOffsetSeconds = offsetSeconds;
}

void EventDialog::setupUi()
{
    // 1. Khởi tạo các widget
    m_titleEdit = new QLineEdit;
    m_allDayCheckBox = new QCheckBox("Cả ngày");

    // -- MỚI: Hiển thị như --
    m_showAsComboBox = new QComboBox;
    m_showAsComboBox->addItem("Bận"); // "Bận" (Busy) là tùy chọn hợp lý nhất làm mặc định
    m_showAsComboBox->addItem("Rảnh");
    m_showAsComboBox->addItem("Làm việc ở nơi khác");
    m_showAsComboBox->addItem("Dự định");
    m_showAsComboBox->addItem("Vắng mặt");

    // -- Thời gian bắt đầu --
    m_startDateEdit = new QDateEdit(QDate::currentDate());
    m_startDateEdit->setCalendarPopup(true);
    m_startDateEdit->setDisplayFormat("dd/MM/yyyy");
    m_startTimeComboBox = new QComboBox;
    populateTimeComboBox(m_startTimeComboBox);
    m_startTimeComboBox->setEditable(true);

    // -- Thời gian kết thúc --
    m_endDateEdit = new QDateEdit(QDate::currentDate());
    m_endDateEdit->setCalendarPopup(true);
    m_endDateEdit->setDisplayFormat("dd/MM/yyyy");
    m_endTimeComboBox = new QComboBox;
    populateTimeComboBox(m_endTimeComboBox);
    m_endTimeComboBox->setEditable(true);

    // -- Danh mục --
    m_categoryComboBox = new QComboBox;
    // THÊM CÁC DÒNG NÀY ĐỂ ĐẢM BẢO ĐÚNG THỨ TỰ
    m_categoryComboBox->addItem("Không");
    m_categoryComboBox->addItem("Đỏ");
    m_categoryComboBox->addItem("Cam");
    m_categoryComboBox->addItem("Vàng");
    m_categoryComboBox->addItem("Xanh lá");
    m_categoryComboBox->addItem("Xanh dương");
    m_categoryComboBox->addItem("Tím");

    // -- MỚI: Nút chọn màu --
    m_colorButton = new QPushButton;
    m_colorButton->setObjectName("colorButton");
    m_colorButton->setCursor(Qt::PointingHandCursor);
    updateColorButton(m_selectedColor); // Cập nhật màu ban đầu

    // -- MỚI: Lặp lại sự kiện --
    m_recurrenceCheckBox = new QCheckBox("Lặp lại sự kiện");
    m_recurrenceWidget = new QWidget;
    m_recurrenceWidget->setObjectName("recurrenceWidget"); // Để style QSS
    QFormLayout *recurrenceLayout = new QFormLayout(m_recurrenceWidget);
    recurrenceLayout->setContentsMargins(0, 9, 0, 0);

    QHBoxLayout *weekdaysLayout = new QHBoxLayout;
    weekdaysLayout->setSpacing(5);
    QStringList weekdays = {"T2", "T3", "T4", "T5", "T6", "T7", "CN"};
    for (const QString &day : weekdays) {
        QCheckBox *cb = new QCheckBox(day);
        weekdaysLayout->addWidget(cb);
        m_weekdayCheckBoxes.append(cb);
    }
    weekdaysLayout->addStretch();

    m_recurrenceEndDateEdit = new QDateEdit(QDate::currentDate().addMonths(1));
    m_recurrenceEndDateEdit->setCalendarPopup(true);
    m_recurrenceEndDateEdit->setDisplayFormat("dd/MM/yyyy");
    m_recurrenceEndDateEdit->setMinimumDate(QDate::currentDate().addDays(1));

    // Tạo một hàm (lambda) trợ giúp để áp dụng cài đặt
    auto configureCalendar = [](QDateEdit* dateEdit) {
        if (dateEdit->calendarPopup()) {
            QCalendarWidget* popupCalendar = dateEdit->calendarWidget();
            if (popupCalendar) {
                // 1. Đổi sang lịch Tiếng Việt
                popupCalendar->setLocale(QLocale(QLocale::Vietnamese));
                // 2. Đặt Thứ 2 là ngày đầu tuần (Chủ Nhật ở cuối)
                popupCalendar->setFirstDayOfWeek(Qt::Monday);
            }
        }
    };

    // Áp dụng cho cả 3 lịch
    configureCalendar(m_startDateEdit);
    configureCalendar(m_endDateEdit);
    configureCalendar(m_recurrenceEndDateEdit);

    recurrenceLayout->addRow("Lặp vào các thứ:", weekdaysLayout);
    recurrenceLayout->addRow("Cho đến ngày:", m_recurrenceEndDateEdit);
    m_recurrenceWidget->setVisible(false); // Ẩn đi lúc đầu

    // -- MỚI: Mô tả --
    m_descriptionEdit = new QTextEdit;
    m_descriptionEdit->setPlaceholderText("Thêm mô tả chi tiết cho sự kiện...");
    m_descriptionEdit->setMinimumHeight(80);

    // === THÊM MỚI: LOẠI SỰ KIỆN VÀ STACKED WIDGET ===
    m_eventTypeComboBox = new QComboBox;
    m_eventTypeComboBox->addItem("Sự kiện");        // Index 0
    m_eventTypeComboBox->addItem("Cuộc họp");       // Index 1
    m_eventTypeComboBox->addItem("Học tập");         // Index 2
    m_eventTypeComboBox->addItem("Ngày lễ");         // Index 3
    m_eventTypeComboBox->addItem("Cuộc hẹn");       // Index 4

    m_extraDataStack = new QStackedWidget;
    m_extraDataStack->addWidget(new QWidget); // Index 0: Trang trống cho "Sự kiện"
    m_extraDataStack->addWidget(createMeetingPage());   // Index 1
    m_extraDataStack->addWidget(createStudyPage());     // Index 2
    m_extraDataStack->addWidget(createHolidayPage());    // Index 3
    m_extraDataStack->addWidget(createAppointmentPage());// Index 4

    // -- Nút OK/Cancel --
    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    // 1. Thêm nút "Xóa"
    m_deleteButton = m_buttonBox->addButton("Xóa", QDialogButtonBox::DestructiveRole);

    this->setObjectName("EventDialog");
    m_buttonBox->button(QDialogButtonBox::Ok)->setText("Lưu");
    m_buttonBox->button(QDialogButtonBox::Ok)->setObjectName("okButton");
    m_buttonBox->button(QDialogButtonBox::Cancel)->setObjectName("cancelButton");
    m_deleteButton->setObjectName("deleteButton");

    m_deleteButton->setVisible(false);

    this->setObjectName("EventDialog");
    m_buttonBox->button(QDialogButtonBox::Ok)->setObjectName("okButton");
    m_buttonBox->button(QDialogButtonBox::Cancel)->setObjectName("cancelButton");

    // 2. Thiết lập giá trị mặc định thông minh
    QTime now = QTime::currentTime();
    int minutes = (now.hour() * 60 + now.minute() + 15) / 30 * 30;
    QTime startTime = QTime(minutes / 60, minutes % 60);
    m_startTimeComboBox->setCurrentText(startTime.toString("HH:mm"));
    m_endTimeComboBox->setCurrentText(startTime.addSecs(3600).toString("HH:mm"));

    // 3. Sắp xếp layout
    QHBoxLayout *startLayout = new QHBoxLayout;
    startLayout->addWidget(m_startDateEdit);
    startLayout->addWidget(m_startTimeComboBox);

    QHBoxLayout *endLayout = new QHBoxLayout;
    endLayout->addWidget(m_endDateEdit);
    endLayout->addWidget(m_endTimeComboBox);

    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow("Tiêu đề:", m_titleEdit);
    formLayout->addRow("Loại sự kiện:", m_eventTypeComboBox);
    formLayout->addRow("Bắt đầu:", startLayout);
    formLayout->addRow("Kết thúc:", endLayout);
    formLayout->addRow("", m_allDayCheckBox);
    formLayout->addRow("Trạng thái:", m_showAsComboBox);
    formLayout->addRow("Thẻ/Tag:", m_categoryComboBox);
    formLayout->addRow("Màu sắc:", m_colorButton); // <-- Mới
    formLayout->addRow("", m_recurrenceCheckBox); // <-- Mới
    formLayout->addRow(m_recurrenceWidget); // <-- Mới
    formLayout->addRow("Mô tả:", m_descriptionEdit); // <-- Mới

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(m_extraDataStack);
    mainLayout->addWidget(m_buttonBox);
    setLayout(mainLayout);

    // 4. Kết nối tín hiệu (signals/slots)
    connect(m_allDayCheckBox, &QCheckBox::toggled, this, &EventDialog::onAllDayToggled);
    connect(m_colorButton, &QPushButton::clicked, this, &EventDialog::onColorButtonClicked);
    connect(m_categoryComboBox, &QComboBox::currentTextChanged, this, &EventDialog::onCategoryChanged);
    connect(m_recurrenceCheckBox, &QCheckBox::toggled, m_recurrenceWidget, &QWidget::setVisible);

    connect(m_startDateEdit, &QDateEdit::dateChanged, this, [this](const QDate &date){
        m_endDateEdit->setMinimumDate(date);
        if (m_endDateEdit->date() < date) {
            m_endDateEdit->setDate(date);
        }
        m_recurrenceEndDateEdit->setMinimumDate(date.addDays(1));
        if (m_recurrenceEndDateEdit->date() <= date) {
            m_recurrenceEndDateEdit->setDate(date.addMonths(1));
        }
    });

    connect(m_buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked, this, &EventDialog::onSaveClicked);
    connect(m_deleteButton, &QPushButton::clicked, this, &EventDialog::onDeleteClicked);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(m_eventTypeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            m_extraDataStack, &QStackedWidget::setCurrentIndex);
    connect(m_eventTypeComboBox, &QComboBox::currentTextChanged, this, &EventDialog::onEventTypeChanged);
}

// Hàm đổ dữ liệu giờ vào ComboBox
void EventDialog::populateTimeComboBox(QComboBox *comboBox)
{
    for (int hour = 0; hour < 24; ++hour) {
        for (int minute = 0; minute < 60; minute += 30) {
            QTime time(hour, minute);
            comboBox->addItem(time.toString("HH:mm"));
        }
    }
}

// Slot xử lý khi checkbox "Cả ngày" được nhấn
void EventDialog::onAllDayToggled(bool checked)
{
    m_startTimeComboBox->setDisabled(checked);
    m_endTimeComboBox->setDisabled(checked);
}

// Slot xử lý tự động cập nhật giờ kết thúc
void EventDialog::onStartTimeChanged(const QString &text)
{
    QTime startTime = QTime::fromString(text, "HH:mm");
    if (startTime.isValid()) {
        QTime endTime = startTime.addSecs(3600); // Tự động +1 giờ
        m_endTimeComboBox->setCurrentText(endTime.toString("HH:mm"));
    }
}

// --- CÁC HÀM MỚI ---

// Slot khi nhấn nút chọn màu
void EventDialog::onColorButtonClicked()
{
    QColor color = QColorDialog::getColor(m_selectedColor, this, "Chọn màu cho sự kiện");
    if (color.isValid()) {
        m_selectedColor = color;
        updateColorButton(color);

        // === THÊM MỚI TẠI ĐÂY ===
        // Tự động cập nhật ComboBox dựa trên màu vừa chọn

        // 1. Tìm xem màu này có trong danh sách category chuẩn không
        QString matchingCategory;
        for (auto it = m_categories.constBegin(); it != m_categories.constEnd(); ++it) {
            if (it.value() == m_selectedColor) {
                matchingCategory = it.key();
                break;
            }
        }

        // 2. Cập nhật ComboBox
        if (!matchingCategory.isEmpty()) {
            // Nếu khớp (ví dụ: chọn màu đỏ), đặt ComboBox thành "Đỏ"
            m_categoryComboBox->setCurrentText(matchingCategory);
        } else {
            // Nếu không khớp (màu tùy chỉnh), đặt ComboBox về "Không"
            m_categoryComboBox->setCurrentText("Không");
        }
        // === KẾT THÚC THÊM MỚI ===
    }
}

// Slot khi danh mục thay đổi (để cập nhật màu mặc định)
void EventDialog::onCategoryChanged(const QString &categoryName)
{
    m_selectedColor = m_categories.value(categoryName);
    updateColorButton(m_selectedColor);
}

// Helper: Cập nhật giao diện nút màu
void EventDialog::updateColorButton(const QColor &color)
{
    // Tính toán màu chữ (đen hoặc trắng) để tương phản với màu nền
    QColor textColor = (color.redF() * 0.299 + color.greenF() * 0.587 + color.blueF() * 0.114) > 0.5
                           ? QColor(0,0,0) : QColor(255,255,255);

    QString style = QString("background-color: %1; color: %2; border: 1px solid %3;")
                        .arg(color.name())
                        .arg(textColor.name())
                        .arg(color.darker(120).name());

    m_colorButton->setStyleSheet(style);
    m_colorButton->setText(color.name().toUpper()); // Hiển thị mã hex
}


// --- CÁC HÀM GETTER ĐÃ CẬP NHẬT ---

QString EventDialog::title() const {
    return m_titleEdit->text();
}

QString EventDialog::description() const {
    return m_descriptionEdit->toPlainText();
}

QDateTime EventDialog::startDateTime() const
{
    QDate date = m_startDateEdit->date();
    if (m_allDayCheckBox->isChecked()) {
        // --- SỬA LẠI: Trả về QDateTime có offset ---
        return QDateTime(date, QTime(0, 0, 0), QTimeZone(m_timezoneOffsetSeconds));
    }
    QTime time = QTime::fromString(m_startTimeComboBox->currentText(), "HH:mm");
    // --- SỬA LẠI: Trả về QDateTime có offset ---
    return QDateTime(date, time, QTimeZone(m_timezoneOffsetSeconds));
}

QDateTime EventDialog::endDateTime() const
{
    QDate date = m_endDateEdit->date();
    if (m_allDayCheckBox->isChecked()) {
        // --- SỬA LẠI: Trả về QDateTime có offset ---
        return QDateTime(date, QTime(23, 59, 59), QTimeZone(m_timezoneOffsetSeconds));
    }
    QTime time = QTime::fromString(m_endTimeComboBox->currentText(), "HH:mm");
    // --- SỬA LẠI: Trả về QDateTime có offset ---
    return QDateTime(date, time, QTimeZone(m_timezoneOffsetSeconds));
}

QColor EventDialog::eventColor() const {
    return m_selectedColor; // Trả về màu đã chọn
}

bool EventDialog::isAllDay() const {
    return m_allDayCheckBox->isChecked();
}

EventDialog::RecurrenceRule EventDialog::recurrenceRule() const {
    RecurrenceRule rule;
    rule.isRecurrent = m_recurrenceCheckBox->isChecked();

    if (!rule.isRecurrent) {
        return rule; // Trả về quy tắc rỗng
    }

    rule.endDate = m_recurrenceEndDateEdit->date();

    // Qt::DayOfWeek: Monday=1, ..., Sunday=7
    // Index của m_weekdayCheckBoxes: 0=T2, 1=T3, ..., 6=CN
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

EventDialog::EditResult EventDialog::getEditResult() const
{
    return m_editResult;
}

QString EventDialog::category() const {
    // === BẮT ĐẦU SỬA LỖI ===

    // 1. Kiểm tra xem màu người dùng chọn (m_selectedColor)
    //    có khớp với một trong các màu category chuẩn không.
    // (m_categories là QMap<QString, QColor> đã định nghĩa ở constructor)
    for (auto it = m_categories.constBegin(); it != m_categories.constEnd(); ++it) {
        // it.key() là tên (ví dụ: "Đỏ")
        // it.value() là màu (ví dụ: QColor("#d9534f"))
        if (it.value() == m_selectedColor) {
            // Nếu khớp (ví dụ: người dùng chọn đúng màu đỏ),
            // trả về tên category đó.
            return it.key();
        }
    }

    // 2. Nếu không khớp (ví dụ: người dùng chọn màu tùy chỉnh #AA00FF),
    //    chúng ta vẫn trả về "Không".
    return "Không";

    // (XÓA DÒNG CŨ NÀY)
    // return m_categoryComboBox->currentText();
    // === KẾT THÚC SỬA LỖI ===
}

// MỚI: Hàm điền dữ liệu vào dialog (chế độ chỉnh sửa)
void EventDialog::setEventData(const QString &title, const QDateTime &start, const QDateTime &end,
                               const QColor &color, const QString &description,
                               const QString &showAs, const QString &category,
                               bool isAllDay, const RecurrenceRule &rule,
                               // Tham số mới
                               const QString &eventType,
                               const QJsonObject &extraData)
{
    m_deleteButton->setVisible(true);

    // 1. Điền thông tin cơ bản (Phần này an toàn)
    m_titleEdit->setText(title);
    m_descriptionEdit->setText(description);

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

    m_recurrenceCheckBox->setChecked(rule.isRecurrent);
    m_recurrenceWidget->setVisible(rule.isRecurrent);

    if (rule.isRecurrent) {
        m_recurrenceEndDateEdit->setDate(rule.endDate);
        for (int i = 0; i < m_weekdayCheckBoxes.size(); ++i) {
            Qt::DayOfWeek day = static_cast<Qt::DayOfWeek>(i + 1);
            m_weekdayCheckBoxes[i]->setChecked(rule.days.contains(day));
        }
    }

    // === BẮT ĐẦU SỬA LỖI CRASH ===

    // 2. Điền dữ liệu bổ sung
    // để việc set "Loại sự kiện" không ghi đè "Tag" đã chọn
    {
        QSignalBlocker blocker(m_eventTypeComboBox);
        m_eventTypeComboBox->setCurrentText(eventType);
    }

    int index = m_eventTypeComboBox->currentIndex();
    m_extraDataStack->setCurrentIndex(index);
    QWidget *currentPage = m_extraDataStack->widget(index);

    if (!currentPage) {
        qWarning() << "Trang không hợp lệ, index:" << index;
        return; // Thoát an toàn
    }

    // !!! Chúng ta KHÔNG lấy 'layout' ở đây nữa !!!

    switch (index) {
    case 0: // Sự kiện
        // Không làm gì cả, trang này trống.
        break;

    case 1: // Cuộc họp
        // Code này an toàn, không cần layout
        currentPage->findChild<QLineEdit*>("hostEdit")->setText(extraData["host"].toString());
        currentPage->findChild<QLineEdit*>("participantsEdit")->setText(extraData["participants"].toString());
        currentPage->findChild<QComboBox*>("statusComboBox")->setCurrentText(extraData["meetingStatus"].toString());
        break;

    case 2: // Học tập
        // Code này an toàn, không cần layout
        currentPage->findChild<QComboBox*>("studyMethodCombo")->setCurrentText(extraData["studyMethod"].toString());
        currentPage->findChild<QLineEdit*>("subjectEdit")->setText(extraData["subject"].toString());
        break;

    case 3: // Ngày lễ
    {
            // 3. LẤY LAYOUT (An toàn, bên trong 'case')
        QFormLayout* layout = qobject_cast<QFormLayout*>(currentPage->findChild<QGroupBox*>()->layout());
        if (!layout) break; // Thoát nếu không tìm thấy layout

        QString scope = extraData["holidayScope"].toString("Quốc tế");
        currentPage->findChild<QComboBox*>("holidayScopeCombo")->setCurrentText(scope);

        QLineEdit* customEdit = currentPage->findChild<QLineEdit*>("customHolidayEdit");
        QWidget* label = layout->labelForField(customEdit); // An toàn vì 'layout' đã được kiểm tra

        bool visible = (scope == "Tùy chỉnh");
        customEdit->setVisible(visible);
        if (label) label->setVisible(visible);

        if (visible) {
            customEdit->setText(extraData["customHolidayName"].toString());
        }
    }
    break;

    case 4: // Cuộc hẹn
    {
            // 3. LẤY LAYOUT (An toàn, bên trong 'case')
        QFormLayout* layout = qobject_cast<QFormLayout*>(currentPage->findChild<QGroupBox*>()->layout());
        if (!layout) break; // Thoát nếu không tìm thấy

        currentPage->findChild<QLineEdit*>("locationEdit")->setText(extraData["location"].toString());
        currentPage->findChild<QCheckBox*>("privateCheck")->setChecked(extraData["isPrivate"].toBool());
        QString type = extraData["appointmentType"].toString("Khác");
        currentPage->findChild<QComboBox*>("appointmentTypeCombo")->setCurrentText(type);

        QLineEdit* customEdit = currentPage->findChild<QLineEdit*>("customAppointmentEdit");
        QWidget* label = layout->labelForField(customEdit); // An toàn

        bool visible = (type == "Khác");
        customEdit->setVisible(visible);
        if (label) label->setVisible(visible);

        if (visible) {
            customEdit->setText(extraData["customAppointmentType"].toString());
        }
    }
    break;

    default:
        break;
    }
    // === KẾT THÚC SỬA LỖI CRASH ===
}

// Slot khi nhấn "Lưu"
void EventDialog::onSaveClicked()
{
    // --- Lấy validation logic từ kết nối 'accepted' cũ ---
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
            if (cb->isChecked()) {
                oneDayChecked = true;
                break;
            }
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
    // --------------------------------------------------

    // Nếu mọi thứ hợp lệ
    m_editResult = EditResult::Save;
    accept(); // Đóng dialog với kết quả QDialog::Accepted
}

// Slot khi nhấn "Xóa"
void EventDialog::onDeleteClicked()
{
    // Hỏi xác nhận trước khi xóa
    auto reply = QMessageBox::warning(this, "Xác nhận xóa",
                                      "Bạn có chắc chắn muốn xóa sự kiện này?",
                                      QMessageBox::Yes | QMessageBox::No,
                                      QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        m_editResult = EditResult::Delete;
        accept(); // Đóng dialog với kết quả QDialog::Accepted
    }
    // Nếu người dùng chọn "No", không làm gì cả, dialog vẫn mở
}

void EventDialog::setNewStartDateTime(const QDateTime &start)
{
    // Chuyển sang múi giờ hiển thị
    QDateTime displayStart = start.toOffsetFromUtc(m_timezoneOffsetSeconds);

    if (m_startDateEdit) {
        m_startDateEdit->setDate(displayStart.date());
    }
    if (m_startTimeComboBox) {
        m_startTimeComboBox->setCurrentText(displayStart.toString("HH:mm"));
    }
}

void EventDialog::setNewEndDateTime(const QDateTime &end)
{
    // Chuyển sang múi giờ hiển thị
    QDateTime displayEnd = end.toOffsetFromUtc(m_timezoneOffsetSeconds);

    if (m_endDateEdit) {
        m_endDateEdit->setDate(displayEnd.date());
    }
    if (m_endTimeComboBox) {
        m_endTimeComboBox->setCurrentText(displayEnd.toString("HH:mm"));
    }
}

void EventDialog::setNewRecurrenceRule(const EventDialog::RecurrenceRule &rule)
{
    // Kiểm tra để đảm bảo các widget đã được tạo
    if (!m_recurrenceCheckBox || !m_recurrenceWidget || !m_recurrenceEndDateEdit) {
        return;
    }

    // 1. Cập nhật Checkbox
    m_recurrenceCheckBox->setChecked(rule.isRecurrent);
    m_recurrenceWidget->setVisible(rule.isRecurrent);

    if (rule.isRecurrent) {
        // 2. Cập nhật ngày kết thúc
        m_recurrenceEndDateEdit->setDate(rule.endDate);

        // 3. Cập nhật các ngày trong tuần
        for (int i = 0; i < m_weekdayCheckBoxes.size(); ++i) {
            Qt::DayOfWeek day = static_cast<Qt::DayOfWeek>(i + 1);
            m_weekdayCheckBoxes[i]->setChecked(rule.days.contains(day));
        }
    }
}

QString EventDialog::getEventType() const
{
    return m_eventTypeComboBox->currentText();
}

void EventDialog::setEventType(const QString &eventType)
{
    m_eventTypeComboBox->setCurrentText(eventType);
    // Logic tự động chọn tag đã được chuyển sang slot onEventTypeChanged
}

// Hàm này tạo trang "Cuộc họp" (Giống hệt code bạn đã có)
QWidget* EventDialog::createMeetingPage()
{
    QWidget *page = new QWidget;
    QGroupBox *meetingGroup = new QGroupBox("Chi tiết Cuộc họp");
    QFormLayout *layout = new QFormLayout(meetingGroup);

    QLineEdit *hostEdit = new QLineEdit;
    hostEdit->setObjectName("hostEdit"); // Quan trọng: Đặt tên để tìm

    QLineEdit *participantsEdit = new QLineEdit;
    participantsEdit->setPlaceholderText("Phân cách bằng dấu phẩy (,)");
    participantsEdit->setObjectName("participantsEdit");

    QComboBox *statusComboBox = new QComboBox;
    statusComboBox->addItem("Dự kiến");
    statusComboBox->addItem("Đã xác nhận");
    statusComboBox->addItem("Đã hủy");
    statusComboBox->setObjectName("statusComboBox");

    layout->addRow("Người chủ trì:", hostEdit);
    layout->addRow("Người tham gia:", participantsEdit);
    layout->addRow("Trạng thái:", statusComboBox);

    QVBoxLayout *pageLayout = new QVBoxLayout(page);
    pageLayout->addWidget(meetingGroup);
    return page;
}

// Hàm này tạo trang "Học tập" (Mới)
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

// Hàm này tạo trang "Ngày lễ" (Mới)
QWidget* EventDialog::createHolidayPage()
{
    QWidget *page = new QWidget;
    QGroupBox *groupBox = new QGroupBox("Chi tiết Ngày lễ");
    QFormLayout *layout = new QFormLayout(groupBox);

    QComboBox *scopeCombo = new QComboBox;
    scopeCombo->addItems({"Quốc tế", "Quốc gia", "Tôn giáo", "Tùy chỉnh"});
    scopeCombo->setObjectName("holidayScopeCombo");

    layout->addRow("Ảnh hưởng:", scopeCombo);

    // === BẮT ĐẦU SỬA LỖI ===
    QLineEdit *customHolidayEdit = new QLineEdit;
    customHolidayEdit->setPlaceholderText("Ví dụ: Tự thưởng...");
    customHolidayEdit->setObjectName("customHolidayEdit");

    // 1. Thêm hàng mới. Hàm này trả về void.
    layout->addRow("Tùy chỉnh:", customHolidayEdit);

    // 2. Lấy index của hàng vừa thêm (hàng cuối cùng)
    int newRowIndex = layout->rowCount() - 1;

    // 3. Lấy con trỏ tới Label và Field bằng index
    QLayoutItem *labelItem = layout->itemAt(newRowIndex, QFormLayout::LabelRole);
    QLayoutItem *fieldItem = layout->itemAt(newRowIndex, QFormLayout::FieldRole);

    // 4. Ẩn cả hai
    labelItem->widget()->setVisible(false);
    fieldItem->widget()->setVisible(false);

    // 5. Kết nối tín hiệu (dùng con trỏ item đã lấy)
    connect(scopeCombo, &QComboBox::currentTextChanged, this, [labelItem, fieldItem](const QString &text) {
        bool visible = (text == "Tùy chỉnh");
        labelItem->widget()->setVisible(visible);
        fieldItem->widget()->setVisible(visible);
    });
    // === KẾT THÚC SỬA LỖI ===

    QVBoxLayout *pageLayout = new QVBoxLayout(page);
    pageLayout->addWidget(groupBox);
    return page;
}

// Hàm này tạo trang "Cuộc hẹn" (Mới)
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

    QLineEdit *customAppointmentEdit = new QLineEdit;
    customAppointmentEdit->setPlaceholderText("Ví dụ: Phỏng vấn...");
    customAppointmentEdit->setObjectName("customAppointmentEdit");

    // 1. Thêm hàng mới.
    layout->addRow("Tùy chỉnh:", customAppointmentEdit);

    // 2. Lấy index
    int newRowIndex = layout->rowCount() - 1;

    // 3. Lấy con trỏ
    QLayoutItem *labelItem = layout->itemAt(newRowIndex, QFormLayout::LabelRole);
    QLayoutItem *fieldItem = layout->itemAt(newRowIndex, QFormLayout::FieldRole);

    // 4. Ẩn
    labelItem->widget()->setVisible(false);
    fieldItem->widget()->setVisible(false);

    // 5. Kết nối (sửa thành "Khác")
    connect(typeCombo, &QComboBox::currentTextChanged, this, [labelItem, fieldItem](const QString &text) {
        bool visible = (text == "Khác");
        labelItem->widget()->setVisible(visible);
        fieldItem->widget()->setVisible(visible);
    });
    // === KẾT THÚC SỬA LỖI ===

    layout->addRow("", privateCheck);

    QVBoxLayout *pageLayout = new QVBoxLayout(page);
    pageLayout->addWidget(groupBox);
    return page;
}

QJsonObject EventDialog::getExtraData() const
{
    QJsonObject data;
    // Lấy trang hiện tại trong QStackedWidget
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
        // Chỉ lưu tên tùy chỉnh nếu chọn "Khác"
        if (data["appointmentType"].toString() == "Khác") {
            data["customAppointmentType"] = currentPage->findChild<QLineEdit*>("customAppointmentEdit")->text();
        }
        break;
    default: // Sự kiện (Không có dữ liệu thêm)
        break;
    }
    return data;
}

/**
 * @brief Slot này được gọi khi người dùng thay đổi
 * ComboBox "Loại sự kiện" trong dialog.
 */
void EventDialog::onEventTypeChanged(const QString &text)
{
    // TỰ ĐỘNG CHỌN TAG MẶC ĐỊNH DỰA TRÊN LOẠI SỰ KIỆN
    if (text == "Cuộc họp") {
        m_categoryComboBox->setCurrentText("Xanh dương");
    } else if (text == "Học tập") {
        m_categoryComboBox->setCurrentText("Vàng");
    } else if (text == "Ngày lễ") {
        m_categoryComboBox->setCurrentText("Xanh lá");
    } else if (text == "Cuộc hẹn") {
        m_categoryComboBox->setCurrentText("Đỏ");
    } else {
        // "Sự kiện" hoặc loại khác
        m_categoryComboBox->setCurrentText("Không");
    }
}
