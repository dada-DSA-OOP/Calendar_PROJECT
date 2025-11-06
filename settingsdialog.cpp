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
