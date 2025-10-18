#ifndef SIDEPANEL_H
#define SIDEPANEL_H

#include <QWidget>

class QPropertyAnimation;
class QVBoxLayout;

class SidePanel : public QWidget
{
    Q_OBJECT

public:
    explicit SidePanel(const QString &title, QWidget *parent = nullptr);

    // Hàm để thêm nội dung vào panel
    void setContentLayout(QLayout *layout);

public slots:
    void toggleVisibility(const QRect &parentGeo, int topBarHeight);
    void hidePanel(const QRect &parentGeo, int topBarHeight);

private:
    QPropertyAnimation *m_animation;
    QVBoxLayout *m_mainLayout;
};

#endif // SIDEPANEL_H
