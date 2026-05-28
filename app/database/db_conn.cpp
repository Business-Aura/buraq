//
// Created by talik on 8/15/2025.
//

#include <QMessageBox>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QFileInfo>
#include <filesystem> // Requires C++17. For older C++, use platform-specific directory iteration.
#include <fstream>

#include "../../include/buraq.h"

#include "./db_conn.h"

namespace database
{
    void setWorkspacePath(const QString& folderPath)
    {
        QSqlQuery query;
        if (!query.prepare(INSERT_WORKSPACE_SQL))
        {
            file_utils::file_log("Error preparing setWorkspacePath query: " + query.lastError().text().toStdString());
        }

        query.addBindValue(folderPath);
        if (!query.exec()) {
            file_utils::file_log("Error executing setWorkspacePath query: " + query.lastError().text().toStdString());
        }
    }

    QString getWorkspacePath()
    {
        QSqlQuery query;

        if (!query.exec(SELECT_WORKSPACE_SQL))
        {
             file_utils::file_log("Error executing getWorkspacePath query: " + query.lastError().text().toStdString());
        }

        if (query.next())
        {
            QString folderPath = query.value(0).toString();
            if (QFileInfo::exists(folderPath) && QFileInfo(folderPath).isDir())
            {
                return folderPath;
            }
            else
            {
                // Directory doesn't exist anymore, clear it
                clearWorkspacePath();
            }
        }

        return QString();
    }
    
    void clearWorkspacePath()
    {
        QSqlQuery query;
        if (!query.exec(DELETE_WORKSPACE_SQL))
        {
            file_utils::file_log("Error executing clearWorkspacePath query: " + query.lastError().text().toStdString());
        }
    }

    void setLastOpenedFilePath(const QString& filePath)
    {
        QSqlQuery query;
        if (!query.prepare(UPDATE_LAST_FILE_SQL))
        {
            file_utils::file_log("Error preparing setLastOpenedFilePath query: " + query.lastError().text().toStdString());
        }
        query.addBindValue(filePath);
        if (!query.exec()) {
            file_utils::file_log("Error executing setLastOpenedFilePath query: " + query.lastError().text().toStdString());
        }
    }

    QString getLastOpenedFilePath()
    {
        QSqlQuery query;
        if (!query.exec(SELECT_LAST_FILE_SQL))
        {
             file_utils::file_log("Error executing getLastOpenedFilePath query: " + query.lastError().text().toStdString());
        }
        if (query.next())
        {
            return query.value(0).toString();
        }
        return QString();
    }

    void clearLastOpenedFilePath()
    {
        QSqlQuery query;
        if (!query.exec(CLEAR_LAST_FILE_SQL))
        {
            file_utils::file_log("Error executing clearLastOpenedFilePath query: " + query.lastError().text().toStdString());
        }
    }

    QSqlError init_db()
    {
        QSqlQuery query;
        if (!query.exec(WORKSPACE_SQL))
        {
            file_utils::file_log("Error executing query: " + query.lastError().text().toStdString());
            return query.lastError();
        }

        // Schema migration: Add last_file_path column if it doesn't already exist in the existing table
        query.exec("ALTER TABLE workspace ADD COLUMN last_file_path VARCHAR;");

        return {};
    }

    bool db_conn()
    {
        using file_utils::file_log;

        file_log("Initiating DB connection..");

        QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        std::filesystem::path dirName = std::filesystem::path(appDataPath.toStdString()) / ".data";
        if (!std::filesystem::exists(dirName) && !std::filesystem::create_directories(dirName))
        {
            file_log("Failed to create directory: " + dirName.string());
        }

        std::filesystem::path dbPathName = dirName / "itools.db";
        std::string dbName = dbPathName.string();

        file_log("current dir: " + std::filesystem::current_path().string());
        file_log("DB name path: " + dbName);

        try
        {
            if (std::fstream db(dbName, std::ios::in); db.is_open())
            {
                // db file exists
                db.close();
            }
            else
            {
                // create the db file
                std::ofstream outputFile(dbName, std::ios::out);
                outputFile.close();
            }

            static QSqlDatabase dbEngine = QSqlDatabase::addDatabase("QSQLITE");
            dbEngine.setDatabaseName(QString::fromStdString(dbName));

            if (!dbEngine.open())
            {
                QSqlError error = dbEngine.lastError();

                QMessageBox::critical(nullptr, QObject::tr("Cannot open database"),
                                      "Unable to establish a database connection.\n"
                                      "Click Cancel to exit.",
                                      QMessageBox::Cancel);
                file_log("Failed to open DB connection.");
                file_log("DATABASE OPEN FAILED!");
                file_log("  Database file checked: " + dbName);
                file_log(&"  Error Type:"[error.type()]);
                file_log("  Error (Driver Text):" + error.driverText().toStdString());
                file_log(&"  Driver available:"[QSqlDatabase::isDriverAvailable("QSQLITE")]);
                file_log("  Error (Database Text):" + error.databaseText().toStdString());
                return false;
            }
            else
            {
                file_log("DB connection is good!"); // Initialize the database:
                if (QSqlError err = init_db(); err.type() != QSqlError::NoError)
                {
                    file_log("Error executing initializing db: " + err.text().toStdString());
                }
            }
        }
        catch (const std::ios_base::failure& failure)
        {
            file_log("failed: ");
            return false;
        }
        catch (...)
        {
            file_log("Exception catch all: db_conn failed!");
            return false;
        }

        return true;
    }
}
