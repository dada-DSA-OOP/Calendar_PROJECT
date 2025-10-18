#include "sidepanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QPropertyAnimation>
#include <QScrollArea>
#include <QSpacerItem>
#include <QGraphicsDropShadowEffect>
#include <QEasingCurve>

SidePanel::SidePanel(const QString &title, QWidget *parent)
    : QWidget(parent)
{
    setObjectName("sidePanel"); // ID chung cho style
    setMinimumWidth(400);

    // Hiệu ứng đổ bóng
    auto *shadowEffect = new QGraphicsDropShadowEffect(this);
    shadowEffect->setBlurRadius(25);
    shadowEffect->setXOffset(0);
    shadowEffect->setYOffset(0);
    shadowEffect->setColor(QColor(0, 0, 0, 100));
    setGraphicsEffect(shadowEffect);

    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 10);
    m_mainLayout->setSpacing(0);

    // Thanh tiêu đề
    QWidget *titleBar = new QWidget();
    titleBar->setObjectName("sidePanelTitleBar");
    titleBar->setLayout(new QHBoxLayout());
    titleBar->layout()->setContentsMargins(15, 8, 8, 8);

    QLabel *titleLabel = new QLabel(title);
    titleLabel->setObjectName("sidePanelTitle");

    QPushButton *btnClose = new QPushButton("×");
    btnClose->setObjectName("sidePanelCloseButton");
    btnClose->setCursor(Qt::PointingHandCursor);

    titleBar->layout()->addWidget(titleLabel);
    titleBar->layout()->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
    titleBar->layout()->addWidget(btnClose);
    m_mainLayout->addWidget(titleBar);

    // Animation
    m_animation = new QPropertyAnimation(this, "geometry", this);
    m_animation->setDuration(300);
    m_animation->setEasingCurve(QEasingCurve::InOutCubic);

    // Kết nối nút đóng với logic ẩn panel
    connect(btnClose, &QPushButton::clicked, this, [this, parent](){
        if(parent) {
            toggleVisibility(parent->geometry(), parent->findChild<QWidget*>("topBar")->height());
        }
    });
}

void SidePanel::setContentLayout(QLayout *contentLayout)
{
    // Tạo vùng cuộn và thêm nội dung vào
    QScrollArea *scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setObjectName("sidePanelScrollArea");
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QWidget *contentWidget = new QWidget;
    contentWidget->setLayout(contentLayout);
    scrollArea->setWidget(contentWidget);

    m_mainLayout->addWidget(scrollArea);
}

void SidePanel::toggleVisibility(const QRect &parentGeo, int topBarHeight)
{
    if (m_animation->state() == QAbstractAnimation::Running) return;

    int panelWidth = this->width();
    int parentWidth = parentGeo.width();
    int parentHeight = parentGeo.height();

    if (isHidden()) {
        raise();
        show();
        disconnect(m_animation, &QAbstractAnimation::finished, this, &QWidget::hide);
        m_animation->setStartValue(QRect(parentWidth, topBarHeight, panelWidth, parentHeight - topBarHeight));
        m_animation->setEndValue(QRect(parentWidth - panelWidth, topBarHeight, panelWidth, parentHeight - topBarHeight));
    } else {
        connect(m_animation, &QAbstractAnimation::finished, this, &QWidget::hide);
        m_animation->setStartValue(geometry());
        m_animation->setEndValue(QRect(parentWidth, topBarHeight, panelWidth, parentHeight - topBarHeight));
    }
    m_animation->start();
}

void SidePanel::hidePanel(const QRect &parentGeo, int topBarHeight)
{
    if (isHidden() || m_animation->state() == QAbstractAnimation::Running) return;

    connect(m_animation, &QAbstractAnimation::finished, this, &QWidget::hide);
    m_animation->setStartValue(geometry());
    m_animation->setEndValue(QRect(parentGeo.width(), topBarHeight, width(), parentGeo.height() - topBarHeight));
    m_animation->start();
}
