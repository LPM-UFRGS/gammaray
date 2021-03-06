#ifndef POSTIKDIALOG_H
#define POSTIKDIALOG_H

#include <QDialog>

namespace Ui {
class PostikDialog;
}

class CartesianGridSelector;
class FileSelectorWidget;
class VariableSelector;
class GSLibParameterFile;
class CartesianGrid;

class PostikDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PostikDialog(QWidget *parent = 0);
    ~PostikDialog();

private slots:
    void onConfigureAndRun();
    void onPostikCompletes();
    void onSave();

private:
    Ui::PostikDialog *ui;
    CartesianGridSelector *m_Ik3dGridSelector;
    FileSelectorWidget* m_thresholdsSelector;
    FileSelectorWidget* m_fileForDistSelector;
    VariableSelector* m_variableForDistSelector;
    VariableSelector* m_weightForDistSelector;
    GSLibParameterFile* m_gpf_postik;
    CartesianGrid* m_cg_postprocess;
    void preview();
};

#endif // POSTIKDIALOG_H
