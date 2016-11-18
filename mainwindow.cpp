#include "mainwindow.h"
#include "configuration.h"
#include "pictureworker.h"
#include "view.h"

#include <QAction>
#include <QSplitter>
#include <QStyle>
#include <QMenuBar>
#include <QMenu>
#include <QToolBar>
#include <QMessageBox>
#include <QFileDialog>
#include <QCloseEvent>
#include <QFileInfo>
#include <QFile>
#include <QSettings>
#include <QTextCodec>
#include <QTreeView>
#include <QLabel>
#include <QFileSystemModel>
#include <QModelIndex>
#include <QDebug>
#include <QMovie>
#include <QStatusBar>
#include <QHeaderView>
#include <QThread>
#include <QTableWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setCodec(QTextCodec::codecForName("CP1251"));

    createActions();
    createMenus();
    createToolBar();
    createStatusBar();

    picViewer = new GraphicsView(this);
    config = new Configuration(this);
    workPlace = new QSplitter(Qt::Horizontal, this);
    workPlace2 = new QSplitter(Qt::Horizontal, this);
    workPlace3 = new QSplitter(Qt::Vertical, this);
    folderViewer = new QTreeView(this);
    fsModel = new QFileSystemModel(this);
    worker = new PictureWorker;
    thread = new QThread;
    resultView = new QTableWidget(7, 2, this);

    resultView->setHorizontalHeaderLabels(QStringList() << tr("Исходное") << tr("После обработки"));
    resultView->setVerticalHeaderLabels(QStringList() << tr("Среднее") << tr("Сигма") << tr("Сигма^2")
                                                      << tr("Макс.Амплитуда") << tr("Мощьность Сигнала")
                                                      << tr("Отнош. Сигнал/Шум1") << tr("Отнош. Сигнал/Шум2"));
    resultView->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    resultView->verticalHeader()->setResizeMode(QHeaderView::Stretch);
    resultView->setSelectionMode(QAbstractItemView::NoSelection);
    resultView->setEditTriggers(QTableWidget::NoEditTriggers);

    config->setEnabled(false);

    fsModel->setFilter(QDir::Dirs | QDir::NoDotAndDotDot | QDir::Files);
    fsModel->setRootPath("");
    folderViewer->setModel(fsModel);
    folderViewer->setIndentation(10);
    folderViewer->hideColumn(1);
    folderViewer->hideColumn(2);
    folderViewer->hideColumn(3);

    workPlace3->addWidget(config);
    workPlace3->addWidget(resultView);

    workPlace->addWidget(picViewer);
    workPlace->addWidget(workPlace3);
    workPlace->setStretchFactor(0, 1);

    workPlace2->addWidget(folderViewer);
    workPlace2->addWidget(workPlace);
    workPlace2->setStretchFactor(1, 1);

    setCentralWidget(workPlace2);

    readSettings();

    worker->moveToThread(thread);

    connect(config, SIGNAL(modified()), this, SLOT(configModified()));
    connect(folderViewer, SIGNAL(expanded(QModelIndex)), folderViewer, SIGNAL(doubleClicked(QModelIndex)));
    connect(folderViewer, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(showFiles(QModelIndex)));
    connect(this, SIGNAL(paintImageInView(QString)), picViewer, SLOT(paintImage(QString)));
    connect(worker, SIGNAL(sendResultFolder(QString, bool)), this, SLOT(receiveResultFolder(QString, bool)));
    connect(thread, SIGNAL(finished()), lbProc, SLOT(hide()));
    connect(thread, SIGNAL(started()), lbProc, SLOT(show()));
    connect(thread, SIGNAL(started()), lbProc->movie(), SLOT(start()));
    connect(thread, SIGNAL(finished()), lbProc->movie(), SLOT(stop()));
    connect(thread, SIGNAL(finished()), this, SLOT(calculationFinished()));
    connect(worker, SIGNAL(calculateFinished()), thread, SLOT(quit()));
    connect(this, SIGNAL(beginCalculation(int, QString, QString, QString, QString, int, int, int, int, int, int, int, double, double)),
            worker, SLOT(beginCalculation(int, QString, QString, QString, QString, int, int, int, int, int, int, int, double, double)));
    connect(worker, SIGNAL(sendError(QString)), this, SLOT(receiveError(QString)));
}

MainWindow::~MainWindow()
{    
    delete worker;

    thread->terminate();
    delete thread;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (okToContinue())
    {
        writeSettings();
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

void MainWindow::newFile()
{
    if (okToContinue())
    {
        setCurrentFile("");
        config->createNewFile();        
    }
}

void MainWindow::open()
{
    if (okToContinue())
    {
        QString fileName = QFileDialog::getOpenFileName(this,
                                      tr("Открыть конфигурационный файл"),
                                      tr("."),
                                      tr("Конфигурационный файл ipmms (*.cfg)"));
        if (!fileName.isEmpty())
        {
            loadFile(fileName);
        }
    }
}

void MainWindow::start()
{
    newAction->setEnabled(false);
    startAction->setEnabled(false);
    openAction->setEnabled(false);

    for (int i = 0; i < MAXRECENTFILES; ++i)
    {
        recentFileActions[i]->setEnabled(false);
    }

    thread->start();

    QSize fragSize = config->getSizeFragment();
    QSize imageSize = config->getImageSize();
    QPoint coorsObj = config->getPositionObject();

    emit beginCalculation(config->getMode(), config->getBackgroundImageName(), config->getObjectImageName(),
                          config->getCovarMatrixImage(), config->getResultFolder(), config->getBorder(),
                          coorsObj.x(), coorsObj.y(), fragSize.width(), fragSize.height(), imageSize.width(),
                          imageSize.height(), config->getParameterObject(), config->getIntensity());
}

bool MainWindow::save()
{
    if (curFile.isEmpty())
    {
        return saveAs();
    }
    else
    {
        return saveFile(curFile);
    }
}

bool MainWindow::saveAs()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                tr("Сохранить конфигурационный файл ipmms"),
                                tr("."),
                                tr("Конфигурационный файл (*.cfg)"));

    if (fileName.isEmpty())
    {
        return false;
    }

    return saveFile(fileName);
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("Об ipmms"), tr("<h2>IPMMS 1.0</h2>"
                                                "<p>Автор: Будеев Д.Е.</p>"));
}

void MainWindow::openRecentFile()
{
    if (okToContinue())
    {
        QAction* action = qobject_cast<QAction*>(sender());

        if (action)
        {
            loadFile(action->data().toString());
        }
    }
}

void MainWindow::configModified()
{
    setWindowModified(true);
}

void MainWindow::showTextResult(const QString &fileName)
{
    QFile file(fileName);

    if (!file.open(QFile::ReadOnly))
    {
        QMessageBox::critical(this, tr("Ошибка"), tr("Ошибка открытия файла ") + fileName);

        return;
    }

    QTextStream out(&file);
    QStringList resultStr = out.readAll().split('\n', QString::SkipEmptyParts);
    QStringList value;
    QTableWidgetItem* item;
    int j = 0;

    for (int i = 1; i < resultStr.count(); ++i)
    {
        value = resultStr[i].trimmed().split(": ");

        if (value[0] == tr("Фильтрованное изображение"))
        {
            j++;

            continue;
        }

        if (value.count() == 2)
        {
            item = new QTableWidgetItem(value[1]);
            item->setFlags((item->flags() ^ Qt::ItemIsEditable) ^ Qt::ItemIsSelectable);
            resultView->setItem((i - 1 - j) % 7, j, item);
        }
    }
}

void MainWindow::setCodec(QTextCodec *codec)
{
    if (codec)
    {
        QTextCodec::setCodecForLocale(codec);
        QTextCodec::setCodecForCStrings(codec);
        QTextCodec::setCodecForTr(codec);
    }
}

void MainWindow::createActions()
{
    newAction = new QAction(tr("&Новый"), this);
    newAction->setIcon(QIcon(":icons/images/new.png"));
    newAction->setShortcut(QKeySequence::New);
    connect(newAction, SIGNAL(triggered()), this, SLOT(newFile()));

    openAction = new QAction(tr("&Открыть"), this);
    openAction->setIcon(QIcon(":icons/images/open.png"));
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, SIGNAL(triggered()), this, SLOT(open()));

    saveAction = new QAction(tr("&Сохранить"), this);
    saveAction->setIcon(QIcon(":icons/images/save.png"));
    saveAction->setShortcut(QKeySequence::Save);
    saveAction->setEnabled(false);
    connect(saveAction, SIGNAL(triggered()), this, SLOT(save()));

    saveAsAction = new QAction(tr("&Сохранить как"), this);
    saveAsAction->setIcon(QIcon(":icons/images/save_as.png"));
    saveAsAction->setShortcut(QKeySequence::SaveAs);
    saveAsAction->setEnabled(false);
    connect(saveAsAction, SIGNAL(triggered()), this, SLOT(saveAs()));

    for (int i = 0; i < MAXRECENTFILES; ++i)
    {
        recentFileActions[i] = new QAction(this);
        recentFileActions[i]->setVisible(false);
        connect(recentFileActions[i], SIGNAL(triggered()),
                this, SLOT(openRecentFile()));
    }

    exitAction = new QAction(tr("&Выход"), this);
    exitAction->setShortcut(tr("Ctrl+Q"));
    connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));

    startAction = new QAction(tr("&Старт"), this);
    startAction->setIcon(QIcon(":icons/images/start.png"));
    startAction->setShortcut(tr("Ctrl+R"));
    startAction->setEnabled(false);
    connect(startAction, SIGNAL(triggered()), this, SLOT(start()));

    aboutAction = new QAction(tr("&Справка"), this);
    aboutAction->setIcon(QIcon(":icons/images/help.png"));
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&Файл"));
    fileMenu->addAction(newAction);
    fileMenu->addAction(openAction);
    fileMenu->addAction(saveAction);
    fileMenu->addAction(saveAsAction);
    separatorAction = fileMenu->addSeparator();

    for (int i = 0; i < MAXRECENTFILES; ++i)
    {
        fileMenu->addAction(recentFileActions[i]);
    }

    fileMenu->addSeparator();
    fileMenu->addAction(exitAction);

    menuBar()->addSeparator();

    helpMenu = menuBar()->addMenu(tr("&Справка"));
    helpMenu->addAction(aboutAction);
}

void MainWindow::createToolBar()
{
    fileToolBar = addToolBar(tr("&Файл"));
    fileToolBar->addAction(newAction);
    fileToolBar->addAction(openAction);
    fileToolBar->addAction(saveAction);
    fileToolBar->addAction(saveAsAction);
    fileToolBar->addSeparator();
    fileToolBar->addAction(startAction);
}

void MainWindow::createStatusBar()
{
    lbProc = new QLabel(this);
    QMovie* loading = new QMovie(":animation/animation/load.gif");
    lbProc->setMovie(loading);
    statusBar()->addWidget(lbProc);
}

void MainWindow::readSettings()
{
    QSettings settings("IPMMS", "ipmms");

    restoreGeometry(settings.value("geometry").toByteArray());
    recentFiles = settings.value("recentFiles").toStringList();
    updateRecentFileActions();
    workPlace->restoreState(settings.value("workPlace").toByteArray());
    workPlace2->restoreState(settings.value("workPlace2").toByteArray());
}

void MainWindow::writeSettings()
{
    QSettings settings("IPMMS", "ipmms");

    settings.setValue("geometry", saveGeometry());
    settings.setValue("recentFiles", recentFiles);
    settings.setValue("workPlace", workPlace->saveState());
    settings.setValue("workPlace2", workPlace2->saveState());
}

void MainWindow::setCurrentFile(const QString& fileName)
{
    curFile = fileName;
    setWindowModified(false);

    QString showName = tr("Без названия");

    if (!curFile.isEmpty())
    {
        showName = strippedName(curFile);
        recentFiles.removeAll(curFile);
        recentFiles.prepend(curFile);
        updateRecentFileActions();
    }

    saveAction->setEnabled(true);
    saveAsAction->setEnabled(true);
    startAction->setEnabled(true);
    config->setEnabled(true);

    setWindowTitle(tr("%1[*] - %2").arg(showName).arg("ipmms"));
}

void MainWindow::updateRecentFileActions()
{
    QMutableStringListIterator iter(recentFiles);

    while (iter.hasNext())
    {
        if (!QFile::exists(iter.next()))
        {
            iter.remove();
        }
    }

    for (int i = 0; i < MAXRECENTFILES; ++i)
    {
        if (i < recentFiles.count())
        {
            QString text = tr("&%1 %2").arg(i + 1)
                                       .arg(strippedName(recentFiles[i]));
            recentFileActions[i]->setText(text);
            recentFileActions[i]->setData(recentFiles[i]);
            recentFileActions[i]->setVisible(true);
        }
        else
        {
            recentFileActions[i]->setVisible(false);
        }
    }

    separatorAction->setVisible(!recentFiles.isEmpty());
}

bool MainWindow::loadFile(const QString &fileName)
{
    if (!config->readFile(fileName))
    {
        return false;
    }

    setCurrentFile(fileName);

    return true;
}

bool MainWindow::saveFile(const QString &fileName)
{
    if (!config->writeFile(fileName))
    {
        return false;
    }

    setCurrentFile(fileName);

    return true;
}

bool MainWindow::okToContinue()
{
    if (isWindowModified())
    {
        int r = QMessageBox::warning(this, tr("Ipmms"),
                                     tr("Конфигурации были изменены.\n"
                                        "Хотите сохранить изменения?"),
                                     QMessageBox::Yes | QMessageBox::No |
                                     QMessageBox::Cancel);
        if (r == QMessageBox::Yes)
        {
            return save();
        }
        else
        {
            if (r == QMessageBox::Cancel)
            {
                return false;
            }
        }
    }

    return true;
}

QString MainWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

void MainWindow::showFiles(const QModelIndex &index)
{
    QFileInfo fileInfo = fsModel->fileInfo(index);

    if (fileInfo.isFile() && (fileInfo.suffix() == tr("tif")))
    {
        //qDebug() << fileInfo.absoluteFilePath();

        emit paintImageInView(fileInfo.absoluteFilePath());
    }
}

void MainWindow::receiveResultFolder(const QString &folderName, bool isVisible)
{
    //folderViewer->collapse(folderViewer->currentIndex());
    QDir folder(folderName);
    QStringList files = folder.entryList(QStringList() << "*.tiff" << "*.tif", QDir::Files);

    if (files.isEmpty())
    {
        return;
    }

    QString fileName = folderName + QString("/") + files[0];

    //qDebug() << fileName << "\n";

    QModelIndex index = fsModel->index(fileName);
    folderViewer->expand(index);
    folderViewer->setCurrentIndex(index);
    showFiles(index);

    resultView->clearContents();

    if (isVisible)
    {
        fileName = folderName + QString("/result.txt");
        showTextResult(fileName);
    }
}

void MainWindow::calculationFinished()
{
    newAction->setEnabled(true);
    startAction->setEnabled(true);
    openAction->setEnabled(true);

    for (int i = 0; i < MAXRECENTFILES; ++i)
    {
        recentFileActions[i]->setEnabled(true);
    }
}

void MainWindow::receiveError(const QString &error)
{
    thread->quit();
    QMessageBox::critical(this, tr("Ошибка"), error);
}
