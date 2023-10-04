#include "visualizer.h"
#include <QImage>
#include <QVBoxLayout>

#define WIDTH 512
#define HEIGHT 512

Visualizer::Visualizer(QWidget *parent) : QMainWindow(parent)
{
    imageLabel = new QLabel(this);
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(imageLabel);
    QWidget *centralWidget = new QWidget(this);
    centralWidget->setLayout(layout);
    setCentralWidget(centralWidget);
}

Visualizer::~Visualizer() 
{
    delete imageLabel;
}

void Visualizer::updateImage(uint8_t **state)
{
    QImage img = convertToQImage(state, WIDTH, HEIGHT);
    imageLabel->setPixmap(QPixmap::fromImage(img));
}

QImage Visualizer::convertToQImage(uint8_t **state, int width, int height)
{
    QImage img(width, height, QImage::Format_RGB32);
    for(int i = 0; i < height; ++i) {
        for(int j = 0; j < width; ++j) {
            int val = state[i][j];
            img.setPixelColor(j, i, QColor(val, val, val));
        }
    }
    return img;
}
