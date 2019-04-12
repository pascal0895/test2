#include "stackwindow.h"
#include "ui_stackwindow.h"

CStackWindow::CStackWindow(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::CStackWindow)
{
  ui->setupUi(this);

  this->mainWindow=parent;
  this->keyName[SSLCertificates::KeyRSA]="rsa";
  this->keyName[SSLCertificates::KeyDSA]="dsa";
  this->keyName[SSLCertificates::KeyEC]="ec";
  this->stack.clear();

  this->windowsPositionSet=false;
  connect(this->ui->listWidget,SIGNAL(itemSelectionChanged()),this,SLOT(select_cert()));
  this->signing_cert.cert_type = nocert;
  this->ui->labelSignCert->setText("<none>");
}

CStackWindow::~CStackWindow()
{
  delete ui;
}

void CStackWindow::stack_empty(bool empty)
{
   this->ui->pushButtonPurge->setDisabled(empty);
   if (this->ui->listWidget->selectedItems().size() == 0) empty=true;
   this->ui->pushButtonDelete->setDisabled(empty);
   this->ui->pushButtonPop->setDisabled(empty);
   this->ui->pushButtonSelectSign->setDisabled(empty);
}

void CStackWindow::select_cert()
{
  bool empty;
  if (this->ui->listWidget->selectedItems().size() == 0)
  {
    empty=true;
  }
  else
  {
    empty=false;
  }
  this->ui->pushButtonDelete->setDisabled(empty);
  this->ui->pushButtonPop->setDisabled(empty);
  this->ui->pushButtonSelectSign->setDisabled(empty);
}

void CStackWindow::update_list()
{
  this->ui->listWidget->clear();

  this->stack_empty((this->stack.size() == 0));

  for (int i = 0; i < this->stack.size(); ++i)
  {
    QString dis="";
    if (stack[i].cert_type != nocert)
    {
      if (stack[i].cert_type == certificate) dis+="Cert :";
      else if (stack[i].cert_type == csr) dis+="CSR  :";
      dis+=stack[i].name;
    }
    else
    {
      dis+="<No cert> ";
    }
    dis+=" : ";
    if (stack[i].key_type != SSLCertificates::KeyNone)
    {
      dis+= this->keyName[stack[i].key_type] +'(' + stack[i].key_param +')';
    }
    else
    {
       dis+="<No key>";
    }
    this->ui->listWidget->addItem(dis);
  }
}

int CStackWindow::push_cert(CertData * cert)
{
  this->stack << *cert;
  this->update_list();
  return 0;
}

void CStackWindow::on_pushButtonHide_clicked()
{
  // Save position (restored on overrided function show()
    this->windowsPosition=this->pos();
    this->windowsPositionSet=true;
    //qDebug()<<this->windowsPosition.x() << "/" << this->windowsPosition.y();
    this->hide();
}

void CStackWindow::show()
{
    // Call parent show and move to stored position
    QDialog::show();
    if (this->windowsPositionSet)
      this->move(this->windowsPosition);
}

void CStackWindow::on_pushButtonPurge_clicked()
{
    this->stack.clear();
    this->update_list();
}

void CStackWindow::on_pushButtonDelete_clicked()
{
    QList<QListWidgetItem *> selected;

    selected=this->ui->listWidget->selectedItems();
    if (selected.size()==0) return;
    this->stack.removeAt(this->ui->listWidget->row(selected[0]));
    this->update_list();
}

void CStackWindow::on_pushButtonPop_clicked()
{
  QList<QListWidgetItem *> selected;
  // get selected item
  selected=this->ui->listWidget->selectedItems();
  if (selected.size()==0) return;
  emit pop_certificate(
        this->stack[
          this->ui->listWidget->row(selected[0])
        ]);
}

void CStackWindow::on_pushButtonSelectSign_clicked()
{
  QList<QListWidgetItem *> selected;
  // get selected item
  selected=this->ui->listWidget->selectedItems();
  if (selected.size()==0) return;
  CertData certSel=this->stack[this->ui->listWidget->row(selected[0])];
  if (certSel.cert_type != certificate || certSel.key_type == SSLCertificates::KeyNone)
  {
    QMessageBox::warning(this,"Cannot select","Must have a certificate and a key to be able to sign");
    return;
  }
  this->signing_cert=this->stack[this->ui->listWidget->row(selected[0])];
  QString display= this->signing_cert.name + " / " + this->keyName[this->signing_cert.key_type];

  this->ui->labelSignCert->setText(display);
}

CStackWindow::CertData CStackWindow::getSigningCert()
{
  return this->signing_cert;
}