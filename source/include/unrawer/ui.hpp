/*
 * UnRAWer - camera raw batch processor on top of OpenImageIO
 * Copyright (c) 2023 Erium Vladlen.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

#ifndef _UNRAWER_UI_HPP
#define _UNRAWER_UI_HPP

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>

#include <QtCore/QDebug>
#include <QtCore/QFileInfo>
#include <QtCore/QMimeData>
#include <QtCore/QProcess>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDropEvent>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QVBoxLayout>

#include <QtConcurrent/QtConcurrentRun>
#include <QtCore/QFutureWatcher>
#include <QtCore/QRandomGenerator>

#include "unrawer/process.hpp"

#define VERSION_MAJOR 1
#define VERSION_MINOR 45

void setPBarColor(QProgressBar *progressBar, const QColor &color = QColor("#05B8CC"));

class DropArea : public QLabel {
  Q_OBJECT // Macro needed to handle signals and slots

      public : DropArea();

signals:
  void filesDropped(QList<QUrl> urls); // New signal to be emitted when files are dropped

protected:
  void dragEnterEvent(QDragEnterEvent *event) override;
  void dropEvent(QDropEvent *event) override;
};

class MainWindow : public QMainWindow {
  Q_OBJECT // Macro needed to handle signals and slots
      public : MainWindow();
  void emitUpdateTextSignal(const QString &text) { emit updateTextSignal(text); } // Public method to emit the signal

signals:
  void updateTextSignal(const QString &text);
  void changeProgressBarColorSignal(QColor color);

public slots:
  void setProgressBarValueSlot(int value) { progressBar->setValue(value); }
  void changeProgressBarColorSlot(const QColor &color) { setPBarColor(progressBar, color); }

private slots:
  void restartApp();
  void reloadConfig();

  void toggleConsole(bool checked);
  void verbLevel();

  void toggleSubfldr(bool checked);
  void startProcessing(QList<QUrl> urls);
  void rngSettings();
  void frmtSettings();
  void bitSettings();
  void rawSettings();
  void halfSizeSettings(bool checked);
  void demSettings();
  void rclrSettings();
  void lutSettings();
  void lutPSettings();
  void denoiseSettings();
  void sharpSettings();
  void sharpKSettings();

  void prntSettings();

private:
  QFutureWatcher<bool> processingWatcher;
  QProgressBar *progressBar;

  QAction *con_enable;
  QList<QAction *> verbActions;

  QList<QAction *> rngActions;
  QList<QAction *> demActions;
  QList<QAction *> frmtActions;
  QList<QAction *> bitActions;
  QList<QAction *> rawActions;
  QList<QAction *> lutActions;
  QList<QAction *> lutPActions;
  QList<QAction *> denoiseActions;
  QList<QAction *> sharpActions;
  QList<QAction *> sharpKActions;
  QList<QAction *> rclrActions;
};

#endif // !_UNRAWER_UI_HPP
