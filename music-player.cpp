#include "widget.h"
#include "ui_widget.h"
#include "playlist.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    myEdit = new QLineEdit(this);
    myVBox = new QVBoxLayout(this);
    myButton = new QPushButton("Search", this);
    playlistWidget = new QListWidget(this);
    posterWidget = new QListWidget(this);

    setupToolBar();

    songSlider = new QSlider(Qt::Horizontal, this);
    songSlider->setRange(0, 100);
    songSlider->setStyleSheet("margin: 5px;");

    songAudio = new QAudioOutput(this);
    player = new QMediaPlayer(this);
    player->setAudioOutput(songAudio);
    songAudio->setVolume(0.5);

    playlistWidget->setFixedSize(600, 200);
    myVBox->addWidget(playlistWidget, 0, Qt::AlignVCenter | Qt::AlignLeft);
    myVBox->addWidget(myEdit);
    myVBox->addWidget(myButton);
    myVBox->addWidget(toolbar);
    myVBox->addWidget(songSlider);
    setLayout(myVBox);

    connect(myButton, &QPushButton::clicked, this, &Widget::onSubmit);
    connect(playlistWidget, &QListWidget::itemClicked, this, &Widget::handlePlaylistSelection);
    connect(playlistWidget, &QListWidget::customContextMenuRequested, this, &Widget::showContextMenu);
    playlistWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(player, &QMediaPlayer::durationChanged, this, &Widget::onDurationChanged);
    connect(player, &QMediaPlayer::positionChanged, this, &Widget::onPositionChanged);
    connect(songSlider, &QSlider::sliderMoved, this, &Widget::onSliderMoved);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::onJsonResponse(QNetworkReply *reply)
{
    QByteArray myByteArray = reply->readAll();
    QJsonDocument myDoc = QJsonDocument::fromJson(myByteArray);
    QJsonObject myObj = myDoc.object();

    qDebug() << myByteArray;
    qDebug() << "resultCount:" << myObj.value("resultCount").toInt();

    QJsonArray myArray = myObj.value("results").toArray();
    n = 0;

    for (int i = 0; i < myArray.size(); i++) {
        QJsonObject obj = myArray[i].toObject();
        QString collectionName = obj.value("collectionName").toString();
        QString songTitle = obj.value("trackName").toString();
        QString artworkStr = obj.value("artworkUrl100").toString();

        qDebug() << collectionName << artworkStr;

        request_image(artworkStr, i);
        name[n] = collectionName;
        n++;
    }
}

void Widget::onImageResponse(QNetworkReply *reply)
{
    QNetworkRequest myRequest = reply->request();
    int original_i = myRequest.attribute(QNetworkRequest::User).toInt();
    img[original_i].loadFromData(reply->readAll());
    repaint();
}

void Widget::handleImageButtonClick(int index)
{
    qDebug() << "Image button clicked:" << index;
    if (index >= 0 && index < playlistUrls.size()) {
        QString songUrl = playlistUrls[index];
        player->setSource(QUrl(songUrl));
        player->play();
    }
}

void Widget::request_image(QString urlStr, int index)
{
    QNetworkAccessManager *m = new QNetworkAccessManager(this);
    QUrl myUrl(urlStr);
    QNetworkRequest myRequest(myUrl);
    myRequest.setAttribute(QNetworkRequest::User, QVariant(index));

    connect(m, &QNetworkAccessManager::finished, this, &Widget::onImageResponse);
    m->get(myRequest);
}

void Widget::onSubmit()
{
    QString idStr = myEdit->text();
    qDebug() << "id:" << idStr;

    QString urlStr = "https://itunes.apple.com/search?term=" + idStr + "&entity=album";
    QUrl myUrl(urlStr);

    searchAndPlay(idStr);

    myManager = new QNetworkAccessManager(this);
    QNetworkRequest myRequest(myUrl);

    connect(myManager, &QNetworkAccessManager::finished, this, &Widget::onJsonResponse);
    myManager->get(myRequest);
}

void Widget::searchAndPlay(QString &term)
{
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QString url = "https://itunes.apple.com/search?term=" + term + "&entity=song&limit=1";
    QNetworkRequest request((QUrl(url)));
    QNetworkReply *reply = manager->get(request);

    connect(reply, &QNetworkReply::finished, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray response = reply->readAll();
            QJsonDocument jsonDoc = QJsonDocument::fromJson(response);
            QJsonArray results = jsonDoc["results"].toArray();

            if (!results.isEmpty()) {
                QJsonObject song = results.first().toObject();
                QString trackName = song["trackName"].toString();
                QString artistName = song["artistName"].toString();
                QString previewUrl = song["previewUrl"].toString();

                qDebug() << "Playing:" << trackName << "by" << artistName;
                qDebug() << "Preview URL:" << previewUrl;

                currentSongUrl = previewUrl;
                TrackN = trackName;

                player->setSource(QUrl(previewUrl));
                player->play();
            }
        } else {
            qDebug() << "Search failed:" << reply->errorString();
        }
        reply->deleteLater();
    });
}

void Widget::paintEvent(QPaintEvent *evt)
{
    QPainter p(this);
    for (int i = 0; i < 20; i++) {
        p.drawImage(100 * i, 10, img[i]);
    }
}

void Widget::setupToolBar()
{
    toolbar = new QToolBar("Media Controls");
    QPushButton *playButton = new QPushButton("Play", this);
    QPushButton *pauseButton = new QPushButton("Pause", this);
    QPushButton *nextButton = new QPushButton("Next", this);
    QPushButton *prevButton = new QPushButton("Previous", this);
    QPushButton *queueButton = new QPushButton("Queue", this);
    QPushButton *favorite = new QPushButton("Favorite", this);
    QPushButton *remove = new QPushButton("Remove", this);

    toolbar->addWidget(playButton);
    toolbar->addWidget(pauseButton);
    toolbar->addWidget(prevButton);
    toolbar->addWidget(nextButton);
    toolbar->addWidget(queueButton);
    toolbar->addWidget(favorite);
    toolbar->addWidget(remove);

    connect(playButton, &QPushButton::clicked, this, &Widget::playSong);
    connect(pauseButton, &QPushButton::clicked, this, &Widget::pauseSong);
    connect(nextButton, &QPushButton::clicked, this, &Widget::skipToNextSong);
    connect(prevButton, &QPushButton::clicked, this, &Widget::skipToPreviousSong);
    connect(queueButton, &QPushButton::clicked, this, &Widget::addToPlaylist);
    connect(favorite, &QPushButton::clicked, this, &Widget::addToPlaylist);
    connect(remove, &QPushButton::clicked, this, &Widget::removeFromPlaylist);
}

void Widget::playSong()
{
    player->play();
}

void Widget::pauseSong()
{
    player->pause();
}

void Widget::skipToNextSong()
{
    if (currentIndex < playlistUrls.size() - 1) {
        currentIndex++;
        QString nextUrl = playlistUrls[currentIndex];
        qDebug() << "Playing next song:" << nextUrl;
        player->setSource(QUrl(nextUrl));
        player->play();
        playlistWidget->setCurrentRow(currentIndex);
    } else {
        qDebug() << "No next song in playlist.";
    }
}

void Widget::skipToPreviousSong()
{
    if (currentIndex > 0) {
        currentIndex--;
        QString prevUrl = playlistUrls[currentIndex];
        qDebug() << "Playing previous song:" << prevUrl;
        player->setSource(QUrl(prevUrl));
        player->play();
        playlistWidget->setCurrentRow(currentIndex);
    } else {
        qDebug() << "No previous song in playlist.";
    }
}

void Widget::handlePlaylistSelection()
{
    int selectedIndex = playlistWidget->currentRow();
    if (selectedIndex < 0 || selectedIndex >= playlistUrls.size()) {
        qDebug() << "Invalid playlist selection.";
        return;
    }
    QString selectedUrl = playlistUrls[selectedIndex];
    player->setSource(QUrl(selectedUrl));
    player->play();
}

void Widget::showContextMenu(const QPoint &pos)
{
    QListWidgetItem *item = playlistWidget->itemAt(pos);
    if (!item) return;

    QMenu contextMenu(this);
    QAction *addAction = contextMenu.addAction("Add to Playlist");
    QAction *removeAction = contextMenu.addAction("Remove from Playlist");
    QAction *selectedAction = contextMenu.exec(playlistWidget->viewport()->mapToGlobal(pos));

    if (selectedAction == addAction) {
        addToPlaylist();
    } else if (selectedAction == removeAction) {
        removeFromPlaylist();
    }
}

void Widget::addToPlaylist()
{
    QString currentSong = currentSongUrl.toString();
    QString track = TrackN;

    if (!playlistUrls.contains(currentSong)) {
        playlistUrls.append(currentSong);
        playlistWidget->addItem(track);
        qDebug() << "Added to favorites:" << track;
    } else {
        qDebug() << "Song already in favorites.";
    }
}

void Widget::removeFromPlaylist()
{
    QListWidgetItem *selectedItem = playlistWidget->currentItem();
    if (!selectedItem) return;

    QString songTitle = selectedItem->text();
    int index = playlistWidget->row(selectedItem);

    if (index >= 0 && index < playlistUrls.size()) {
        playlistUrls.removeAt(index);
        delete playlistWidget->takeItem(index);
        qDebug() << "Removed from favorites:" << songTitle;
    }
}

void Widget::mousePressEvent(QMouseEvent *event)
{
    QPoint globalPos = mapToGlobal(event->pos());
    QPoint widgetPos = playlistWidget->mapFromGlobal(globalPos);
    QListWidgetItem *item = playlistWidget->itemAt(widgetPos);
    if (item) {
        showContextMenu(widgetPos);
    }
}

void Widget::onDurationChanged(qint64 duration)
{
    songSlider->setRange(0, static_cast<int>(duration));
}

void Widget::onPositionChanged(qint64 position)
{
    songSlider->setValue(static_cast<int>(position));
}

void Widget::onSliderMoved(int position)
{
    player->setPosition(static_cast<qint64>(position));
}
