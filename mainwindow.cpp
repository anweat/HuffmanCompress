#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QFileInfo>
#include <QProgressBar>
#include <QPushButton>
#include <QMessageBox>
#include <QDir>
#include <QFileDialog>
#include <QHeaderView>
#include <QTimer>
#include <iostream>
#include "task/submit_convertTask.h"
#include <thread>
#include <future>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    
    // 启用拖拽功能
    setAcceptDrops(true);
    ui->taskTable->setAcceptDrops(false); // 主窗口接收拖拽
    
    // 初始化表格
    ui->taskTable->setColumnCount(6);
    QStringList headers;
    headers << "图标" << "文件名" << "文件大小" << "压缩率" << "进度" << "操作";
    ui->taskTable->setHorizontalHeaderLabels(headers);
    
    // 设置列宽
    ui->taskTable->setColumnWidth(0, 30);
    ui->taskTable->setColumnWidth(1, 250);
    ui->taskTable->setColumnWidth(2, 100);
    ui->taskTable->setColumnWidth(3, 100);
    ui->taskTable->setColumnWidth(4, 200);
    ui->taskTable->setColumnWidth(5, 50);
    
    // 表头自适应
    ui->taskTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    
    // 设置默认输出目录为当前目录
    outputDir = QDir::currentPath();
    ui->OutputPlace->setText(outputDir);
}

MainWindow::~MainWindow()
{
    // 清理进度条
    for (auto &bar : progressBars) {
        delete bar.second;
    }
    progressBars.clear();
    
    delete ui;
}

// 拖拽进入事件
void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

// 拖拽放下事件
void MainWindow::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        QList<QUrl> urlList = mimeData->urls();
        for (const QUrl &url : urlList) {
            QString filePath = url.toLocalFile();
            if (isSupportedFileType(filePath)) {
                QFileInfo fileInfo(filePath);
                FileTask task;
                task.filePath = filePath;
                task.fileName = fileInfo.fileName();
                task.fileSize = fileInfo.size();
                task.outputFileSize = 0;
                task.fileType = fileInfo.suffix().toLower();
                task.progress = 0.0;
                task.compressionRatio = 0.0;
                task.isProcessing = false;
                
                tasks.push_back(task);
                addTaskToTable(task);
            } else {
                QMessageBox::warning(this, "不支持的文件类型", 
                                     QString("文件 %1 不是支持的BMP或HUF文件类型").arg(filePath));
            }
        }
        event->acceptProposedAction();
    }
}

// 检查是否为支持的文件类型
bool MainWindow::isSupportedFileType(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    QString suffix = fileInfo.suffix().toLower();
    return (suffix == "bmp" || suffix == "huf") && fileInfo.isFile();
}

// 获取文件类型图标
QIcon MainWindow::getFileTypeIcon(const QString &fileType)
{
    if (fileType == "bmp") {
        return QIcon::fromTheme("image-x-generic", QIcon("://icons/image.png"));
    } else if (fileType == "huf") {
        return QIcon::fromTheme("package-x-generic", QIcon("://icons/compressed.png"));
    }
    return QIcon::fromTheme("text-x-generic");
}

// 添加任务到表格
void MainWindow::addTaskToTable(const FileTask &task)
{
    int row = ui->taskTable->rowCount();
    ui->taskTable->insertRow(row);
    
    // 设置图标
    QTableWidgetItem *iconItem = new QTableWidgetItem(getFileTypeIcon(task.fileType), "");
    iconItem->setTextAlignment(Qt::AlignCenter);
    ui->taskTable->setItem(row, 0, iconItem);
    
    // 设置文件名
    QTableWidgetItem *nameItem = new QTableWidgetItem(task.fileName);
    nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);
    ui->taskTable->setItem(row, 1, nameItem);
    
    // 设置文件大小
    QString sizeText;
    if (task.fileSize < 1024) {
        sizeText = QString::number(task.fileSize) + " B";
    } else if (task.fileSize < 1024 * 1024) {
        sizeText = QString::number(task.fileSize / 1024.0, 'f', 2) + " KB";
    } else {
        sizeText = QString::number(task.fileSize / (1024.0 * 1024.0), 'f', 2) + " MB";
    }
    QTableWidgetItem *sizeItem = new QTableWidgetItem(sizeText);
    sizeItem->setTextAlignment(Qt::AlignCenter);
    sizeItem->setFlags(sizeItem->flags() & ~Qt::ItemIsEditable);
    ui->taskTable->setItem(row, 2, sizeItem);
    
    // 设置压缩率
    QTableWidgetItem *ratioItem = new QTableWidgetItem("- %");
    ratioItem->setTextAlignment(Qt::AlignCenter);
    ratioItem->setFlags(ratioItem->flags() & ~Qt::ItemIsEditable);
    ui->taskTable->setItem(row, 3, ratioItem);
    
    // 设置进度条
    QProgressBar *progressBar = new QProgressBar();
    progressBar->setValue(0);
    progressBar->setAlignment(Qt::AlignCenter);
    progressBars[row] = progressBar;
    ui->taskTable->setCellWidget(row, 4, progressBar);
    
    // 设置删除按钮
    QPushButton *deleteButton = new QPushButton("×");
    deleteButton->setStyleSheet("background-color: #f0f0f0; color: #ff0000; border-radius: 10px; min-width: 20px; max-width: 20px; min-height: 20px; max-height: 20px;");
    connect(deleteButton, &QPushButton::clicked, [=]() {
        deleteTask(row);
    });
    
    QWidget *buttonWidget = new QWidget();
    QHBoxLayout *buttonLayout = new QHBoxLayout(buttonWidget);
    buttonLayout->addWidget(deleteButton);
    buttonLayout->setAlignment(Qt::AlignCenter);
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonWidget->setLayout(buttonLayout);
    ui->taskTable->setCellWidget(row, 5, buttonWidget);
}

// 删除任务
void MainWindow::deleteTask(int row)
{
    if (row >= 0 && row < ui->taskTable->rowCount()) {
        // 如果任务正在处理，先停止处理
        if (row < tasks.size() && tasks[row].isProcessing) {
            tasks[row].isProcessing = false;
        }
        
        // 清理进度条
        auto it = progressBars.find(row);
        if (it != progressBars.end()) {
            delete it->second;
            progressBars.erase(it);
        }
        
        // 移除表格行和任务数据
        ui->taskTable->removeRow(row);
        
        // 更新任务列表和进度条映射
        tasks.erase(tasks.begin() + row);
        std::unordered_map<int, QProgressBar*> newProgressBars;
        for (auto &bar : progressBars) {
            if (bar.first > row) {
                newProgressBars[bar.first - 1] = bar.second;
            }
        }
        progressBars = newProgressBars;
    }
}

// 清除所有按钮点击
void MainWindow::on_CleanAllButton_clicked()
{
    // 停止所有处理中的任务
    for (auto &task : tasks) {
        task.isProcessing = false;
    }
    
    // 清理进度条
    for (auto &bar : progressBars) {
        delete bar.second;
    }
    progressBars.clear();
    
    // 清空表格和任务列表
    ui->taskTable->setRowCount(0);
    tasks.clear();
}

// 输出目录选择按钮点击
void MainWindow::on_OutputFileButton_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, "选择输出目录", outputDir);
    if (!dir.isEmpty()) {
        outputDir = dir;
        ui->OutputPlace->setText(outputDir);
    }
}

// 开始按钮点击
void MainWindow::on_startButton_clicked()
{
    if (tasks.empty()) {
        QMessageBox::warning(this, "警告", "请先添加文件");
        return;
    }
    
    if (outputDir.isEmpty()) {
        QMessageBox::warning(this, "警告", "请选择输出目录");
        return;
    }
    
    // 开始处理所有文件
    for (size_t i = 0; i < tasks.size(); i++) {
        if (!tasks[i].isProcessing) {
            processFile(i);
        }
    }
}

// 处理单个文件
void MainWindow::processFile(int taskIndex)
{
    if (taskIndex < 0 || taskIndex >= tasks.size()) {
        return;
    }
    
    tasks[taskIndex].isProcessing = true;
    
    // 获取任务信息
    FileTask &task = tasks[taskIndex];
    std::string inputPath = task.filePath.toStdString();
    
    // 构建输出文件路径
    QFileInfo inputFileInfo(task.filePath);
    QString outputFileName;
    if (task.fileType == "bmp") {
        outputFileName = inputFileInfo.completeBaseName() + ".huf";
    } else {
        outputFileName = inputFileInfo.completeBaseName() + ".bmp";
    }
    std::string outputPath = (outputDir + "/" + outputFileName).toStdString();
    
    // 更新进度条（需要在主线程中执行）
    QMetaObject::invokeMethod(this, "updateProgress", Qt::QueuedConnection, Q_ARG(int, taskIndex));
    
    // 根据文件类型提交不同的转换任务
    std::future<bool> future;
    if (task.fileType == "bmp") {
        // BMP转换为HUF
        future = submit_bmp2huf(inputPath, outputPath, &tasks[taskIndex].progress);
    } else if (task.fileType == "huf") {
        // HUF转换为BMP
        future = submit_huf2bmp(inputPath, outputPath, &tasks[taskIndex].progress);
    }
    
    // 异步等待任务完成
    std::thread([=, future = std::move(future), taskName = tasks[taskIndex].fileName.toStdString()]() mutable {
        try {
            // 等待任务完成
            bool success = future.get();
            
            // 在主线程中更新UI
            QMetaObject::invokeMethod(this, [=]() {
                if (success) {
                    // 任务成功完成
                    // 读取输出文件大小
                    QFileInfo outputFileInfo(QString::fromStdString(outputPath));
                    qint64 outputSize = outputFileInfo.size();
                    
                    // 计算压缩率
                    double ratio = 0.0;
                    if (tasks[taskIndex].fileSize > 0) {
                        if (tasks[taskIndex].fileType == "bmp") {
                            // 压缩时：(原文件大小 - 压缩后大小) / 原文件大小 * 100
                            ratio = ((double)(tasks[taskIndex].fileSize - outputSize) / tasks[taskIndex].fileSize) * 100;
                        } else {
                            // 解压缩时：(压缩后大小 - 原文件大小) / 原文件大小 * 100
                            ratio = ((double)(outputSize - tasks[taskIndex].fileSize) / tasks[taskIndex].fileSize) * 100;
                        }
                    }
                    
                    // 更新任务信息
                    tasks[taskIndex].outputFileSize = outputSize;
                    tasks[taskIndex].compressionRatio = ratio;
                    
                    // 更新表格中的压缩率
                    QTableWidgetItem *ratioItem = ui->taskTable->item(taskIndex, 3);
                    if (ratioItem) {
                        ratioItem->setText(QString::number(ratio, 'f', 2) + "%");
                    }
                    
                    // 移除成功弹窗，避免多任务时影响用户体验
                    // QMessageBox::information(this, "处理成功", 
                    //                        QString("文件 %1 处理完成！").arg(tasks[taskIndex].fileName));
                } else {
                    // 任务失败
                    QMessageBox::critical(this, "处理失败", 
                                          QString("处理文件 %1 时出错！").arg(tasks[taskIndex].fileName));
                    
                    // 转换失败时自动删除对应任务项
                    deleteTask(taskIndex);
                    return;
                }
                
                // 更新进度为100%
                if (taskIndex < tasks.size()) {
                    tasks[taskIndex].progress = 1.0;
                    tasks[taskIndex].isProcessing = false;
                    
                    auto it = progressBars.find(taskIndex);
                    if (it != progressBars.end()) {
                        it->second->setValue(100);
                    }
                }
            }, Qt::QueuedConnection);
        } catch (const std::exception& e) {
            // 记录错误到日志
            std::cerr << "处理文件失败: " << taskName << ": " << e.what() << std::endl;
            
            // 在主线程中处理错误
            QMetaObject::invokeMethod(this, [=]() {
                // 显示异常错误弹窗
                QMessageBox::critical(this, "处理失败", 
                                      QString("处理文件 %1 时发生异常: %2").arg(QString::fromStdString(taskName)).arg(QString::fromStdString(e.what())));
                
                // 转换失败时自动删除对应任务项
                if (taskIndex < tasks.size()) {
                    deleteTask(taskIndex);
                }
            }, Qt::QueuedConnection);
        }
    }).detach();
}

// 更新进度条
void MainWindow::updateProgress(int row)
{
    if (row >= 0 && row < tasks.size()) {
        auto it = progressBars.find(row);
        if (it != progressBars.end()) {
            it->second->setValue(static_cast<int>(tasks[row].progress * 100));
        }
        
        // 如果任务仍在处理，继续更新进度
        if (tasks[row].isProcessing) {
            QTimer::singleShot(100, this, [=]() {
                updateProgress(row);
            });
        }
    }
}
