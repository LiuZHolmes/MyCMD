#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QMessageBox>
#include <QProcess>
#include <QKeyEvent>
#include "stdio.h"
#include <unistd.h>
#include <stdlib.h>
#include <string>
#include <vector>
using namespace std;
// Command
const string EXIT = "exit";
const string BACK = "back";
const string FORWARD = "forward";
const string CD = "cd"; // Change Directory (CD)
// HTML
const QString BREAK = "<br />";
string HTMLColor(string s,string color)
{
    return "<span style='color:" + color + ";'>" + s + "</span>";
}
struct
{
    string directory = "/";
    string last_directory= "";
    void ClearLastDirectory()
    {
        last_directory= "";
    }
    string color = "#4169E1";
}DirectoryInf;
struct
{
    QString initial = "retr0@ ";
    string color = "#B0C4DE";
    QString initial_html = QString::fromStdString(HTMLColor(initial.toStdString(),color));
}CMD_Info;
vector<QString> Instructions; // Command history
int index_of_ins;
int isChangeDirectory(string s)
{
    // Back
    if (s == BACK)
    {
        return 0;
    }
    // Change directory
    if (s.find(CD) != string::npos)
    {
        return 1;
    }
    // Forward
    if (s == FORWARD)
    {
        return 2;
    }
    else return -1;
}
int FindSecondLast(string s,string note)
{
    int last = 0 , second_last = 0;
    while(1)
    {
        int result = s.find(note,last+1);
        if (result != string::npos)
        {
            second_last = last;
            last = result;
        }
        else return second_last == last ? -1 : second_last;
    }
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // Set default focus
    ui->lineEdit->setFocus();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
        {
        case Qt::Key_Up :
            index_of_ins = index_of_ins - 1 >= 0 ? index_of_ins - 1 : 0;
            ui->lineEdit->setText(Instructions[index_of_ins]);
            break ;
        case Qt::Key_Down :
            index_of_ins = index_of_ins + 1 <= Instructions.size() - 1 ? index_of_ins + 1 : Instructions.size() - 1;
            ui->lineEdit->setText(Instructions[index_of_ins]);
            break ;
        default :
            break ;
        }
}
QString ExecuteShell(QString command, QString previous)
{
    QString output ="";
    // Set Directory by default or last setting
    chdir((DirectoryInf.directory).c_str());
    // Print current directory
    output += QString::fromStdString(HTMLColor(DirectoryInf.directory,DirectoryInf.color)) + BREAK;
    // Print input command
    output += CMD_Info.initial_html + "  " + command + BREAK;
    // Require changing directory or not
    string s = command.toStdString();
    int C = isChangeDirectory(s);
    // Need to change
    if (C>=0)
    {
        if (C == 0)
        {
            int second_last = FindSecondLast(DirectoryInf.directory,"/");
            if(second_last != -1)
            {
                DirectoryInf.last_directory = DirectoryInf.directory.substr(second_last);
                DirectoryInf.last_directory = DirectoryInf.last_directory.substr(0,DirectoryInf.last_directory.length()-1);
                DirectoryInf.directory = DirectoryInf.directory.substr(0,second_last);
             }
         }
         if (C == 1)
         {
             string NewDirectory = s.substr(3);
             DirectoryInf.directory = DirectoryInf.directory + NewDirectory + "/";
             DirectoryInf.ClearLastDirectory();
         }
         if (C == 2)
         {
             DirectoryInf.directory = (DirectoryInf.last_directory == "") ? DirectoryInf.directory : (DirectoryInf.directory + DirectoryInf.last_directory + "/");
             DirectoryInf.ClearLastDirectory();
         }
         output += CMD_Info.initial_html + " >>  Change Directory to " + QString::fromStdString(DirectoryInf.directory) + BREAK;
    }
    // No need to change
    else
    {
        // Modify command
        command += " 2>&1";
        // Execute command and read the shell return value
        FILE* fp;
        char buffer[1024];
        if((fp = popen(command.toStdString().c_str(),"r")) != NULL)
        {
            output += CMD_Info.initial_html + " >> ";
            while(fgets(buffer,sizeof(buffer),fp) != NULL)
            {
                output += QString(QLatin1String(buffer)) + BREAK ;
            }
            pclose(fp);
        }
        else
        {
            // ERROR!
        }
    }
    return output;
}
QString ExecuteShellProcess(QString command,QString previous) // unfinished, unsolved
{
    QProcess *myProcess = new QProcess;
    QStringList options;
    options << command;
    myProcess->start("/bin/bash",options);
    QByteArray qOutput = myProcess->readAllStandardOutput();
    QList<QByteArray> list = qOutput.split('\n');
    QList<QByteArray>::iterator itor = list.begin();
    for ( ; itor != list.end(); itor++)
    {
       QByteArray strline = *itor;
       qDebug() << strline;
    }
    return "";
}
void MainWindow::on_pushButton_clicked()
{
    // Read input and previous content
    QString command = ui->lineEdit->text();
    QString previous = ui->textEdit->toPlainText();
    // Push new command to instruction vector
    if(Instructions.size() <= 0 || command != Instructions.back())
        Instructions.push_back(command);
    index_of_ins = Instructions.size() ;
    // Execute
    QString output = ExecuteShell(command,previous);
    // QString output = ExecuteShellProcess(command,previous);
    // Print output
    ui->textEdit->insertHtml(output);
    // Scroll
    QTextCursor cursor=ui->textEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->textEdit->setTextCursor(cursor);
    // Clear input line
    ui->lineEdit->setText("");
}
