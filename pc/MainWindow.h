#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "BLEInterface.h"
#include <QTimer>
#include <memory>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
private:
	Ui::MainWindow *ui;
	struct Private;
	Private *m;
	void dataReceived(QByteArray data, const QLowEnergyCharacteristic &c);
	void connectToDevice();
	void scanDevices();
	void connectionChanged(bool connected);
public:
    explicit MainWindow(QWidget *parent = 0);
	~MainWindow();
	void setButtons(uint8_t v);
	void showStatusMessage(QString text);
private slots:
	void devicesChanged();
	void on_action_connect_triggered();
	void on_servicesComboBox_currentIndexChanged(int index);
	void servicesChanged();
	void on_pushButton_clicked();

protected:
	void customEvent(QEvent *event);
};

#endif // MAINWINDOW_H
