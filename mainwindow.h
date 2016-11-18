#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QStringList>

class QSplitter;
class QMenu;
class QToolBar;
class QAction;
class Configuration;
class QTextCodec;
class PictureWorker;
class QTreeView;
class QLabel;
class QFileSystemModel;
class QTextEdit;
class QTableWidget;
class QModelIndex;
class GraphicsView;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent* event);    

signals:
    void paintImageInView(const QString& filePath);
    void beginCalculation(int mode, const QString& imageName, const QString& objName, const QString& covName,
                          const QString& dirName, int b, int x, int y, int fW, int fH, int iW, int iH,
                          double s, double i);

private slots:
    void newFile();
    void open();
    void start();
    bool save();
    bool saveAs();
    void about();
    void openRecentFile();
    void configModified();
    void showTextResult(const QString& fileName);
    void showFiles(const QModelIndex& index);
    void receiveResultFolder(const QString& folderName, bool isVisible);
    void calculationFinished();
    void receiveError(const QString& error);

private:
    void setCodec(QTextCodec *codec);
    void createActions();
    void createMenus();
    void createToolBar();
    void createStatusBar();
    void readSettings();
    void writeSettings();
    void setCurrentFile(const QString& fileName);
    void updateRecentFileActions();
    bool loadFile(const QString& fileName);
    bool saveFile(const QString& fileName);
    bool okToContinue();
    QString strippedName(const QString& fullFileName);


    QStringList recentFiles;
    QString curFile;

    enum {MAXRECENTFILES = 5};
    QAction* recentFileActions[MAXRECENTFILES];
    QAction* separatorAction;

    QMenu* fileMenu;
    QMenu* helpMenu;

    QToolBar* fileToolBar;

    QAction* newAction;
    QAction* startAction;
    QAction* openAction;
    QAction* saveAction;
    QAction* saveAsAction;
    QAction* exitAction;
    QAction* aboutAction;

    QLabel* lbProc;
    QLabel* lbComment;
    QSplitter* workPlace;
    QSplitter* workPlace2;
    QSplitter* workPlace3;
    QTreeView* folderViewer;
    //QTextEdit* resultText;
    QTableWidget* resultView;
    QFileSystemModel* fsModel;    
    Configuration* config;
    GraphicsView* picViewer;
    PictureWorker* worker;
    QThread* thread;
};

#endif // MAINWINDOW_H
