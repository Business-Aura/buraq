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
// Created by talik on 4/26/2024.
//

#ifndef IT_TOOLS_DB_CONN_H
#define IT_TOOLS_DB_CONN_H

#include <QSqlError>
#include <QSqlQuery>

#include "../FileObject.h"

namespace database
{
    constexpr auto WORKSPACE_SQL = "CREATE TABLE IF NOT EXISTS workspace(id INTEGER PRIMARY KEY, folder_path VARCHAR UNIQUE, last_file_path VARCHAR);";

    constexpr auto INSERT_WORKSPACE_SQL = "INSERT OR REPLACE INTO workspace(id, folder_path) VALUES(1, ?);";

    constexpr auto SELECT_WORKSPACE_SQL = "SELECT folder_path FROM workspace WHERE id = 1;";

    constexpr auto DELETE_WORKSPACE_SQL ="DELETE FROM workspace WHERE id = 1;";

    constexpr auto UPDATE_LAST_FILE_SQL = "UPDATE workspace SET last_file_path = ? WHERE id = 1;";

    constexpr auto SELECT_LAST_FILE_SQL = "SELECT last_file_path FROM workspace WHERE id = 1;";

    constexpr auto CLEAR_LAST_FILE_SQL = "UPDATE workspace SET last_file_path = NULL WHERE id = 1;";

    void setWorkspacePath(const QString& folderPath);
    QString getWorkspacePath();
    void clearWorkspacePath();

    void setLastOpenedFilePath(const QString& filePath);
    QString getLastOpenedFilePath();
    void clearLastOpenedFilePath();
    
    QSqlError init_db();
    bool db_conn();
}

#endif // IT_TOOLS_DB_CONN_H
