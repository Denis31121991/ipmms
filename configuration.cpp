#include "configuration.h"

#include <QLabel>
#include <QComboBox>
#include <QTreeView>
#include <QStandardItemModel>
#include <QFileDialog>
#include <QStandardItem>
#include <QGridLayout>
#include <QString>
#include <QStringList>
#include <QCheckBox>
#include <QSpinBox>
#include <QHeaderView>
#include <QFile>
#include <QMessageBox>
#include <QTextStream>
#include <QDoubleSpinBox>
#include <QDebug>
#include <QHBoxLayout>

#include "controls.h"

ConfigurationDelegate::ConfigurationDelegate(QObject* parent) :
    QItemDelegate(parent)
{

}

QWidget *ConfigurationDelegate::createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const
{
    if (index.data(Qt::UserRole + 1).toString() == tr("int"))
    {
        QSpinBox* editor = new QSpinBox(parent);
        editor->setRange(0, INT_MAX);

        return editor;
    }

    if (index.data(Qt::UserRole + 1).toString() == tr("file"))
    {
        OpenWidget* editor = new OpenWidget(true, parent);

        return editor;
    }

    if (index.data(Qt::UserRole + 1).toString() == tr("folder"))
    {
        OpenWidget* editor = new OpenWidget(false, parent);

        return editor;
    }

    if (index.data(Qt::UserRole + 1).toString() == tr("double"))
    {
        QDoubleSpinBox* editor = new QDoubleSpinBox(parent);
        editor->setRange(0, DBL_MAX);

        return editor;
    }

    return 0;
}

void ConfigurationDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QSpinBox *spinBox = qobject_cast<QSpinBox*>(editor);

    if (spinBox)
    {
        spinBox->setValue(index.data(Qt::EditRole).toInt());

        return;
    }

    OpenWidget* openWidget = qobject_cast<OpenWidget*>(editor);

    if (openWidget)
    {
        //qDebug() << index.data(Qt::UserRole + 2).toString() << "123\n";
        openWidget->setPath(index.data(Qt::EditRole).toString());

        return;
    }

    QDoubleSpinBox*  doubleSpinBox = qobject_cast<QDoubleSpinBox*>(editor);

    if (doubleSpinBox)
    {
        doubleSpinBox->setValue(index.data(Qt::EditRole).toDouble());

        return;
    }
}

void ConfigurationDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                         const QModelIndex &index) const
{
    QSpinBox *spinBox = qobject_cast<QSpinBox*>(editor);

    if (spinBox)
    {
        spinBox->interpretText();
        model->setData(index, spinBox->value(), Qt::EditRole);

        return;
    }

    OpenWidget* openWidget = qobject_cast<OpenWidget*>(editor);

    if (openWidget)
    {
        QString path = openWidget->getPath();

        if (!path.isEmpty())
        {
            model->setData(index, openWidget->getPath(), Qt::EditRole);
        }
        else
        {
            model->setData(index, tr("empty"), Qt::EditRole);
        }

        return;
    }

    QDoubleSpinBox*  doubleSpinBox = qobject_cast<QDoubleSpinBox*>(editor);

    if (doubleSpinBox)
    {
        doubleSpinBox->interpretText();
        model->setData(index, doubleSpinBox->value(), Qt::EditRole);

        return;
    }
}

void ConfigurationDelegate::updateEditorGeometry(QWidget *editor,
             const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    editor->setGeometry(option.rect);
}



Configuration::Configuration(QWidget *parent) : QWidget(parent)
{    
    lbMode = new QLabel(tr("Режим: "), this);

    cbMode = new QComboBox(this);
    cbMode->addItems(QStringList() << tr("Разбеливание")
                                   << tr("Согласованный фильтр")
                                   << tr("Согласованный фильтр с белым шумом"));

    chbObject = new QCheckBox(tr("Создать объект(Гауссойда)"), this);
    //chbObject->setCheckState(Qt::Checked);
    chbObject->setEnabled(false);

    chbGenBackground = new QCheckBox(tr("Сгенерировать фон(Белый шум)"), this);
    //chbGenBackground->setCheckState(Qt::Checked);

    chbCalcCovarMat = new QCheckBox(tr("Вычислить ковариационную матрицу"), this);
    //chbCalcCovarMat->setCheckState(Qt::Checked);

    listConfig = new QTreeView(this);
    listConfig->setItemDelegateForColumn(1, new ConfigurationDelegate());

    mainLayout = new QGridLayout(this);
    mainLayout->setContentsMargins(0, 5, 0, 0);
    mainLayout->addWidget(lbMode, 0, 0, 1, 1);
    mainLayout->addWidget(cbMode, 0, 1, 1, 1);
    mainLayout->addWidget(chbObject, 1, 0, 1, 3);
    mainLayout->addWidget(chbGenBackground, 2, 0, 1, 3);
    mainLayout->addWidget(chbCalcCovarMat, 3, 0, 1, 3);
    mainLayout->addWidget(listConfig, 4, 0, 1, 3);

    setLayout(mainLayout);

    standardModel = new QStandardItemModel(this);
    standardModel->setHorizontalHeaderLabels(QStringList() << "Параметр"
                                                           << "Значение");
    readSettingFile(":params/cfg/parameters.cfg");

    listConfig->header()->setResizeMode(QHeaderView::ResizeToContents);
    listConfig->setAlternatingRowColors(true);
    listConfig->setModel(standardModel);

    chbCalcCovarMat->setCheckState(Qt::Checked);
    chbGenBackground->setCheckState(Qt::Checked);
    chbObject->setCheckState(Qt::Checked);

    connect(cbMode, SIGNAL(currentIndexChanged(int)), this, SLOT(selectMode(int)));
    connect(chbGenBackground, SIGNAL(stateChanged(int)), this, SLOT(hideParameterFileImage(int)));
    connect(chbObject, SIGNAL(stateChanged(int)), this, SLOT(hideParameterFileObject(int)));
    connect(chbCalcCovarMat, SIGNAL(stateChanged(int)), this, SLOT(hideParameterFileCovarMat(int)));
    connect(listConfig->itemDelegateForColumn(1), SIGNAL(closeEditor(QWidget*)), this, SIGNAL(modified()));
    selectMode(0);
}

Configuration::~Configuration()
{    
}

bool Configuration::readFile(const QString &fileName)
{
    createNewFile();

    QFile file(fileName);

    if (!file.open(QFile::ReadOnly))
    {
        QMessageBox::critical(this, tr("Ошибка"), tr("Ошибка чтения файла ") + fileName);

        return false;
    }

    QTextStream instream(&file);

    QStringList list;
    int mode = -1;
    QString line = instream.readLine();

    QStandardItem* root = standardModel->invisibleRootItem();
    int rowCount = root->rowCount();

    bool flags[3] = {false, false, false};

    while (!line.isNull())
    {
        if (line[0] == QChar('\n') || line[0] == QChar('#') || line == tr(""))
        {
            line = instream.readLine();

            continue;
        }

        list = line.split(QRegExp("[=]"));

        if (list[0].trimmed().toUpper() == tr("Режим").toUpper())
        {
            mode = list[1].toInt();
        }
        else
        {
            for (int i = 0; i < rowCount; ++i)
            {
                if (root->child(i)->data(Qt::DisplayRole).toString().toUpper() == list[0].trimmed().toUpper())
                {
                    root->child(i, 1)->setData(list[1].trimmed(), Qt::DisplayRole);

                    if (root->child(i)->data(Qt::DisplayRole).toString().toUpper() ==
                            tr("Файл изображения для обработки").toUpper())
                    {
                        chbGenBackground->setCheckState(Qt::Unchecked);
                        flags[0] = true;
                    }

                    if (root->child(i)->data(Qt::DisplayRole).toString().toUpper() ==
                            tr("Файл изображения объекта").toUpper())
                    {
                        chbObject->setCheckState(Qt::Unchecked);
                        flags[1] = true;
                    }

                    if (root->child(i)->data(Qt::DisplayRole).toString().toUpper() ==
                            tr("Файл изображения ковариационной матрицы").toUpper())
                    {
                        chbCalcCovarMat->setCheckState(Qt::Unchecked);
                        flags[2] = true;
                    }

                    break;
                }
            }
        }

        line = instream.readLine();
    }

    if (!flags[0])
    {
        chbGenBackground->setCheckState(Qt::Checked);
    }

    if (!flags[1])
    {
        chbObject->setCheckState(Qt::Checked);
    }

    if (!flags[2])
    {
        chbCalcCovarMat->setCheckState(Qt::Checked);
    }

    if (mode < 0 && mode > 3)
    {
        QMessageBox::critical(this, tr("Ошибка"), tr("Неправильно задан режим. Доступные значение 0, 1, 2"));

        file.close();

        return false;
    }

    QString error = checkParameters();

    if (!error.isEmpty())
    {
        QMessageBox::critical(this, tr("Ошибка"), error);

        file.close();

        return false;
    }

    cbMode->setCurrentIndex(mode);

    file.close();

    return true;
}

bool Configuration::writeFile(const QString &fileName) const
{
    QFile file(fileName);

    if (!file.open(QFile::WriteOnly))
    {
        QMessageBox::critical((QWidget*)this->parent(), tr("Ошибка"), tr("Ошибка записи файла ") + fileName);

        return false;
    }

    QTextStream outstream(&file);

    QStandardItem* root = standardModel->invisibleRootItem();

    outstream << tr("Режим = ") << cbMode->currentIndex() << QChar('\n');

    for (int i = 0; i < root->rowCount(); ++i)
    {
        if (listConfig->isRowHidden(root->child(i)->row(), root->index()))
        {
            outstream << QChar('#');
        }

        outstream << root->child(i)->data(Qt::DisplayRole).toString()
                  << tr(" = ") << root->child(i, 1)->data(Qt::DisplayRole).toString() << QChar('\n');

    }

    file.close();

    return true;
}

QString Configuration::getBackgroundImageName() const
{
    return getParameter(tr("Файл изображения для обработки"));
}

QString Configuration::getObjectImageName() const
{
    return getParameter(tr("Файл изображения объекта"));
}

QString Configuration::getCovarMatrixImage() const
{
    return getParameter(tr("Файл изображения ковариационной матрицы"));
}

int Configuration::getMode() const
{
    return cbMode->currentIndex();
}

int Configuration::getBorder() const
{
    return getParameter(tr("Рамка")).toInt();
}

QPoint Configuration::getPositionObject() const
{
    QPoint point;

    QString coor = getParameter(tr("X координата объекта"));
    point.setX(coor.isEmpty() ? -1 : coor.toInt());
    coor = getParameter(tr("Y координата объекта"));
    point.setY(coor.isEmpty() ? -1 : coor.toInt());

    return point;
}

QSize Configuration::getSizeFragment() const
{
    QSize size;
    QString w = getParameter(tr("Ширина фрагмента"));
    QString h = getParameter(tr("Высота фрагмента"));

    if (w.isEmpty() && h.isEmpty())
    {
        if (chbObject->checkState() == Qt::Checked && chbCalcCovarMat->checkState() == Qt::Unchecked)
        {
            size.setWidth(-2);
            size.setHeight(-2);
        }
        else
        {
            size.setWidth(-1);
            size.setHeight(-1);
        }

    }
    else
    {        
        size.setWidth(w.toInt());
        size.setHeight(h.toInt());
    }

    return size;
}

double Configuration::getParameterObject() const
{
    QString sigma = getParameter(tr("Сигма гауссойды"));
    double s = sigma.toDouble();

    if (sigma.isEmpty())
    {
        s = DBL_MAX;
    }

    return s;
}

QString Configuration::getResultFolder() const
{
    return getParameter(tr("Папка для записи результатов"));
}

double Configuration::getIntensity() const
{
    return getParameter(tr("Интенсивность")).toDouble();
}

QSize Configuration::getImageSize() const
{
    QSize size;
    QString w = getParameter(tr("Ширина изображения"));
    QString h = getParameter(tr("Высота изображения"));

    if (w.isEmpty() && h.isEmpty())
    {
        size.setWidth(-1);
        size.setHeight(-1);
    }
    else
    {
        size.setWidth(w.toInt());
        size.setHeight(h.toInt());
    }

    return size;
}

QString Configuration::getParameter(const QString& parameter) const
{
    QString value;
    QStandardItem* root = standardModel->invisibleRootItem();
    int rowCount = root->rowCount();

    for (int i = 0; i < rowCount; ++i)
    {
        if ((root->child(i)->data(Qt::DisplayRole).toString() == parameter) &&
            (!listConfig->isRowHidden(root->child(i)->row(), root->index())))
        {
            value = root->child(i, 1)->data(Qt::DisplayRole).toString();

            break;
        }
    }    

    return value;
}

void Configuration::createNewFile()
{
    QStandardItem* root = standardModel->invisibleRootItem();
    int rowCount = root->rowCount();
    QStandardItem* child;

    for (int i = 0; i < rowCount; ++i)
    {
        child = root->child(i, 1);

        if (child->data().toString().toUpper() == tr("int").toUpper())
        {
            child->setData(tr("0"), Qt::DisplayRole);
        }
        else
        {
            if (child->data().toString().toUpper() == tr("file").toUpper())
            {
                child->setData(tr("empty"), Qt::DisplayRole);
            }
            else
            {
                if (child->data().toString().toUpper() == tr("folder").toUpper())
                {
                    child->setData(tr("empty"), Qt::DisplayRole);
                }
                else
                {
                    child->setData(tr("0.0"), Qt::DisplayRole);
                }
            }
        }
    }

    emit modified();
}

void Configuration::selectMode(int mode)
{
    QStandardItem* root = standardModel->invisibleRootItem();
    QStandardItem* item;
    QString parameter;
    int rowCount = root->rowCount();

    if (mode == 0)
    {
        chbObject->setEnabled(false);
        chbCalcCovarMat->setEnabled(true);
    }
    else
    {
        if (mode == 1)
        {
            chbCalcCovarMat->setEnabled(true);
            chbObject->setEnabled(true);
        }
        else
        {
            chbCalcCovarMat->setEnabled(false);
            chbObject->setEnabled(true);
        }
    }

    for (int i = 0; i < rowCount; ++i)
    {
        item = root->child(i);
        parameter = root->child(i)->data(Qt::DisplayRole).toString().toUpper();

        if (parameter != tr("Папка для записи результатов").toUpper() && parameter != tr("Рамка").toUpper())
        {
            if (mode == 0)
            {
                if ((parameter == tr("Файл изображения для обработки").toUpper() && chbGenBackground->checkState() == Qt::Unchecked) ||
                   ((parameter == tr("Высота изображения").toUpper() || parameter == tr("Ширина изображения").toUpper()) && chbGenBackground->checkState() == Qt::Checked) ||
                    (parameter == tr("Файл изображения ковариационной матрицы").toUpper() && chbCalcCovarMat->checkState() == Qt::Unchecked) ||
                   ((parameter == tr("Высота фрагмента").toUpper() || parameter == tr("Ширина фрагмента").toUpper()) && chbCalcCovarMat->checkState() == Qt::Checked))
                {
                    listConfig->setRowHidden(item->row(), root->index(), false);
                }
                else
                {
                    listConfig->setRowHidden(item->row(), root->index(), true);
                }
            }
            else
            {
                if (mode == 1)
                {
                    if ((parameter == tr("Файл изображения для обработки").toUpper() && chbGenBackground->checkState() == Qt::Unchecked) ||
                       ((parameter == tr("Высота изображения").toUpper() || parameter == tr("Ширина изображения").toUpper()) && chbGenBackground->checkState() == Qt::Checked) ||
                        (parameter == tr("Файл изображения ковариационной матрицы").toUpper() && chbCalcCovarMat->checkState() == Qt::Unchecked) ||
                        (parameter == tr("Файл изображения объекта").toUpper() && chbObject->checkState() == Qt::Unchecked) ||
                        (parameter == tr("X координата объекта").toUpper() || parameter == tr("Y координата объекта").toUpper()) ||
                       ((parameter == tr("Сигма гауссойды").toUpper() || parameter == tr("Интенсивность").toUpper()) && chbObject->checkState() == Qt::Checked) ||
                       ((parameter == tr("Высота фрагмента").toUpper() || parameter == tr("Ширина фрагмента").toUpper() || parameter == tr("Сигма гауссойды").toUpper() || parameter == tr("Интенсивность").toUpper()) && chbObject->checkState() == Qt::Checked) &&
                       ((parameter == tr("Высота фрагмента").toUpper() || parameter == tr("Ширина фрагмента").toUpper()) && chbCalcCovarMat->checkState() == Qt::Checked))
                    {
                        listConfig->setRowHidden(item->row(), root->index(), false);
                    }
                    else
                    {
                        listConfig->setRowHidden(item->row(), root->index(), true);
                    }
                }
                else
                {
                    if ((parameter == tr("Файл изображения для обработки").toUpper() && chbGenBackground->checkState() == Qt::Unchecked) ||
                       ((parameter == tr("Высота изображения").toUpper() || parameter == tr("Ширина изображения").toUpper()) && chbGenBackground->checkState() == Qt::Checked) ||
                        (parameter == tr("Файл изображения объекта").toUpper() && chbObject->checkState() == Qt::Unchecked) ||
                        (parameter == tr("X координата объекта").toUpper() || parameter == tr("Y координата объекта").toUpper()) ||
                       ((parameter == tr("Высота фрагмента").toUpper() || parameter == tr("Ширина фрагмента").toUpper() || parameter == tr("Сигма гауссойды").toUpper() || parameter == tr("Интенсивность").toUpper()) && chbObject->checkState() == Qt::Checked))
                    {
                        listConfig->setRowHidden(item->row(), root->index(), false);
                    }
                    else
                    {
                        listConfig->setRowHidden(item->row(), root->index(), true);
                    }
                }

            }
        }
    }

    emit modified();
}

//void Configuration::selectMode(int mode)
//{
//    QStandardItem* root = standardModel->invisibleRootItem();
//    QStandardItem* item;
//    QString parameter = "";
//    int rowCount = root->rowCount();

//    for (int i = 0; i < rowCount; ++i)
//    {
//        item = root->child(i);
//        parameter = item->data(Qt::DisplayRole).toString().toUpper();

//        if (parameter != tr("Папка для записи результатов").toUpper() && parameter != tr("Рамка").toUpper())
//        {
//            switch (mode)
//            {
//                case 0:
//                {
//                    chbObject->setEnabled(false);
//                    chbCalcCovarMat->setEnabled(true);

//                    if ((item->data(Qt::DisplayRole).toString() == tr("Файл изображения для обработки") &&
//                         chbGenBackground->checkState() == Qt::Unchecked) ||
//                        (item->data(Qt::DisplayRole).toString() == tr("Файл изображения ковариационной матрицы") &&
//                         chbCalcCovarMat->checkState() == Qt::Unchecked) ||
//                        (item->data(Qt::DisplayRole).toString() == tr("Высота фрагмента")) ||
//                        (item->data(Qt::DisplayRole).toString() == tr("Ширина фрагмента")))
//                    {
//                        listConfig->setRowHidden(item->row(), root->index(), false);
//                    }
//                    else
//                    {
//                        listConfig->setRowHidden(item->row(), root->index(), true);
//                    }

//                    if (chbGenBackground->checkState() == Qt::Checked &&
//                       (item->data(Qt::DisplayRole).toString() == tr("Высота изображения") ||
//                        item->data(Qt::DisplayRole).toString() == tr("Ширина изображения")))
//                    {
//                        listConfig->setRowHidden(item->row(), root->index(), false);
//                    }

//                    break;
//                }

//                case 1:
//                {
//                    chbCalcCovarMat->setEnabled(true);
//                    chbObject->setEnabled(true);

//                    if ((item->data(Qt::DisplayRole).toString() == tr("Файл изображения для обработки") &&
//                         chbGenBackground->checkState() == Qt::Checked) ||
//                        (item->data(Qt::DisplayRole).toString() == tr("Файл изображения ковариационной матрицы") &&
//                         chbCalcCovarMat->checkState() == Qt::Checked) ||
//                        (item->data(Qt::DisplayRole).toString() == tr("Файл изображения объекта") &&
//                         chbObject->checkState() == Qt::Checked) ||
//                        ((item->data(Qt::DisplayRole).toString() == tr("Высота фрагмента") ||
//                          item->data(Qt::DisplayRole).toString() == tr("Ширина фрагмента") ||
//                          item->data(Qt::DisplayRole).toString() == tr("Сигма гауссойды")) &&
//                          chbObject->checkState() == Qt::Unchecked))
//                    {
//                        listConfig->setRowHidden(item->row(), root->index(), true);
//                    }
//                    else
//                    {
//                        listConfig->setRowHidden(item->row(), root->index(), false);
//                    }

//                    if (chbGenBackground->checkState() == Qt::Checked &&
//                       (item->data(Qt::DisplayRole).toString() == tr("Высота изображения") ||
//                        item->data(Qt::DisplayRole).toString() == tr("Ширина изображения")))
//                    {
//                        listConfig->setRowHidden(item->row(), root->index(), false);
//                    }

//                    break;
//                }

//                case 2:
//                {
//                    chbCalcCovarMat->setEnabled(false);
//                    chbObject->setEnabled(true);

//                    if ((item->data(Qt::DisplayRole).toString() == tr("Файл изображения для обработки") &&
//                         chbGenBackground->checkState() == Qt::Checked) ||
//                        (item->data(Qt::DisplayRole).toString() == tr("Файл изображения ковариационной матрицы")) ||
//                        (item->data(Qt::DisplayRole).toString() == tr("Файл изображения объекта") &&
//                         chbObject->checkState() == Qt::Checked) ||
//                        ((item->data(Qt::DisplayRole).toString() == tr("Высота фрагмента") ||
//                          item->data(Qt::DisplayRole).toString() == tr("Ширина фрагмента") ||
//                          item->data(Qt::DisplayRole).toString() == tr("Сигма гауссойды")) &&
//                          chbObject->checkState() == Qt::Unchecked))
//                    {
//                        listConfig->setRowHidden(item->row(), root->index(), true);
//                    }
//                    else
//                    {
//                        listConfig->setRowHidden(item->row(), root->index(), false);
//                    }

//                    if (chbGenBackground->checkState() == Qt::Checked &&
//                       (item->data(Qt::DisplayRole).toString() == tr("Высота изображения") ||
//                        item->data(Qt::DisplayRole).toString() == tr("Ширина изображения")))
//                    {
//                        listConfig->setRowHidden(item->row(), root->index(), false);
//                    }

//                    break;
//                }
//            }
//        }
//    }

//    emit modified();
//}

QString Configuration::checkParameters() const
{
    QStandardItem* root = standardModel->invisibleRootItem();
    QString param = "";
    double value = 0.0;
    int rowCount = root->row();

    for (int i = 0; i < rowCount; ++i)
    {
        param = root->child(i)->data(Qt::DisplayRole).toString().toUpper();
        value = root->child(i, 1)->data(Qt::DisplayRole).toDouble();

        if (param == tr("Высота фрагмента").toUpper() || param == tr("Ширина фрагмента").toUpper())
        {
            if (value < 0.0)
            {
                return tr("Высота и ширина фрагмента должны быть положительными целыми числами");
            }
        }
        else
        {
            if (param == tr("X координата объекта").toUpper() || param == tr("Y координата объекта").toUpper())
            {
                if (value < 0.0)
                {
                    return tr("Координаты объекта должны быть положтельными числами");
                }
            }
            else
            {
                if (param == tr("Сигма гауссойды").toUpper())
                {
                    if (value < 0.0)
                    {
                        return tr("Сигма гауссойды должна быть положительным числом");
                    }
                }
            }
        }
    }

    return QString();
}

void Configuration::hideParameterFileCovarMat(int state)
{
    QStandardItem* root = standardModel->invisibleRootItem();
    QStandardItem* item;
    QString parameter;
    int rowsCount = root->rowCount();

    for (int i = 0; i < rowsCount; ++i)
    {
        item = root->child(i);
        parameter = item->data(Qt::DisplayRole).toString().toUpper();

        if (parameter == tr("Файл изображения ковариационной матрицы").toUpper())
        {
            if (state == Qt::Checked)
            {
                listConfig->setRowHidden(item->row(), root->index(), true);
            }
            else
            {
                listConfig->setRowHidden(item->row(), root->index(), false);
            }
        }
        else
        {
            if (parameter == tr("Высота фрагмента").toUpper() || parameter == tr("Ширина фрагмента").toUpper())
            {
                if (state == Qt::Checked)
                {
                    listConfig->setRowHidden(item->row(), root->index(), false);
                }
                else
                {
                    listConfig->setRowHidden(item->row(), root->index(), true);
                }
            }
        }
    }

    emit modified();
}

void Configuration::hideParameterFileObject(int state)
{
    QStandardItem* root = standardModel->invisibleRootItem();
    QStandardItem* item;
    QString parameter;
    int rowsCount = root->rowCount();

    for (int i = 0; i < rowsCount; ++i)
    {
        item = root->child(i);
        parameter = item->data(Qt::DisplayRole).toString().toUpper();

        if (parameter == tr("Файл изображения объекта").toUpper())
        {
            if (state == Qt::Checked)
            {
                listConfig->setRowHidden(item->row(), root->index(), true);
            }
            else
            {
                listConfig->setRowHidden(item->row(), root->index(), false);
            }
        }

        if (parameter == tr("Сигма гауссойды").toUpper()|| parameter == tr("Интенсивность").toUpper() )
        {
            if (state == Qt::Checked)
            {
                listConfig->setRowHidden(item->row(), root->index(), false);
            }
            else
            {
                listConfig->setRowHidden(item->row(), root->index(), true);
            }
        }

        if ((parameter == tr("Высота фрагмента").toUpper() || parameter == tr("Ширина фрагмента").toUpper()) &&
             chbCalcCovarMat->checkState() == Qt::Checked)
        {
            if (state == Qt::Checked)
            {
                listConfig->setRowHidden(item->row(), root->index(), false);
            }
            else
            {
                listConfig->setRowHidden(item->row(), root->index(), true);
            }
        }
    }

    emit modified();
}

void Configuration::hideParameterFileImage(int state)
{
    QStandardItem* root = standardModel->invisibleRootItem();
    QStandardItem* item;
    QString parameter;
    int rowsCount = root->rowCount();

    for (int i = 0; i < rowsCount; ++i)
    {
        item = root->child(i);
        parameter = item->data(Qt::DisplayRole).toString().toUpper();

        if (parameter == tr("Файл изображения для обработки").toUpper())
        {
            if (state == Qt::Checked)
            {
                listConfig->setRowHidden(item->row(), root->index(), true);
            }
            else
            {
                listConfig->setRowHidden(item->row(), root->index(), false);
            }

            break;
        }
        else
        {
            if (parameter == tr("Высота изображения").toUpper() || parameter == tr("Ширина изображения").toUpper())
            {
                if (state == Qt::Checked)
                {
                    listConfig->setRowHidden(item->row(), root->index(), false);
                }
                else
                {
                    listConfig->setRowHidden(item->row(), root->index(), true);
                }
            }
        }
    }

    emit modified();
}

//переделать функцию,
void Configuration::readSettingFile(const QString &fileName)
{    
    QFile file(fileName);

    if (!file.open(QFile::ReadOnly) && !file.size())
    {
        //QMessageBox::critical(this, tr("Ошибка"), tr("Ошибка чтения файла ") + fileName);

        emit sendError(tr("Ошибка чтения файла ") + fileName);

        return;
    }

    QTextStream instream(&file);

    QStringList list;
    QString line = instream.readLine();

    QStandardItem* root = standardModel->invisibleRootItem();
    QStandardItem* childName;
    QStandardItem* childValue;

    while (!line.isNull())
    {
        if (line[0] == QChar('\n') || line == tr(""))
        {
            line = instream.readLine();

            continue;
        }

        list = line.split(QRegExp("[=|]"));

        for (int i = 0; i < list.count(); ++i)
        {
            list[i] = list[i].trimmed();
        }

        //qDebug() << list << "\n";

        childName = new QStandardItem(list[0]);
        childName->setEditable(false);
        childValue = new QStandardItem(list[1]);
        childValue->setData(list[2]);

        root->appendRow(QList<QStandardItem*>() << childName << childValue);

        line = instream.readLine();
    }    
}
