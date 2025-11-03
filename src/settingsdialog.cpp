#include "settingsdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QGroupBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QLabel>
#include <QMessageBox>
#include <QComboBox>
#include <QCheckBox>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Cài đặt");
    setMinimumWidth(550);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);

    // --- 1. KHUNG GIAO DIỆN ---
    QGroupBox *appearanceGroup = new QGroupBox("Giao diện");
    QFormLayout *formLayout = new QFormLayout(appearanceGroup);

    m_themeComboBox = new QComboBox;
    m_themeComboBox->addItem("Nền mặc định (White)", 0);
    m_themeComboBox->addItem("Nền mặc định (Pink)", 1);
    m_themeComboBox->addItem("Nền mặc định (Green)", 2);
    m_themeComboBox->addItem("Nền mặc định (Blue)", 3);
    m_themeComboBox->addItem("Tùy chỉnh từ file...", 4);
    formLayout->addRow("Chọn nền:", m_themeComboBox);

    QWidget *customPathWidget = new QWidget;
    QHBoxLayout *pathLayout = new QHBoxLayout(customPathWidget);
    pathLayout->setContentsMargins(0, 0, 0, 0);
    m_imagePathLineEdit = new QLineEdit;
    m_imagePathLineEdit->setPlaceholderText("Chọn một file ảnh...");
    QPushButton *browseButton = new QPushButton("...");
    browseButton->setFixedWidth(40);
    pathLayout->addWidget(m_imagePathLineEdit);
    pathLayout->addWidget(browseButton);
    formLayout->addRow("Đường dẫn file:", customPathWidget);

    // THÊM CHECKBOX VÀO ĐÂY
    m_transparentCalendarCheck = new QCheckBox("Làm trong suốt nền lịch chính");
    m_transparentCalendarCheck->setChecked(true); // Mặc định là trong suốt
    formLayout->addRow("", m_transparentCalendarCheck);

    mainLayout->addWidget(appearanceGroup);

    // --- 2. KHUNG ĐÁNH GIÁ ỨNG DỤNG ---
    QGroupBox *reviewGroup = new QGroupBox("Bạn cảm thấy ứng dụng này thế nào?");
    QHBoxLayout *reviewLayout = new QHBoxLayout(reviewGroup);
    reviewLayout->setSpacing(15);

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

    // --- 3. NÚT OK/CANCEL ---
    // Moved this block up before the connect statements
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    mainLayout->addWidget(buttonBox);

    // --- 4. KẾT NỐI TÍN HIỆU ---
    // Review buttons
    auto showThankYou = [this](){ QMessageBox::information(this, "Cảm ơn!", "Cảm ơn bạn đã chia sẻ cảm nhận!"); };
    connect(loveButton, &QPushButton::clicked, this, showThankYou);
    connect(ideaButton, &QPushButton::clicked, this, showThankYou);
    connect(sadButton, &QPushButton::clicked, this, showThankYou);

    // Browse button
    connect(browseButton, &QPushButton::clicked, this, &SettingsDialog::onBrowseClicked);

    // OK/Cancel buttons
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    // ComboBox logic
    connect(m_themeComboBox, &QComboBox::currentIndexChanged, this, [=](int index){
        bool isCustom = (index == 4);
        customPathWidget->setEnabled(isCustom);
    });
    // Set initial state
    customPathWidget->setEnabled(false);
}

// Trả về ID của radio button được chọn
int SettingsDialog::selectedBackgroundIndex() const
{
    return m_themeComboBox->currentIndex();
}

void SettingsDialog::onBrowseClicked()
{
    QString filePath = QFileDialog::getOpenFileName(
        this,
        "Chọn ảnh nền",
        QDir::homePath(),
        "Image Files (*.png *.jpg *.jpeg *.bmp)"
        );

    if (!filePath.isEmpty()) {
        m_imagePathLineEdit->setText(filePath);
    }
}

QString SettingsDialog::selectedImagePath() const
{
    return m_imagePathLineEdit->text();
}

bool SettingsDialog::isCalendarTransparent() const
{
    return m_transparentCalendarCheck->isChecked();
}
