#ifndef VISUALIZER_H
#define VISUALIZER_H

#include <QMainWindow>
#include <QLabel>
#include <QThread>

class Visualizer : public QMainWindow {
    Q_OBJECT

public:
    explicit Visualizer(QWidget *parent = nullptr);
    ~Visualizer();
    
public slots:
    void updateImage(uint8_t **state);  // Convert this function into a slot

signals:
    void imageReady(uint8_t **state);   // Signal when a new image is ready

private:
    QLabel *imageLabel;

    QImage convertToQImage(uint8_t **state, int width, int height);
};

#endif // VISUALIZER_H
