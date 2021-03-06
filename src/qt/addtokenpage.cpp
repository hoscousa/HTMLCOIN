#include "addtokenpage.h"
#include "ui_addtokenpage.h"
#include "guiconstants.h"
#include "wallet/wallet.h"
#include "clientmodel.h"
#include "walletmodel.h"
#include "token.h"
#include "qvalidatedlineedit.h"
#include "contractabi.h"
#include "validation.h"

#include <QRegularExpressionValidator>
#include <QMessageBox>

AddTokenPage::AddTokenPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AddTokenPage),
    m_tokenABI(0),
    m_model(0),
    m_clientModel(0)
{
    ui->setupUi(this);
    ui->lineEditContractAddress->setStyleSheet(STYLE_UNDERLINE);
    ui->lineEditTokenName->setStyleSheet(STYLE_UNDERLINE);
    ui->lineEditTokenSymbol->setStyleSheet(STYLE_UNDERLINE);
    ui->lineEditDecimals->setStyleSheet(STYLE_UNDERLINE);

    ui->labelDescription->setText(tr("(This is your wallet address which will be tied to the token for send/receive oprations)"));
    QFont font = QApplication::font();
    font.setPointSizeF(font.pointSizeF() * 0.8);
    ui->labelDescription->setFont(font);
    ui->labelSpacer->setFont(font);

    m_tokenABI = new Token();

    connect(ui->lineEditContractAddress, SIGNAL(textChanged(const QString &)), this, SLOT(on_addressChanged()));
    connect(ui->lineEditTokenName, SIGNAL(textChanged(const QString &)), SLOT(on_updateConfirmButton()));
    connect(ui->lineEditTokenSymbol, SIGNAL(textChanged(const QString &)), SLOT(on_updateConfirmButton()));

    if(ui->lineEditSenderAddress->isEditable())
        ((QValidatedLineEdit*)ui->lineEditSenderAddress->lineEdit())->setEmptyIsValid(false);
    m_validTokenAddress = false;
}

AddTokenPage::~AddTokenPage()
{
    delete ui;

    if(m_tokenABI)
        delete m_tokenABI;
    m_tokenABI = 0;
}

void AddTokenPage::setClientModel(ClientModel *clientModel)
{
    m_clientModel = clientModel;
    if (m_clientModel)
    {
        connect(m_clientModel, SIGNAL(numBlocksChanged(int,QDateTime,double,bool)), this, SLOT(on_numBlocksChanged()));
        on_numBlocksChanged();
    }
}

void AddTokenPage::clearAll()
{
    ui->lineEditContractAddress->setText("");
    ui->lineEditTokenName->setText("");
    ui->lineEditTokenSymbol->setText("");
    ui->lineEditDecimals->setText("");
    ui->lineEditSenderAddress->setCurrentIndex(-1);
}

void AddTokenPage::setModel(WalletModel *_model)
{
    m_model = _model;
}

void AddTokenPage::on_clearButton_clicked()
{
    clearAll();
}

void AddTokenPage::on_confirmButton_clicked()
{
    if(ui->lineEditSenderAddress->isValidAddress())
    {
        CTokenInfo tokenInfo;
        tokenInfo.strContractAddress = ui->lineEditContractAddress->text().toStdString();
        tokenInfo.strTokenName = ui->lineEditTokenName->text().toStdString();
        tokenInfo.strTokenSymbol = ui->lineEditTokenSymbol->text().toStdString();
        tokenInfo.nDecimals = ui->lineEditDecimals->text().toInt();
        tokenInfo.strSenderAddress = ui->lineEditSenderAddress->currentText().toStdString();

        if(m_model)
        {
            if(m_model->existTokenEntry(tokenInfo))
            {
                QMessageBox::information(this, tr("Token exist"), tr("The token already exist with the specified contract and sender addresses."));
            }
            else
            {
                m_model->addTokenEntry(tokenInfo);

                clearAll();

                if(!fLogEvents)
                {
                    QMessageBox::information(this, tr("Log events"), tr("Enable log events from the option menu in order to receive token transactions."));
                }
            }
        }
    }
}

void AddTokenPage::on_addressChanged()
{
    QString tokenAddress = ui->lineEditContractAddress->text();
    if(m_tokenABI)
    {
        m_tokenABI->setAddress(tokenAddress.toStdString());
        std::string name, symbol, decimals;
        bool ret = m_tokenABI->name(name);
        ret &= m_tokenABI->symbol(symbol);
        ret &= m_tokenABI->decimals(decimals);
        ui->lineEditTokenName->setText(QString::fromStdString(name));
        ui->lineEditTokenSymbol->setText(QString::fromStdString(symbol));
        ui->lineEditDecimals->setText(QString::fromStdString(decimals));
        m_validTokenAddress = ret;
    }
    ui->confirmButton->setEnabled(m_validTokenAddress);
}

void AddTokenPage::on_numBlocksChanged()
{
    ui->lineEditSenderAddress->on_refresh();
}

void AddTokenPage::on_updateConfirmButton()
{
    bool enabled = true;
    if(ui->lineEditTokenName->text().isEmpty())
    {
        enabled = false;
    }
    if(ui->lineEditTokenSymbol->text().isEmpty())
    {
        enabled = false;
    }
    enabled &= m_validTokenAddress;
    ui->confirmButton->setEnabled(enabled);
}