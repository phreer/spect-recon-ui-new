#include "resultdialog.h"
#include "ui_resultdialog.h"

ResultDialog::ResultDialog(const QVector<QPixmap>& result_array, const QPixmap& sinogram, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ResultDialog),
    result_array_(result_array),
    sinogram_(sinogram)
{
    ui->setupUi(this);
    ui->comboBoxResultIndex->addItem("Final Result");
    for (int i = 1; i < result_array_.size(); ++i) {
        ui->comboBoxResultIndex->addItem(QString::number(i));
    }
    ui->horizontalScrollBarResult->setValue(0);
    ui->horizontalScrollBarResult->setMinimum(0);
    ui->horizontalScrollBarResult->setMaximum(result_array_.size() - 1);
    ui->labelSinogramImage->setPixmap(sinogram_);
    ui->labelResultImage->setPixmap(result_array_[0]);
}

ResultDialog::~ResultDialog()
{
    delete ui;
}

void ResultDialog::on_horizontalScrollBarResult_valueChanged(int value)
{
    ui->labelResultImage->setPixmap(result_array_[value]);
    ui->comboBoxResultIndex->setCurrentIndex(value);
}


void ResultDialog::on_comboBoxResultIndex_currentTextChanged(const QString &arg1)
{
    bool ok = false;
    int index = arg1.toInt(&ok);
    if (!ok || index >= result_array_.size() || index < 0) {
        index = 0;
    }
    ui->horizontalScrollBarResult->setValue(index);
    ui->labelResultImage->setPixmap(result_array_[index]);
}
