// MIT License
//
// Copyright (c)  "2025" Talik A. Kasozi
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

//
// Created by talik on 4/30/2024.
//

#ifndef MINION_H
#define MINION_H

#include <QObject>
#include <QString>
#include <queue>

class Minion final : public QObject {
Q_OBJECT

signals:

	// Signal the final result.
	void resultReady(const QVariant &result);

	// Signal that work is being done. Show progress.
	void progressUpdated(int percentage);

	// Signal that all work is done
	void workFinished();

	// This signal is emitted to ask the main thread to run a script.
	void runScriptRequested(const QString &script);

public slots:
	void processScript(const QString& script);
	// This is the main entry point for the worker's task.
	// It will be called when the QThread starts.
	void doWork(const std::function<QVariant()>& task);

	// Adds a worker's task.
	void addTask(const std::function<QVariant()>& task);
	void processNextTaskInQueue();

public:
	explicit Minion(QObject *qObject);

	Minion() = default;

	~Minion() override = default;

private:
	// No members object is moved to a thread
	std::queue<std::function<QVariant()>> m_taskQueue;
};


#endif //MINION_H
