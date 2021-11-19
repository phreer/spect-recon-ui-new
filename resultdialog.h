#ifndef RESULTDIALOG_H
#define RESULTDIALOG_H

#include <QDialog>
#include <QVector>
#include <QPixmap>
#include <QImage>

namespace Ui {
class ResultDialog;
}

class ResultDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ResultDialog(const QVector<QPixmap>& result_array, const QPixmap& sinogram, QWidget *parent = nullptr);
    ~ResultDialog();

private slots:
    void on_horizontalScrollBarResult_valueChanged(int value);

    void on_comboBoxResultIndex_currentTextChanged(const QString &arg1);

private:
    Ui::ResultDialog *ui;
    QVector<QPixmap> result_array_;
    QPixmap sinogram_;
};

#endif // RESULTDIALOG_H
