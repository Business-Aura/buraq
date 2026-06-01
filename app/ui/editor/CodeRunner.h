// CodeRunner.h — Play/Run button in editor margin
#ifndef CODERUNNER_H
#define CODERUNNER_H

#include <QPushButton>

class CodeRunner final : public QPushButton {
    Q_OBJECT

private slots:
    void runCode();

signals:
    void statusUpdate(QString status, int timeout = 10000);
    void updateOutputResult(int exitCode, const QString &output, const QString &error);

public:
    explicit CodeRunner(QWidget *parent = nullptr);
    ~CodeRunner() override;

private:
    QWidget* m_window;

    void setupSignals();
};

#endif //CODERUNNER_H
