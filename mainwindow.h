#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QFileInfo>
#include <QProgressBar>
#include <QPushButton>
#include <QFutureWatcher>
#include <QVBoxLayout>
#include <unordered_map>


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

// 文件任务数据结构
struct FileTask {
    QString filePath;
    QString fileName;
    qint64 fileSize;
    QString fileType;
    double progress;
    bool isProcessing;
    qint64 outputFileSize;
    double compressionRatio;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    // 拖拽事件处理
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    // 按钮点击事件
    void on_CleanAllButton_clicked();
    void on_OutputFileButton_clicked();
    void on_startButton_clicked();
    // 删除单个任务
    void deleteTask(int row);
    // 进度更新槽
    void updateProgress(int row);

private:
    Ui::MainWindow *ui;
    // 任务数据
    std::vector<FileTask> tasks;
    // 进度条映射
    std::unordered_map<int, QProgressBar*> progressBars;
    // 输出目录
    QString outputDir;
    // 检查文件类型
    bool isSupportedFileType(const QString &filePath);
    // 获取文件类型图标
    QIcon getFileTypeIcon(const QString &fileType);
    // 添加任务到表格
    void addTaskToTable(const FileTask &task);
    // 开始处理单个文件
    void processFile(int taskIndex);
};
#endif // MAINWINDOW_H
