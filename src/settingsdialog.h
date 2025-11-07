#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QColor>

class QLineEdit;
class QComboBox;
class QCheckBox;
class QButtonGroup;
class QToolButton;
class QColorDialog;

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);

    QString selectedImagePath() const;
    int selectedBackgroundIndex() const;
    bool isCalendarTransparent() const;
    QColor selectedSolidColor() const;

private slots:
    void onBrowseClicked();
    void onColorClicked();

private:
    QCheckBox *m_transparentCalendarCheck;

    // Các thành phần mới để quản lý preview
    QButtonGroup *m_backgroundGroup;
    QToolButton *m_customPreviewButton;
    QToolButton *m_solidColorPreviewButton;
    QColorDialog *m_colorDialog;

    QString m_customImagePath;
    QColor m_selectedColor;
};

#endif // SETTINGSDIALOG_H
