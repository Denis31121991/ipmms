
#include "controls.h"

//#include <QPushButton>
#include <QToolButton>
#include <QFileDialog>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QDebug>

OpenWidget::OpenWidget(bool isFile, QWidget *parent) :
    QWidget(parent),
    file(isFile)
{
    lineEditPath = new QLineEdit(this);
    //pushButtonPath = new QPushButton;
    pushButtonPath = new QToolButton(this);
    mainLayout = new QHBoxLayout(this);
    setFocusProxy(lineEditPath);

    pushButtonPath->setText("...");
    pushButtonPath->setContentsMargins(0, 0, 0, 0);

    lineEditPath->setContentsMargins(0, 0, 0, 0);

    mainLayout->setContentsMargins( 0, 0, 0, 0 );
    mainLayout->setMargin(0);
    mainLayout->addWidget(lineEditPath);
    mainLayout->addWidget(pushButtonPath);
    mainLayout->setSpacing(0);

    connect(pushButtonPath, SIGNAL(clicked()), this, SLOT(determinePath()));
}

QString OpenWidget::getPath() const
{
    return lineEditPath->text();
}

void OpenWidget::setPath(const QString &path)
{
    lineEditPath->setText(path);
}

void OpenWidget::determinePath()
{
    QString pathFile;

    if (file)
    {
        pathFile = QFileDialog::getOpenFileName(0, tr("Выбрать файл изображения"),
                                                tr("."), tr("*.tif\n*.tiff\n"));
    }
    else
    {
        pathFile = QFileDialog::getExistingDirectory(0, tr("Выбрать директорию"),
                                          tr("."), QFileDialog::ShowDirsOnly |
                                          QFileDialog::DontResolveSymlinks);
    }    

    if ( !pathFile.isEmpty() )
    {
        lineEditPath->setText(pathFile);
    }
}

OpenWidget::~OpenWidget()
{
    delete mainLayout;
    delete lineEditPath;
    delete pushButtonPath;
}
