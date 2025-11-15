#include "settingsdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QToolButton>
#include <QButtonGroup>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QGroupBox>
#include <QLabel>
#include <QMessageBox>
#include <QCheckBox>
#include <QColorDialog>
#include <QIcon>
#include <QComboBox>

// Hàm getPreviewStyle (Giữ nguyên, không thay đổi)
QString getPreviewStyle(const QColor &bgColor = QColor())
{
    QString style = "QToolButton {";
    style += "    border: 2px solid #ccc;"
             "    border-radius: 4px;";

    if (bgColor.isValid()) {
        style += QString("background-color: %1;").arg(bgColor.name());
    } else {
        style += "background-color: #f0f0f0;";
    }

    style += "}";
    style += "QToolButton:checked {"
             "    border: 3px solid #0078d7;"
             "}";
    return style;
}


SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent), m_selectedColor(Qt::white)
{
    setWindowTitle("Cài đặt");
    setMinimumWidth(550);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);

    // --- 1. KHUNG GIAO DIỆN ---
    // === THÊM MỚI: Cài đặt thời gian ===
    QGroupBox *timeGroup = new QGroupBox("Múi giờ hiển thị");
    QVBoxLayout *timeLayout = new QVBoxLayout(timeGroup);

    m_cmbTimezone = new QComboBox;
    m_cmbTimezone->setToolTip("Chọn múi giờ để hiển thị lịch. Mọi sự kiện vẫn được lưu bằng UTC.");

    m_chk24Hour = new QCheckBox("Sử dụng định dạng 24 giờ");
    m_chk24Hour->setToolTip("Nếu không chọn, sẽ dùng định dạng 12 giờ (ví dụ: 2:00 PM)");
    m_chk24Hour->setChecked(true); // Mặc định là 24h

    timeLayout->addWidget(m_chk24Hour); // Thêm checkbox vào layout

    // Tạo danh sách múi giờ (từ UTC-12 đến UTC+14)
    for (int h = -12; h <= 14; ++h) {
        // Xử lý các múi giờ 30 phút và 45 phút
        if (h == 3) { // +3:30
            int offsetSec = (h * 3600) + 1800;
            m_cmbTimezone->addItem(QString("(UTC+03:30) Tehran"), QVariant(offsetSec));
        } else if (h == 4) { // +4:30
            int offsetSec = (h * 3600) + 1800;
            m_cmbTimezone->addItem(QString("(UTC+04:30) Kabul"), QVariant(offsetSec));
        } else if (h == 5) { // +5:30, +5:45
            int offsetSec = (h * 3600) + 1800;
            m_cmbTimezone->addItem(QString("(UTC+05:30) Delhi, Kolkata"), QVariant(offsetSec));
            offsetSec = (h * 3600) + 2700;
            m_cmbTimezone->addItem(QString("(UTC+05:45) Kathmandu"), QVariant(offsetSec));
        } else if (h == 6) { // +6:30
            int offsetSec = (h * 3600) + 1800;
            m_cmbTimezone->addItem(QString("(UTC+06:30) Yangon"), QVariant(offsetSec));
        } else if (h == 8) { // +8:45
            int offsetSec = (h * 3600) + 2700;
            m_cmbTimezone->addItem(QString("(UTC+08:45) Eucla"), QVariant(offsetSec));
        } else if (h == 9) { // +9:30
            int offsetSec = (h * 3600) + 1800;
            m_cmbTimezone->addItem(QString("(UTC+09:30) Adelaide"), QVariant(offsetSec));
        } else if (h == 10) { // +10:30
            int offsetSec = (h * 3600) + 1800;
            m_cmbTimezone->addItem(QString("(UTC+10:30) Lord Howe Island"), QVariant(offsetSec));
        } else if (h == 12) { // +12:45
            int offsetSec = (h * 3600) + 2700;
            m_cmbTimezone->addItem(QString("(UTC+12:45) Chatham Islands"), QVariant(offsetSec));
        }

        // Thêm múi giờ tròn
        int offsetSec = h * 3600;
        QString sign = (h >= 0) ? "+" : "";
        QString hourStr = QString::number(h).rightJustified(2, '0');
        m_cmbTimezone->addItem(QString("(UTC%1%2:00)").arg(sign, hourStr), QVariant(offsetSec));

        // Tự động chọn múi giờ +7 (Việt Nam) làm ví dụ
        if (h == 7) {
            m_cmbTimezone->setItemText(m_cmbTimezone->count() - 1, "(UTC+07:00) Bangkok, Hanoi, Jakarta");
        }
    }

    timeLayout->addWidget(m_cmbTimezone);
    mainLayout->addWidget(timeGroup);

    QGroupBox *appearanceGroup = new QGroupBox("Giao diện");
    QVBoxLayout *appearanceLayout = new QVBoxLayout(appearanceGroup);

    m_backgroundGroup = new QButtonGroup(this);
    m_backgroundGroup->setExclusive(true);

    QGridLayout *previewLayout = new QGridLayout;
    previewLayout->setSpacing(10);

    // Danh sách này giờ có 13 ảnh (ID từ 0 đến 12)
    QList<QPair<QString, QString>> defaultBackgrounds = {
        {":/resource/images/background1.jpg", "Nền mặc định (Pink)"},
        {":/resource/images/background2.jpg", "Nền mặc định (Green)"},
        {":/resource/images/background3.jpg", "Nền mặc định (Blue)"},
        {":/resource/images/background4.jpg", "Nền mặc định (Yellow)"},
        {":/resource/images/background5.jpg", "Nền mặc định (5)"},
        {":/resource/images/background6.jpg", "Nền mặc định (6)"},
        {":/resource/images/background7.jpg", "Nền mặc định (7)"},
        {":/resource/images/background8.jpg", "Nền mặc định (8)"},
        {":/resource/images/background9.jpg", "Nền mặc định (9)"},
        {":/resource/images/background10.jpg", "Nền mặc định (10)"},
        {":/resource/images/background11.jpg", "Nền mặc định (11)"},
        {":/resource/images/background12.jpg", "Nền mặc định (12)"},
        {":/resource/images/background13.jpg", "Nền mặc định (13)"}
    };

    const int itemsPerRow = 5;
    const QSize buttonSize(90, 60);
    const QSize iconSize(buttonSize.width() - 6, buttonSize.height() - 6);

    for (int i = 0; i < defaultBackgrounds.count(); ++i) {
        QToolButton *btn = new QToolButton;
        btn->setFixedSize(buttonSize);
        btn->setCheckable(true);
        btn->setToolTip(defaultBackgrounds[i].second);
        btn->setIcon(QIcon(defaultBackgrounds[i].first));
        btn->setIconSize(iconSize);
        btn->setStyleSheet(getPreviewStyle());

        m_backgroundGroup->addButton(btn, i); // Gán ID từ 0 đến 12
        previewLayout->addWidget(btn, i / itemsPerRow, i % itemsPerRow);
    }

    // Nút Tùy chỉnh (ID 13)
    int row = defaultBackgrounds.count() / itemsPerRow;
    int col = defaultBackgrounds.count() % itemsPerRow;
    m_customPreviewButton = new QToolButton;
    m_customPreviewButton->setFixedSize(buttonSize);
    m_customPreviewButton->setCheckable(true);
    m_customPreviewButton->setText("File...");
    m_customPreviewButton->setToolTip("Chọn ảnh nền từ máy tính. Khuyến nghị 2000x2000px");
    m_customPreviewButton->setStyleSheet(getPreviewStyle());
    // =======================================================
    // SỬA LỖI 1: ID phải là 13 (vì 0-12 đã được dùng)
    m_backgroundGroup->addButton(m_customPreviewButton, 13);
    // =======================================================
    previewLayout->addWidget(m_customPreviewButton, row, col);

    // Nút Màu đơn sắc (ID 14)
    col++;
    if (col >= itemsPerRow) {
        row++;
        col = 0;
    }
    m_solidColorPreviewButton = new QToolButton;
    m_solidColorPreviewButton->setFixedSize(buttonSize);
    m_solidColorPreviewButton->setCheckable(true);
    m_solidColorPreviewButton->setText("Đơn sắc...");
    m_solidColorPreviewButton->setToolTip("Chọn một màu nền đơn sắc");
    m_solidColorPreviewButton->setStyleSheet(getPreviewStyle(m_selectedColor));
    // =======================================================
    // SỬA LỖI 2: ID phải là 14
    m_backgroundGroup->addButton(m_solidColorPreviewButton, 14);
    // =======================================================
    previewLayout->addWidget(m_solidColorPreviewButton, row, col);

    m_backgroundGroup->button(2)->setChecked(true); // Mặc định là ảnh 3 (ID 2)
    appearanceLayout->addLayout(previewLayout);

    m_transparentCalendarCheck = new QCheckBox("Làm trong suốt nền lịch chính");
    m_transparentCalendarCheck->setChecked(true);
    appearanceLayout->addWidget(m_transparentCalendarCheck);

    mainLayout->addWidget(appearanceGroup);

    mainLayout->addWidget(timeGroup);

    mainLayout->addWidget(appearanceGroup);

    // --- 2. KHUNG ĐÁNH GIÁ ỨNG DỤNG (Giữ nguyên) ---
    QGroupBox *reviewGroup = new QGroupBox("Bạn cảm thấy ứng dụng này thế nào?");
    QHBoxLayout *reviewLayout = new QHBoxLayout(reviewGroup);

    auto createReviewButton = [&](const QString &iconPath, const QString &text) {
        QPushButton *button = new QPushButton(QIcon(iconPath), " " + text);
        button->setIconSize(QSize(24, 24));
        button->setCursor(Qt::PointingHandCursor);
        return button;
    };
    QPushButton *loveButton = createReviewButton(":/resource/icons/love.png", "Tuyệt vời");
    QPushButton *ideaButton = createReviewButton(":/resource/icons/idea.png", "Có ý tưởng");
    QPushButton *sadButton = createReviewButton(":/resource/icons/sad.png", "Chưa tốt");
    reviewLayout->addWidget(loveButton);
    reviewLayout->addWidget(ideaButton);
    reviewLayout->addWidget(sadButton);
    reviewLayout->addStretch();
    mainLayout->addWidget(reviewGroup);

    mainLayout->addStretch();

    // --- 3. NÚT OK/CANCEL (Giữ nguyên) ---
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    mainLayout->addWidget(buttonBox);

    // --- 4. KẾT NỐI TÍN HIỆU (Giữ nguyên) ---
    auto showThankYou = [this](){ QMessageBox::information(this, "Cảm ơn!", "Cảm ơn bạn đã chia sẻ cảm nhận!"); };
    connect(loveButton, &QPushButton::clicked, this, showThankYou);
    connect(ideaButton, &QPushButton::clicked, this, showThankYou);
    connect(sadButton, &QPushButton::clicked, this, showThankYou);

    connect(m_customPreviewButton, &QToolButton::clicked, this, &SettingsDialog::onBrowseClicked);
    connect(m_solidColorPreviewButton, &QToolButton::clicked, this, &SettingsDialog::onColorClicked);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

// ... onBrowseClicked và onColorClicked (Giữ nguyên, không thay đổi) ...
int SettingsDialog::selectedBackgroundIndex() const
{
    return m_backgroundGroup->checkedId();
}

void SettingsDialog::onBrowseClicked()
{
    m_customPreviewButton->setChecked(true);
    QString filePath = QFileDialog::getOpenFileName(
        this, "Chọn ảnh nền", QDir::homePath(), "Image Files (*.png *.jpg *.jpeg *.bmp)"
        );
    if (!filePath.isEmpty()) {
        m_customImagePath = filePath;
        m_customPreviewButton->setIcon(QIcon(filePath));
        m_customPreviewButton->setIconSize(QSize(84, 54));
        m_customPreviewButton->setText("");
    }
}

void SettingsDialog::onColorClicked()
{
    m_solidColorPreviewButton->setChecked(true);
    QColor color = QColorDialog::getColor(m_selectedColor, this, "Chọn màu nền");
    if (color.isValid()) {
        m_selectedColor = color;
        m_solidColorPreviewButton->setIcon(QIcon());
        m_solidColorPreviewButton->setStyleSheet(getPreviewStyle(m_selectedColor));
        m_solidColorPreviewButton->setText("");
    }
}


// =======================================================
// SỬA LỖI 3: ID phải là 13
// =======================================================
QString SettingsDialog::selectedImagePath() const
{
    if (m_backgroundGroup->checkedId() == 13) {
        return m_customImagePath;
    }
    return QString();
}

// =======================================================
// SỬA LỖI 4: ID phải là 14
// =======================================================
QColor SettingsDialog::selectedSolidColor() const
{
    if (m_backgroundGroup->checkedId() == 14) {
        return m_selectedColor;
    }
    return QColor();
}

bool SettingsDialog::isCalendarTransparent() const
{
    return m_transparentCalendarCheck->isChecked();
}

void SettingsDialog::setCurrentSettings(bool use24Hour, int offsetSeconds)
{
    // THÊM DÒNG NÀY:
    m_chk24Hour->setChecked(use24Hour);
    // Tìm index của QComboBox có data là offsetSeconds
    int index = m_cmbTimezone->findData(QVariant(offsetSeconds));
    if (index != -1) {
        m_cmbTimezone->setCurrentIndex(index);
    } else {
        // Nếu không tìm thấy (ví dụ: offset của máy là +07:00 = 25200)
        // và chúng ta chưa thêm nó vào list, hãy tìm offset gần nhất
        // Tạm thời chỉ đặt về 0 (UTC) nếu không tìm thấy
        index = m_cmbTimezone->findData(QVariant(0));
        m_cmbTimezone->setCurrentIndex(index);
    }
}

bool SettingsDialog::is24HourFormat() const
{
    // Trả về lựa chọn của người dùng
    return m_chk24Hour->isChecked();
}

int SettingsDialog::getSelectedOffsetSeconds() const
{
    // Lấy data (số giây) mà chúng ta đã lưu
    return m_cmbTimezone->currentData().toInt();
}
