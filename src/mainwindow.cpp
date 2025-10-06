#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QTabBar>
#include <QStackedWidget>
#include <QToolButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QIcon>
#include <QFrame>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // ===== Tab bar =====
    QTabBar *tabBar = new QTabBar(this);
    tabBar->addTab("Trang chủ");
    tabBar->addTab("Dạng xem");
    tabBar->addTab("Trợ giúp");
    tabBar->setExpanding(false);
    tabBar->setDrawBase(false);

    // ===== Toolbar Stack =====
    QStackedWidget *toolbarStack = new QStackedWidget(this);

    // --- Home toolbar ---
    QWidget *homePage = new QWidget;
    QHBoxLayout *homeLayout = new QHBoxLayout(homePage);
    homeLayout->setContentsMargins(10, 6, 10, 6);
    homeLayout->setSpacing(10);

    auto makeBtn = [](const QString &text, const QString &icon = QString()) {
        QToolButton *btn = new QToolButton;
        btn->setText(text);
        if (!icon.isEmpty()) btn->setIcon(QIcon::fromTheme(icon));
        btn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        btn->setCursor(Qt::PointingHandCursor);
        return btn;
    };

    //Hỗ trợ homeLayout->addWidget(makeSeparator());
    auto makeSeparator = []() {
        QFrame *line = new QFrame;
        line->setFrameShape(QFrame::VLine);
        line->setFrameShadow(QFrame::Plain);
        line->setStyleSheet("color: #e0e0e0;"); // màu xám nhẹ
        return line;
    };

    // Nút đặc biệt "New event"
    QToolButton *btnNewEvent = makeBtn(" Sự kiện mới");
    btnNewEvent->setObjectName("btnNewEvent");

    homeLayout->addWidget(btnNewEvent);
    homeLayout->addWidget(makeBtn("Ngày"));
    homeLayout->addWidget(makeBtn("Tuần làm việc"));
    homeLayout->addWidget(makeBtn("Tháng"));
    homeLayout->addWidget(makeBtn("Dạng xem tách"));
    homeLayout->addWidget(makeSeparator());
    homeLayout->addWidget(makeBtn("Bộ lọc"));
    homeLayout->addWidget(makeSeparator());
    homeLayout->addWidget(makeBtn("In"));
    homeLayout->addStretch();

    toolbarStack->addWidget(homePage);

    // --- View toolbar ---
    QWidget *viewPage = new QWidget;
    QHBoxLayout *viewLayout = new QHBoxLayout(viewPage);
    viewLayout->setContentsMargins(10,6,10,6);
    viewLayout->addWidget(new QLabel("View tools here"));
    viewLayout->addStretch();
    toolbarStack->addWidget(viewPage);

    // --- Help toolbar ---
    QWidget *helpPage = new QWidget;
    QHBoxLayout *helpLayout = new QHBoxLayout(helpPage);
    helpLayout->setContentsMargins(10,6,10,6);
    helpLayout->addWidget(new QLabel("Help tools here"));
    helpLayout->addStretch();
    toolbarStack->addWidget(helpPage);

    // ===== Menu cố định =====
    QWidget *topBar = new QWidget;
    QVBoxLayout *topLayout = new QVBoxLayout(topBar);
    topLayout->setContentsMargins(0,0,0,0);
    topLayout->setSpacing(0);
    topLayout->addWidget(tabBar);
    topLayout->addWidget(toolbarStack);

    QFrame *separator = new QFrame;
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Plain);
    separator->setStyleSheet("color: #dcdcdc;");
    topLayout->addWidget(separator);

    // ===== Main content =====
    QWidget *mainContent = new QWidget;
    mainContent->setStyleSheet("background: white;");

    // ===== Tổng layout =====
    QWidget *central = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(topBar, 0, Qt::AlignTop);
    mainLayout->addWidget(mainContent, 1);
    setCentralWidget(central);

    // ===== Kết nối =====
    connect(tabBar, &QTabBar::currentChanged, toolbarStack, &QStackedWidget::setCurrentIndex);

    tabBar->setCurrentIndex(0);
    toolbarStack->setCurrentIndex(0);
}

MainWindow::~MainWindow()
{
    delete ui;
}
