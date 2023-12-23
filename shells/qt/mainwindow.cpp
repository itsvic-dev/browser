#include "mainwindow.h"
#include "qboxlayout.h"
#include "qlabel.h"
#include "qlineedit.h"
#include "qnetworkaccessmanager.h"
#include "qnetworkreply.h"
#include "qnetworkrequest.h"
#include "renderview.h"
#include "version.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  m_centralWidget = new QWidget;
  m_accessManager = new QNetworkAccessManager(this);

  auto layout = new QVBoxLayout;

  auto urlLayout = new QHBoxLayout;
  m_urlBar = new QLineEdit("https://example.com");
  urlLayout->addWidget(new QLabel("URL:"));
  urlLayout->addWidget(m_urlBar);
  layout->addLayout(urlLayout);

  m_renderView = new RenderView;
  layout->addWidget(m_renderView);

  m_centralWidget->setLayout(layout);
  setCentralWidget(m_centralWidget);

  connect(m_urlBar, &QLineEdit::editingFinished, this,
          &MainWindow::onUrlChanged);

  connect(m_accessManager, &QNetworkAccessManager::finished, this,
          [=](QNetworkReply *reply) {
            QString answer = reply->readAll();
            m_renderView->setHtmlData(answer);
          });
}

void MainWindow::onUrlChanged() {
  m_request.setHeader(QNetworkRequest::UserAgentHeader, BROWSER_USER_AGENT);
  m_request.setUrl(m_urlBar->text());
  m_accessManager->get(m_request);
  m_renderView->setHtmlData("<p>Loading...</p>");
}
