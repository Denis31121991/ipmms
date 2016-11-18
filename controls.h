#ifndef CONTROLS_H
#define CONTROLS_H

#include <QWidget>
#include <QString>

class QHBoxLayout;
class QLineEdit;
//class QPushButton;
class QToolButton;

class OpenWidget : public QWidget
{
    Q_OBJECT

public:
    explicit OpenWidget(bool isFile, QWidget* parent = 0);
    ~OpenWidget();
    QString getPath() const;
    void setPath(const QString& path);

private slots:
    void determinePath();

private:
    QHBoxLayout* mainLayout;
    QLineEdit* lineEditPath;
    //QPushButton* pushButtonPath;
    QToolButton* pushButtonPath;
    bool file;
};

#endif // CONTROLS_H

