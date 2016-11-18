#include "pictureworker.h"

#include <QDir>
#include <QTextStream>
#include <QDebug>
#include <QTime>
#include <QtConcurrentMap>

static cv::Mat calculateLocalCovMatrix(const cv::Mat& fragment);
static int covSide = 0;
static int fWidth = 0;
static int fHeight = 0;

PictureWorker::PictureWorker(QObject *parent) : QObject(parent)
{

}

PictureWorker::~PictureWorker()
{

}

bool PictureWorker::loadBackgroundImage(const QString &fileName)
{    
    cv::Mat temp = cv::imread(fileName.toStdString(), cv::IMREAD_UNCHANGED);

    if (!temp.data)
    {
        return false;
    }

    temp.convertTo(background, CV_64F);

    return true;
}

bool PictureWorker::loadObjectImage(const QString &fileName)
{
    cv::Mat temp = cv::imread(fileName.toStdString(), cv::IMREAD_UNCHANGED);

    if (!temp.data)
    {
        return false;
    }

    temp.convertTo(object, CV_64F);

    return true;
}

bool PictureWorker::loadCovarMatrixImage(const QString &fileName)
{    
    cv::Mat temp = cv::imread(fileName.toStdString(), cv::IMREAD_UNCHANGED);

    if (!temp.data)
    {
        return false;
    }

    temp.convertTo(covarMatrix, CV_64F);

    return true;
}

//void PictureWorker::beginCalculation(int mode, const QString &imageName, const QString &objName, const QString &covName,
//                                     const QString &dirName, int b, int x, int y, int fW, int fH, int iW, int iH,
//                                     double s, double i)
//{
//    setMode(mode);

//    if (imageName.isEmpty())
//    {
//        if (iW == 0 || iH == 0)
//        {
//            emit sendError(trUtf8("Неверное значение параметра 'Ширина фрагмента' или 'Высота фрагмента'"));

//            return;
//        }

//        setSizeImage(iW, iH);
//        addWhiteNoise();
//    }
//    else
//    {
//        if (!loadBackgroundImage(imageName))
//        {
//            emit sendError(trUtf8("Ошибка чтения файла ") + imageName);

//            return;
//        }
//    }

//    if (b >= qMin((background.rows + 1) / 2, (background.cols + 1) / 2))
//    {
//        emit sendError(trUtf8("Большое значение параметра 'Рамка'"));

//        return;
//    }

//    setBorder(b);

//    setResultFolder(dirName);

//    qDebug() << "fW: " << fW << "\nfH: " << fH;

//    if ((fW >= background.cols - fW - b || fH >= background.rows - fH - b) ||
//        (fW == 0 || fH == 0))
//    {
//        emit sendError(trUtf8("Неверное значение параметра 'Ширина фрагмента' или 'Высота фрагмента'"));

//        return;
//    }

//    setSizeFragment(fW, fH);

//    if (mode == 0 || mode == 1)
//    {
//        if (covName.isEmpty())
//        {
//            covSide = fragHeight * fragWidth;
//            fHeight = fragHeight;
//            fWidth = fragWidth;

//            covarMatrix = cv::Mat::zeros(covSide, covSide, CV_64F);

//            QVector<cv::Mat> vector;
//            prepareImage(vector);
//            futureWatcher.setFuture(QtConcurrent::mapped(vector, calculateLocalCovMatrix));
//            futureWatcher.waitForFinished();
//            QList<cv::Mat> list(futureWatcher.future().results());

//            for (int i = 0; i < list.count(); ++i)
//            {
//                covarMatrix += list[i];
//            }

//            covarMatrix /= list.count();

//            double* ptrCovData = (double*)covarMatrix.data;

//            for (int row = 0; row < covSide; ++row)
//            {
//                for (int col = row + 1; col < covSide; ++col)
//                {
//                    ptrCovData[col * covSide + row] = ptrCovData[row * covSide + col];
//                }
//            }
//        }
//        else
//        {
//            if (!loadCovarMatrixImage(covName))
//            {
//                emit sendError(trUtf8("Ошибка чтения файла ") + covName);

//                return;
//            }

//            if (covarMatrix.rows > background.rows || covarMatrix.cols > background.cols)
//            {
//                emit sendError(trUtf8("Размер ковариационной матрицы превышает размер изображения"));

//                return;
//            }
//        }
//    }

//    qDebug() << "sigma: " << s;

//    if (mode == 1 || mode == 2)
//    {
//        setIntensityObj(i);

//        if (s > 0)
//        {
//            setParameterObject(s);
//        }
//        else
//        {
//            emit sendError(trUtf8("Неверное значение параметра 'Сигма гауссойды'"));

//            return;
//        }

//        if (objName.isEmpty())
//        {
//            object = cv::Mat::zeros(fragHeight, fragWidth, CV_64F);
//            createGaussian();
//        }
//        else
//        {
//            if (!loadObjectImage(objName))
//            {
//                emit sendError(trUtf8("Ошибка чтения файла ") + objName);

//                return;
//            }
//        }

//        qDebug() << "x: " << x << "\ny: " << y << "\nborder: " << border;

//        if ((x <= border || x >= background.cols - fragWidth - border) ||
//            (y <= border || y >= background.rows - fragHeight - border))
//        {
//            emit sendError(trUtf8("Неверное значение параметра 'X координата объекта' или 'Y координата объекта'"));

//            return;
//        }
//        else
//        {
//            setPositionObject(x, y);
//        }
//    }

//    calculate();

//    emit calculateFinished();
//}

void PictureWorker::beginCalculation(int mode, const QString &imageName, const QString &objName, const QString &covName,
                                     const QString &dirName, int b, int x, int y, int fW, int fH, int iW, int iH,
                                     double s, double i)
{
    //qDebug() << "Settings\nborder: " << b << "\nx: " << x << "\ny: " << y << "\nfW: " << fW << "\nfH: " << fH <<
    //            "\ns: " << s << "\ni: " << i << "\n";
    setMode(mode);

    if (imageName.isEmpty())
    {
        if (iW == 0 || iH == 0)
        {
            emit sendError(trUtf8("Неверное значение параметра 'Ширина фрагмента' или 'Высота фрагмента'"));

            return;
        }

        setSizeImage(iW, iH);
        addWhiteNoise();
    }
    else
    {
        if (!loadBackgroundImage(imageName))
        {
            emit sendError(trUtf8("Ошибка чтения файла ") + imageName);

            return;
        }
    }

    if (b >= qMin((background.rows + 1) / 2, (background.cols + 1) / 2))
    {
        emit sendError(trUtf8("Большое значение параметра 'Рамка'"));

        return;
    }

    setBorder(b);

    setResultFolder(dirName);

    if (mode == 0 || mode == 1)
    {
        if (!covName.isEmpty())
        {
            if (!loadCovarMatrixImage(covName))
            {
                emit sendError(trUtf8("Ошибка чтения файла ") + covName);

                return;
            }
        }
    }

    if (mode == 1 || mode == 2)
    {
        if (!objName.isEmpty())
        {
            if (!loadObjectImage(objName))
            {
                emit sendError(trUtf8("Ошибка чтения файла ") + objName);

                return;
            }
        }
    }

    if ((fW >= background.cols - fW - b || fH >= background.rows - fH - b) ||
        (fW == 0 || fH == 0))
    {
        emit sendError(trUtf8("Неверное значение параметра 'Ширина фрагмента' или 'Высота фрагмента'"));

        return;
    }

    if (!setSizeFragment(fW, fH))
        return;

    if (mode == 1 || mode == 2)
    {
        setIntensityObj(i);

        if (s > 0)
        {
            setParameterObject(s);
        }
        else
        {
            emit sendError(trUtf8("Неверное значение параметра 'Сигма гауссойды'"));

            return;
        }

        if (objName.isEmpty())
        {
            object = cv::Mat::zeros(fragHeight, fragWidth, CV_64F);
            createGaussian();
        }

        //qDebug() << "x: " << x << "\ny: " << y << "\nborder: " << border;

        if ((x <= border || x >= background.cols - fragWidth - border) ||
            (y <= border || y >= background.rows - fragHeight - border))
        {
            emit sendError(trUtf8("Неверное значение параметра 'X координата объекта' или 'Y координата объекта'"));

            return;
        }
        else
        {
            setPositionObject(x, y);
        }
    }

    //qDebug() << "Settings\nborder: " << b << "\nx: " << x << "\ny: " << y << "\nfW: " << fW << "\nfH: " << fH <<
    //            "\ns: " << s << "\ni: " << i << "\n";

    if (mode == 0 || mode == 1)
    {
        if (covName.isEmpty())
        {
            covSide = fragHeight * fragWidth;
            fHeight = fragHeight;
            fWidth = fragWidth;

            covarMatrix = cv::Mat::zeros(covSide, covSide, CV_64F);

            QVector<cv::Mat> vector;
            prepareImage(vector);
            futureWatcher.setFuture(QtConcurrent::mapped(vector, calculateLocalCovMatrix));
            futureWatcher.waitForFinished();
            QList<cv::Mat> list(futureWatcher.future().results());

            for (int i = 0; i < list.count(); ++i)
            {
                covarMatrix += list[i];
            }

            covarMatrix /= list.count();

            double* ptrCovData = (double*)covarMatrix.data;

            for (int row = 0; row < covSide; ++row)
            {
                for (int col = row + 1; col < covSide; ++col)
                {
                    ptrCovData[col * covSide + row] = ptrCovData[row * covSide + col];
                }
            }
        }


        if (mode == 1)
        {
            if (object.rows != sqrt((double)covarMatrix.rows))
            {
                emit sendError(trUtf8("Несогласованные размеры ковариационной матрицы и объекта"));

                return;
            }
        }
    }

    if (object.rows > background.rows || object.cols > background.cols)
    {
        emit sendError(trUtf8("Размер объекта превышает размер изображения"));

        return;
    }

    //qDebug() << "object.rows: " << object.rows << "\nsqrt(covarMatrix.rows): " << sqrt((double)covarMatrix.rows);

    calculate();

    emit calculateFinished();
}

void PictureWorker::createGaussian()
{
    double r, s = 2.0 * sigma * sigma;
    double norma = 0.0;
    const double PI = 3.1415926535897932384626433832795;

    for (int x = -object.rows / 2; x <= object.rows / 2; x++)
    {
        for (int y = -object.cols / 2; y <= object.cols / 2 ; y++)
        {
            r = sqrt(double(x * x) + double(y * y));
            object.at<double>(x + object.rows / 2, y + object.cols / 2) =
                    (exp(-(r * r) / s)) / (PI * s);
        }
    }

    norma = object.at<double>(object.rows / 2, object.cols / 2);

    object /= norma;
    object *= intensity;
}

void PictureWorker::addWhiteNoise()
{
    cv::Mat m = cv::Mat::zeros(1, 1, CV_64F);
    cv::Mat s = cv::Mat::ones(1, 1, CV_64F);
    cv::RNG rng(65535);

    rng.fill(background, cv::RNG::NORMAL, m, s);
}

void PictureWorker::setMode(int value)
{
    mode = value;
}

void PictureWorker::setBorder(int pixels)
{
    border = pixels;
}

void PictureWorker::setPositionObject(int x, int y)
{
    posX = x;
    posY = y;
}

bool PictureWorker::setSizeFragment(int width, int height)
{
    if (width == -1 || height == -1)
    {        
        fragWidth = object.cols;
        fragHeight = object.rows;
    }
    else
    {
        if (width == -2 && width == -2)
        {
            fragWidth = sqrt(double(covarMatrix.cols));
            fragHeight = sqrt(double(covarMatrix.rows));
        }
        else
        {
            if (width % 2 == 0 || height % 2 == 0)
            {
                emit sendError(trUtf8("Неверное значение параметра 'Ширина фрагмента' или 'Высота фрагмента'.\n"
                                      "Значение должно быть нечетное"));

                return false;
            }

            fragWidth = width;
            fragHeight = height;
        }
    }

    return true;
}

void PictureWorker::setSizeImage(int w, int h)
{
    background = cv::Mat::zeros(h, w, CV_64F);
}

void PictureWorker::setParameterObject(double value)
{
    sigma = value;
}

void PictureWorker::setIntensityObj(double value)
{
    intensity = value;
}

void PictureWorker::setResultFolder(const QString &name)
{
    QFileInfo fInfo(name);
    resultFolder = fInfo.absoluteFilePath();

    //qDebug() << "Result: " << resultFolder << "\n";
}

void PictureWorker::calculate()
{
    createResultFolder();

    QString oldCurrent = QDir::currentPath();
    QDir::setCurrent(resultFolder);
    bool isVisible = false;

    cv::Mat mean, stddev;
    cv::meanStdDev(background, mean, stddev);
    background -= mean;    
    cv::meanStdDev(background, mean, stddev);

    QString name = tr("background.tif");

    if (!writeImage(name, background))
    {        
        emit sendError(trUtf8("Ошибка записи файла ") + name);

        return;
    }

    if (mode != 2)
    {
        name = tr("covarMatrix.tif");

        if (!writeImage(name, covarMatrix))
        {
            emit sendError(trUtf8("Ошибка записи файла ") + name);

            return;
        }
    }

    cv::Mat filter;
    cv::Mat outImage = cv::Mat::zeros(background.rows, background.cols, background.type());

    if (mode == 0)
    {         
        calculateWhiteningMatrix(filter);        
        calculateWhiteningTransform(filter, outImage);
    }
    else
    {
        name = tr("object.tif");

        if (!writeImage(name, object))
        {
            emit sendError(trUtf8("Ошибка записи файла ") + name);

            return;
        }        

        double centerValue = object.at<double>(object.rows / 2, object.cols / 2);
        double powerSignal = calculatePowerSignal(object);

        QFile file("result.txt");

        if (!file.open(QFile::WriteOnly))
        {
            emit sendError(trUtf8("Ошибка записи файла result.txt"));

            return;
        }

        writeResult(file, trUtf8("Исходное изображение"), mean, stddev, centerValue, powerSignal);

        cv::Mat noiseIm = background.clone();

        addObject();

        name = tr("srcImWithObj.tif");

        if (!writeImage(name, background))
        {
            emit sendError(trUtf8("Ошибка записи файла ") + name);

            return;
        }

        cv::Mat outNoiseIm = cv::Mat::zeros(noiseIm.rows, noiseIm.cols, noiseIm.type());
        cv::Mat outObject;

        if (mode == 1)
        {
            filter = calculatePatch(object, covarMatrix.inv());
        }
        else
        {
            filter = object.clone();
        }

        name = tr("filter.tif");

        if (!writeImage(name, filter))
        {
            emit sendError(trUtf8("Ошибка записи файла ") + name);

            return;
        }

        cv::filter2D(background, outImage, -1, filter, cv::Point(-1, -1), 0, cv::BORDER_CONSTANT);
        cv::filter2D(noiseIm, outNoiseIm, -1, filter, cv::Point(-1, -1), 0, cv::BORDER_CONSTANT);
        cv::filter2D(object, outObject, -1, filter, cv::Point(-1, -1), 0, cv::BORDER_CONSTANT);

        name = tr("outNoiseIm.tif");

        if (!writeImage(name, outNoiseIm))
        {
            emit sendError(trUtf8("Ошибка записи файла ") + name);

            return;
        }

        int halfBorder = filter.rows / 2;

        cv::meanStdDev(outNoiseIm(cv::Rect(halfBorder, halfBorder,
                                           outNoiseIm.cols - halfBorder, outNoiseIm.rows - halfBorder)), mean, stddev);
        outNoiseIm -= mean;

        centerValue = outObject.at<double>(outObject.rows / 2, outObject.cols / 2);
        powerSignal = calculatePowerSignal(outObject);

        name = tr("outObject.tif");

        if (!writeImage(name, outObject))
        {
            emit sendError(trUtf8("Ошибка записи файла ") + name);

            return;
        }

        isVisible = true;

        cv::meanStdDev(outNoiseIm(cv::Rect(halfBorder, halfBorder,
                                           outNoiseIm.cols - halfBorder, outNoiseIm.rows - halfBorder)), mean, stddev);

        writeResult(file, trUtf8("Фильтрованное изображение"), mean, stddev, centerValue, powerSignal);
    }

    name = tr("outImage.tif");

    if (!writeImage(name, outImage))
    {
        emit sendError(trUtf8("Ошибка записи файла ") + name);

        return;
    }

    QDir::setCurrent(oldCurrent);

    emit sendResultFolder(resultFolder, isVisible);
}

void PictureWorker::createResultFolder() const
{
    QDir folder(resultFolder);

    if (!folder.exists(resultFolder))
    {
        //qDebug() << "creating: " << resultFolder;
        folder.mkdir(resultFolder);
    }        
}

bool PictureWorker::writeImage(const QString &fileName, const cv::Mat &image) const
{
    {
        QFileInfo fileInfo(fileName);
        QString nameTxtFile = fileInfo.absoluteDir().path() + QDir::separator() + fileInfo.baseName() + QString(".txt");
        writeText(nameTxtFile, image);
    }

    cv::Mat result(image.rows, image.cols, CV_16U);

    double min;
    double max;
    cv::minMaxIdx(image, &min, &max);

    //qDebug() << "min: " << min << "\nmax: " << max << "\nrows: " << result.rows;

    double scale = (65535) / (max - min);

    //qDebug() << "scale: " << scale;

    for (int row = 0; row < result.rows; row++)
    {
        for (int col = 0; col < result.cols; col++)
        {
            result.at<unsigned short int>(row, col) = (unsigned short int)(scale * (image.at<double>(row, col) - min));
        }
    }    

    return cv::imwrite(fileName.toStdString(), result);
}

//void PictureWorker::calculateCovarMatrix()
//{
//    int count = 0;
//    int sizeSamples = ((background.cols - 2 * border) - fragWidth + 1) *
//                      ((background.rows - 2 * border) - fragHeight + 1);
//    cv::Mat sample;

//    int sz = covarMatrix.rows;
//    int rowCount = background.rows - border - fragWidth;
//    int colCount = background.cols - border - fragHeight;
//    double* ptrCovData = (double*)covarMatrix.data;
//    double* ptrSampleData;

//    for (int row = border; row <= rowCount; ++row)
//    {
//        for (int col = border; col <= colCount; ++col)
//        {
//            sample = background(cv::Rect(col, row, fragWidth, fragHeight)).clone();
//            ptrSampleData = (double*)sample.data;

//            for (int i = 0; i < sz; ++i)
//            {
//                for (int j = i; j < sz; ++j)
//                {
//                    ptrCovData[i * sz + j] += ptrSampleData[i] * ptrSampleData[j];
//                }
//            }

//            //qDebug() << ptrCovData[25] << "\n";

//            count++;
//        }
//    }

//    covarMatrix /= count;

//    for (int row = 0; row < covarMatrix.rows; ++row)
//    {
//        for (int col = row + 1; col < covarMatrix.cols; ++col)
//        {
//            ptrCovData[col * sz + row] = ptrCovData[row * sz + col];
//        }
//    }
//}

cv::Mat calculateLocalCovMatrix(const cv::Mat& fragment)
{
    int count = 0;
    cv::Mat localCovMat = cv::Mat::zeros(covSide, covSide, CV_64F);
    cv::Mat sample;
    int rowCount = fragment.rows - fHeight;
    int colCount = fragment.cols - fWidth;
    double* ptrCovData = (double*)localCovMat.data;
    double* ptrSampleData = 0;

    for (int row = 0; row <= rowCount; ++row)
    {
        for (int col = 0; col <= colCount; ++col)
        {
            sample = fragment(cv::Rect(col, row, fWidth, fHeight)).clone();
            ptrSampleData = (double*)sample.data;

            for (int i = 0; i < covSide; ++i)
            {
                for (int j = i; j < covSide; ++j)
                {
                    ptrCovData[i * covSide + j] += ptrSampleData[i] * ptrSampleData[j];
                }
            }

            count++;
        }
    }

    //qDebug() << count << '\n';

    localCovMat /= count;

    return localCovMat;

}

void PictureWorker::prepareImage(QVector<cv::Mat> &frags)
{
    int idealThreadCount = QThread::idealThreadCount();
    background = background(cv::Rect(border, border, background.cols - border, background.rows - border)).clone();

    int dy = (background.rows) / idealThreadCount;

    for (int y = 0; y <= background.rows - dy; y += dy)
    {
        frags << background(cv::Rect(0, y, background.cols, dy));
    }
}

void PictureWorker::writeResult(QFile& file, const QString& text, const cv::Mat &mean,
                                const cv::Mat &stddev, double maxAmpl, double powerSignal) const
{
    QTextStream outstream(&file);

    double sqrStddev = stddev.at<double>(0,0) * stddev.at<double>(0,0);

    outstream << text << '\n'
              << trUtf8("           Среднее: ") << mean.at<double>(0, 0) << '\n'
              << trUtf8("             Сигма: ") << stddev.at<double>(0, 0) << '\n'
              << trUtf8("           Сигма^2: ") << sqrStddev << '\n'
              << trUtf8("    Макс.Амплитуда: ") << maxAmpl << '\n'
              << trUtf8(" Мощьность Сигнала: ") << powerSignal << '\n'
              << trUtf8("Отнош. Сигнал/Шум1: ") << maxAmpl / stddev.at<double>(0, 0) << '\n'
              << trUtf8("Отнош. Сигнал/Шум2: ") << powerSignal / sqrStddev << '\n';
}

double PictureWorker::calculatePowerSignal(const cv::Mat& dst)
{
    double powerSignal = 0.0;

    for (int row = 0; row < dst.rows; ++row)
    {
        for (int col = 0; col < dst.cols; ++col)
        {
            powerSignal += dst.at<double>(row, col) * dst.at<double>(row, col);
        }
    }

    powerSignal /= (dst.rows * dst.cols);

    return powerSignal;
}

void PictureWorker::calculateWhiteningMatrix(cv::Mat &filter)
{
    cv::Mat w, u, vt;
    cv::SVD::compute(covarMatrix, w, u, vt);    
    cv::Mat diagMat = cv::Mat::zeros(w.rows, w.rows, covarMatrix.type());
    createDiagEigenValMatrix(w, diagMat);
    filter = diagMat * vt;
}

void PictureWorker::createDiagEigenValMatrix(const cv::Mat &values, cv::Mat &dst) const
{
    for (int row = 0; row < dst.rows; ++row)
    {
        for (int col = 0; col < dst.cols; ++col)
        {
            if (col == row)
            {
                 dst.at<double>(row, col) = 1.0/sqrt(values.at<double>(row,0));
            }
        }
    }
}

cv::Mat PictureWorker::calculatePatch(const cv::Mat& srcIm, const cv::Mat& filter) const
{
    cv::Mat patch = cv::Mat::zeros(srcIm.rows, srcIm.cols, srcIm.type());
    double* vectIm = new double[srcIm.rows * srcIm.cols];
    double* tempIm = new double[srcIm.rows * srcIm.cols];
    int vectSize = 0;

    for (int row = 0; row < srcIm.rows ; ++row)
    {
        for (int col = 0; col < srcIm.cols; ++col)
        {
            vectIm[vectSize] = srcIm.at<double>(row, col);
            tempIm[vectSize] = 0;
            vectSize++;
        }
    }

    vectSize = 0;

    for (int i = 0; i < filter.rows; ++i)
    {
        for (int j = 0; j < filter.rows; ++j)
        {
            tempIm[i] += filter.at<double>(i, j) * vectIm[j];
        }
    }

    vectSize = 0;

    for (int row = 0; row < patch.rows; ++row)
    {
        for (int col = 0; col < patch.cols; ++col)
        {
            patch.at<double>(row, col) = tempIm[vectSize++];
        }
    }

    delete[] vectIm;
    delete[] tempIm;
    return patch;
}

void PictureWorker::calculateWhiteningTransform(const cv::Mat &filter, cv::Mat &distIm)
{
    for (int row = border; row <= background.rows - border - fragHeight; row += fragHeight)
    {
        for (int col = border; col <= background.cols - border - fragWidth; col += fragWidth)
        {
            cv::Mat regionIm = background(cv::Rect(col, row, fragWidth, fragHeight));
            distIm(cv::Rect(col, row, fragWidth, fragHeight)) += calculatePatch(regionIm, filter);            
        }
    }    
}

void PictureWorker::addObject()
{
    for (int row = 0; row < object.rows; ++row)
    {
        for (int col = 0; col < object.cols; ++col)
        {
            background.at<double>(row + posY, col + posX) += object.at<double>(row, col);
        }
    }
}

void PictureWorker::writeText(const QString &fileName, const cv::Mat &image) const
{
    QFile file(fileName);

    if (!file.open(QFile::WriteOnly))
    {
        //emit sendError(trUtf8("Ошибка чтения файла ") + fileName);

        return;
    }

    QTextStream outstream(&file);
    outstream.setFieldWidth(15);

    for (int i = 0; i < image.rows; ++i)
    {
        for (int j = 0; j < image.cols; ++j)
        {
            outstream << image.at<double>(i, j);
        }

        outstream << '\n';
    }

    file.close();
}
