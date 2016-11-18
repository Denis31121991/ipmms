#ifndef PICTUREWORKER_H
#define PICTUREWORKER_H

#include <QObject>
#include <QFutureWatcher>
#include <opencv2/opencv.hpp>

class QFile;

class PictureWorker : public QObject
{
    Q_OBJECT
public:
    explicit PictureWorker(QObject *parent = 0);
    ~PictureWorker();    

public slots:
    void beginCalculation(int mode, const QString& imageName, const QString& objName, const QString& covName,
                          const QString& dirName, int b, int x, int y, int fW, int fH, int iW, int iH, double s,
                          double i);
signals:
    void calculateFinished();
    void sendResultFolder(const QString& nameFolder, bool isVisibles);
    void sendError(const QString& error);

private:
    void setMode(int value);
    void setBorder(int pixels);
    void setPositionObject(int x, int y);
    bool setSizeFragment(int width, int height);
    void setSizeImage(int w, int h);
    void setParameterObject(double value);
    void setIntensityObj(double value);
    void setResultFolder(const QString& name);
    void calculate();
    bool loadBackgroundImage(const QString &fileName);
    bool loadObjectImage(const QString &fileName);
    bool loadCovarMatrixImage(const QString &fileName);
    void createGaussian();
    void addWhiteNoise();
    void createResultFolder() const;    
    bool writeImage(const QString& fileName, const cv::Mat& image) const;
    //void calculateCovarMatrix();
    void prepareImage(QVector<cv::Mat> &frags);
    void writeResult(QFile &file, const QString &text, const cv::Mat& mean,
                     const cv::Mat& stddev, double maxAmpl, double powerSignal) const;
    double calculatePowerSignal(const cv::Mat &dst);
    void calculateWhiteningMatrix(cv::Mat& filter);
    void createDiagEigenValMatrix(const cv::Mat& values, cv::Mat& dst) const;
    void calculateWhiteningTransform(const cv::Mat& filter, cv::Mat& distIm);
    cv::Mat calculatePatch(const cv::Mat &srcIm, const cv::Mat &filter) const;
    void writeText(const QString &fileName, const cv::Mat &image) const;
    void addObject();

    QFutureWatcher<cv::Mat> futureWatcher;
    cv::Mat background;
    cv::Mat object;
    cv::Mat covarMatrix;
    QString resultFolder;
    int mode;
    int border;
    int posX;
    int posY;
    int fragWidth;
    int fragHeight;
    double sigma;
    double intensity;
};

#endif // PICTUREWORKER_H
