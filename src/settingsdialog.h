#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

class QLineEdit;
class QComboBox;
class QCheckBox;

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);

    QString selectedImagePath() const;
    int selectedBackgroundIndex() const;
    bool isCalendarTransparent() const;

private slots:
    void onBrowseClicked();

private:
    QLineEdit *m_imagePathLineEdit;
    QComboBox *m_themeComboBox;
    QCheckBox *m_transparentCalendarCheck;
};

#endif // SETTINGSDIALOG_H
