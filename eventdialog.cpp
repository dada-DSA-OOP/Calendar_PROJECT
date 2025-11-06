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

EventDialog::EventDialog(QWidget *parent)
    : QDialog(parent)
{
    // Khởi tạo dữ liệu cho danh mục
    m_categories["Công việc"] = QColor("#3a87ad");
    m_categories["Cá nhân"] = QColor("#8cbb63");
    m_categories["Quan trọng"] = QColor("#e09445");
    m_categories["Sinh nhật"] = QColor("#af67de");
    m_categories["Chưa phân loại"] = Qt::gray;

    setupUi();
    setWindowTitle("Tạo/Chỉnh sửa sự kiện");
    setMinimumWidth(450);
}

void EventDialog::setupUi()
{
    // 1. Khởi tạo các widget
    m_titleEdit = new QLineEdit;
    m_allDayCheckBox = new QCheckBox("Cả ngày");

    // -- Thời gian bắt đầu --
    m_startDateEdit = new QDateEdit(QDate::currentDate());
    m_startDateEdit->setCalendarPopup(true);
    m_startDateEdit->setDisplayFormat("dd/MM/yyyy");
    m_startTimeComboBox = new QComboBox;
    populateTimeComboBox(m_startTimeComboBox); // Đổ dữ liệu giờ
    m_startTimeComboBox->setEditable(true);    // Cho phép nhập tay

    // -- Thời gian kết thúc --
    m_endDateEdit = new QDateEdit(QDate::currentDate());
    m_endDateEdit->setCalendarPopup(true);
    m_endDateEdit->setDisplayFormat("dd/MM/yyyy");
    m_endTimeComboBox = new QComboBox;
    populateTimeComboBox(m_endTimeComboBox);
    m_endTimeComboBox->setEditable(true);

    // -- Danh mục --
    m_categoryComboBox = new QComboBox;
    m_categoryComboBox->addItems(m_categories.keys());

    // -- Nút OK/Cancel --
    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    this->setObjectName("EventDialog"); // Đặt tên cho cả dialog
    m_buttonBox->button(QDialogButtonBox::Ok)->setObjectName("okButton");
    m_buttonBox->button(QDialogButtonBox::Cancel)->setObjectName("cancelButton");

    // 2. Thiết lập giá trị mặc định thông minh
    QTime now = QTime::currentTime();
    int minutes = (now.hour() * 60 + now.minute() + 15) / 30 * 30; // Làm tròn đến 30p gần nhất
    QTime startTime = QTime(minutes / 60, minutes % 60);
    m_startTimeComboBox->setCurrentText(startTime.toString("HH:mm"));
    m_endTimeComboBox->setCurrentText(startTime.addSecs(3600).toString("HH:mm")); // Mặc định 1 giờ sau


    // 3. Sắp xếp layout
    QHBoxLayout *startLayout = new QHBoxLayout;
    startLayout->addWidget(m_startDateEdit);
    startLayout->addWidget(m_startTimeComboBox);

    QHBoxLayout *endLayout = new QHBoxLayout;
    endLayout->addWidget(m_endDateEdit);
    endLayout->addWidget(m_endTimeComboBox);

    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow("Tiêu đề:", m_titleEdit);
    formLayout->addRow("Bắt đầu:", startLayout);
    formLayout->addRow("Kết thúc:", endLayout);
    formLayout->addRow("", m_allDayCheckBox);
    formLayout->addRow("Danh mục:", m_categoryComboBox);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(m_buttonBox);
    setLayout(mainLayout);

    // 4. Kết nối tín hiệu (signals/slots)
    connect(m_allDayCheckBox, &QCheckBox::toggled, this, &EventDialog::onAllDayToggled);
    connect(m_startTimeComboBox, &QComboBox::currentTextChanged, this, &EventDialog::onStartTimeChanged);
    connect(m_startDateEdit, &QDateEdit::dateChanged, this, [this](const QDate &date){
        m_endDateEdit->setMinimumDate(date); // Ngày kết thúc không được trước ngày bắt đầu
        if (m_endDateEdit->date() < date) {
            m_endDateEdit->setDate(date);
        }
    });

    connect(m_buttonBox, &QDialogButtonBox::accepted, this, [this](){
        if (m_titleEdit->text().isEmpty()) {
            QMessageBox::warning(this, "Thiếu thông tin", "Vui lòng nhập tiêu đề cho sự kiện.");
            return;
        }
        if (!m_allDayCheckBox->isChecked() && startDateTime() >= endDateTime()) {
            QMessageBox::warning(this, "Thời gian không hợp lệ", "Thời gian kết thúc phải sau thời gian bắt đầu.");
            return;
        }
        accept();
    });
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
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

// Các hàm getter để lấy dữ liệu
QString EventDialog::title() const {
    return m_titleEdit->text();
}

QDateTime EventDialog::startDateTime() const {
    QDate date = m_startDateEdit->date();
    if (m_allDayCheckBox->isChecked()) {
        return QDateTime(date, QTime(0, 0, 0));
    }
    QTime time = QTime::fromString(m_startTimeComboBox->currentText(), "HH:mm");
    return QDateTime(date, time);
}

QDateTime EventDialog::endDateTime() const {
    QDate date = m_endDateEdit->date();
    if (m_allDayCheckBox->isChecked()) {
        return QDateTime(date, QTime(23, 59, 59));
    }
    QTime time = QTime::fromString(m_endTimeComboBox->currentText(), "HH:mm");
    return QDateTime(date, time);
}

QColor EventDialog::categoryColor() const {
    return m_categories.value(m_categoryComboBox->currentText());
}
