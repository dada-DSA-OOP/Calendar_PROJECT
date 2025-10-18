#ifndef FUNNYTIPWIDGET_H
#define FUNNYTIPWIDGET_H

#include <QWidget>
#include <QStringList>

class QLabel;
class QPropertyAnimation;

class FunnyTipWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FunnyTipWidget(QWidget *parent = nullptr);

    void reposition();
    void start();

private slots:
    void updateAndShowNextTip();

private:
    QLabel *m_tipLabel;
    QPropertyAnimation *m_animation;
    QWidget* m_parentWidget;
    QStringList m_tipDeck;
};

#endif // FUNNYTIPWIDGET_H
