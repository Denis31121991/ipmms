#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <QWidget>
#include <QItemDelegate>

class QLabel;
class QComboBox;
class QTreeView;
class QGridLayout;
class QCheckBox;
class QString;
class QStandardItemModel;
class QStandardItem;
class QHBoxLayout;

class ConfigurationDelegate : public QItemDelegate
{
    Q_OBJECT

public:

    ConfigurationDelegate(QObject* parent = 0);
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                             const QModelIndex &index) const;

    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const;

    void updateEditorGeometry(QWidget *editor,
            const QStyleOptionViewItem &option, const QModelIndex &index) const;

};

class Configuration : public QWidget
{
    Q_OBJECT
public:
    explicit Configuration(QWidget *parent = 0);
    ~Configuration();

    bool readFile(const QString& fileName);
    bool writeFile(const QString& fileName) const;
    QString getBackgroundImageName() const;
    QString getObjectImageName() const;
    QString getCovarMatrixImage() const;
    int getMode() const;
    int getBorder() const;
    QPoint getPositionObject() const;
    QSize getSizeFragment() const;
    double getParameterObject() const;
    QString getResultFolder() const;
    double getIntensity() const;
    QSize getImageSize() const;

signals:
    void modified();
    void sendError(const QString& error);

public slots:
    void createNewFile();        

private slots:
    QString checkParameters() const;
    void hideParameterFileCovarMat(int state);
    void hideParameterFileObject(int state);
    void hideParameterFileImage(int state);
    void selectMode(int mode);

private:
    void readSettingFile(const QString& fileName);
    QString getParameter(const QString &parameter) const;

    QLabel* lbMode;
    QComboBox* cbMode;
    QTreeView* listConfig;
    QStandardItemModel* standardModel;
    QGridLayout* mainLayout;
    QCheckBox* chbObject;
    QCheckBox* chbGenBackground;
    QCheckBox* chbCalcCovarMat;    
};

#endif // CONFIGURATION_H
