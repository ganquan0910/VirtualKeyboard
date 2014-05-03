/*---------------------------------------------------------------------------------------------------------------------------------

Copyright (c) 2014 Arnaud Vazard

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

-----------------------------------------------------------------------------------------------------------------------------------*/

#include "VirtualKeyboard.h"


VirtualKeyboard::VirtualKeyboard(QWidget *w_parent) :
    QFrame(w_parent),
    ui(new Ui::VirtualKeyboard),
    mw_lineEdit(NULL),
    mw_plainTextEdit(NULL),
    mw_textEdit(NULL),
    mi_inputType(-1)
{
}


VirtualKeyboard::~VirtualKeyboard()
{
    if (this->ui != NULL) delete this->ui;
}


int VirtualKeyboard::initialisation(QWidget *w_inputWidget, QString s_language, bool b_displaySecondaryKeys, bool b_displayBorder)
{
    // --- Check type of the input field to bind to the keyboard
    if (this->mw_lineEdit = qobject_cast<QLineEdit *>(w_inputWidget))
    {
        this->mi_inputType = VIRTUALKEYBOARD_INPUT_LINEEDIT;
    }
    else if (this->mw_textEdit = qobject_cast<QTextEdit *>(w_inputWidget))
    {
        this->mi_inputType = VIRTUALKEYBOARD_INPUT_TEXTEDIT;
    }
    else if (this->mw_plainTextEdit = qobject_cast<QPlainTextEdit *>(w_inputWidget))
    {
        this->mi_inputType = VIRTUALKEYBOARD_INPUT_PLAINTEXTEDIT;
    }
    else return VIRTUALKEYBOARD_UNKNOWINPUTTYPE;

    // Set the focus on the input widget
    w_inputWidget->setFocus();

    // Save pointer to the default widget
    this->mw_defaultInputWidget = w_inputWidget;

    // --- Keymaps Initialisation
    if (s_language == "EN")
    {
        this->mlists_lowerKeymap << "q" << "w" << "e" << "r" << "t" << "y" << "u" << "i" << "o" << "p"
                                 << "a" << "s" << "d" << "f" << "g" << "h" << "j" << "k" << "l" << ""
                                 << "z" << "x" << "c" << "v" << "b" << "n" << "m";

        this->mlists_upperKeymap << "Q" << "W" << "E" << "R" << "T" << "Y" << "U" << "I" << "O" << "P"
                                 << "A" << "S" << "D" << "F" << "G" << "H" << "J" << "K" << "L" << ""
                                 << "Z" << "X" << "C" << "V" << "B" << "N" << "M";
    }
    else if (s_language == "FR")
    {
        this->mlists_lowerKeymap << "a" << "z" << "e" << "r" << "t" << "y" << "u" << "i" << "o" << "p"
                                 << "q" << "s" << "d" << "f" << "g" << "h" << "j" << "k" << "l" << "m"
                                 << "w" << "x" << "c" << "v" << "b" << "n" ;

        this->mlists_upperKeymap << "A" << "Z" << "E" << "R" << "T" << "Y" << "U" << "I" << "O" << "P"
                                 << "Q" << "S" << "D" << "F" << "G" << "H" << "J" << "K" << "L" << "M"
                                 << "W" << "X" << "C" << "V" << "B" << "N" ;
    }
    else
        return VIRTUALKEYBOARD_UNKNOWLANGUAGE;

    this->mlists_numbersKeymap << "1" << "2" << "3" << "4" << "5" << "6" << "7" << "8" << "9" << "0"
                               << "!" << "@" << "#" << "$" << "%" << "&&" << "*" << "(" << ")" << ""
                               << "," << "-" << "_" << "[" << "]" << "?" << ".";

    this->mlists_punctuationKeymap << "!" << "@" << "#" << "$" << "%" << "&&" << "*" << "(" << ")" << ""
                                   << "," << "-" << "_" << "[" << "]" << "?" << ".";

    // Setup widget's UI
    this->ui->setupUi(this);

    // --- Set CapsLock, numbers and punctuation off by default
    this->mb_isCapsOn = false;
    this->mb_isNumberOn = false;
    this->mb_isPunctuationOn = false;

    // Display secondary keys ?
    this->ui->frame_secondary->setVisible(b_displaySecondaryKeys);

    // Display border around keyboard ?
    this->setFrameShape(b_displayBorder ? QFrame::StyledPanel : QFrame::NoFrame);

    // Extraction of every QPushButton matching the regex "pushButton_principalKey_\\d\\d" into a list
    this->mlistw_principalKeys = this->findChildren<QPushButton *>(QRegExp("pushButton_principalKey_\\d\\d"));

    // --- Signals Mapping for non specific keys
    connect(&this->mo_mapper,   SIGNAL(mapped(int)),
            this,               SLOT(keyPressed(int)));

    for (int i_i = 0; i_i < this->mlistw_principalKeys.size(); ++i_i)
    {
        connect(this->mlistw_principalKeys.at(i_i), SIGNAL(clicked()),
                &this->mo_mapper,                   SLOT(map()));

        this->mo_mapper.setMapping(this->mlistw_principalKeys.at(i_i), i_i);
    }

    // --- Connection to change the input widget dynamically
    connect(qApp, &QApplication::focusChanged,
            this, &VirtualKeyboard::setInputWidget);

    // --- Set the initial keymap
    this->setKeymap(this->mlists_lowerKeymap);

    return VIRTUALKEYBOARD_SUCCESS;
}


void VirtualKeyboard::setKeymap(QList<QString> &lists_keys)
{
    for (int i_i = 0; i_i < this->mlistw_principalKeys.size(); ++i_i)
    {
        // if the index is superior to the size of "lists_keys" OR if the string at i_i in "lists_keys" is empty, we hide the button
        if (i_i >= lists_keys.size() || lists_keys.at(i_i).isEmpty())
        {
            this->mlistw_principalKeys.at(i_i)->hide();
        }
        else // We set the text of the key to the value of lists_keys[i_i]
        {
            this->mlistw_principalKeys.at(i_i)->setText(lists_keys.at(i_i));
            if (this->mlistw_principalKeys.at(i_i)->isHidden()) this->mlistw_principalKeys.at(i_i)->show();
        }
    }
}


void VirtualKeyboard::toggleCapsLock()
{
    // We change the state of the caps lock and reset to false the others states and buttons
    this->mb_isCapsOn = !this->mb_isCapsOn;
    this->mb_isNumberOn = false;
    this->ui->pushButton_principalKey_numbers->setText(VIRTUALKEYBOARD_BUTTONTEXT_NUMBERS_OFF);
    this->mb_isPunctuationOn = false;
    this->ui->pushButton_principalKey_punctuation->setText(VIRTUALKEYBOARD_BUTTONTEXT_PUNCTUATION_OFF);

    if (this->mb_isCapsOn)
    {
        this->setKeymap(this->mlists_upperKeymap);
        this->ui->pushButton_principalKey_caps->setStyleSheet("QPushButton { background-color: cyan; border-radius: 3px; }");
    }
    else
    {
        this->setKeymap(this->mlists_lowerKeymap);
        this->ui->pushButton_principalKey_caps->setStyleSheet("");
    }
}


void VirtualKeyboard::toggleNumbers()
{
    // We change the state of the "numbers" boolean and reset to false the others states and buttons
    this->mb_isNumberOn = !this->mb_isNumberOn;
    this->mb_isCapsOn = false;
    this->ui->pushButton_principalKey_caps->setStyleSheet("");
    this->mb_isPunctuationOn = false;
    this->ui->pushButton_principalKey_punctuation->setText(VIRTUALKEYBOARD_BUTTONTEXT_PUNCTUATION_OFF);

    if (this->mb_isNumberOn)
    {
        this->setKeymap(this->mlists_numbersKeymap);
        this->ui->pushButton_principalKey_numbers->setText(VIRTUALKEYBOARD_BUTTONTEXT_NUMBERS_ON);
    }
    else
    {
        this->setKeymap(this->mlists_lowerKeymap);
        this->ui->pushButton_principalKey_numbers->setText(VIRTUALKEYBOARD_BUTTONTEXT_NUMBERS_OFF);
    }
    this->ui->pushButton_principalKey_caps->setEnabled(!this->mb_isNumberOn);
}


void VirtualKeyboard::togglePunctuation()
{
    // We change the state of the "Punctuation" boolean and reset to false the others states and buttons
    this->mb_isPunctuationOn = !this->mb_isPunctuationOn;
    this->mb_isCapsOn = false;
    this->ui->pushButton_principalKey_caps->setStyleSheet("");
    this->mb_isNumberOn = false;
    this->ui->pushButton_principalKey_numbers->setText(VIRTUALKEYBOARD_BUTTONTEXT_NUMBERS_OFF);

    if (this->mb_isPunctuationOn)
    {
        this->setKeymap(this->mlists_punctuationKeymap);
        this->ui->pushButton_principalKey_punctuation->setText(VIRTUALKEYBOARD_BUTTONTEXT_PUNCTUATION_ON);
    }
    else
    {
        this->setKeymap(this->mlists_lowerKeymap);
        this->ui->pushButton_principalKey_punctuation->setText(VIRTUALKEYBOARD_BUTTONTEXT_PUNCTUATION_OFF);
    }
    this->ui->pushButton_principalKey_caps->setEnabled(!this->mb_isPunctuationOn);
}


void VirtualKeyboard::setInputWidget(QWidget *w_old, QWidget *w_new)
{
    Q_UNUSED(w_old)

    if (this->mw_lineEdit = qobject_cast<QLineEdit *>(w_new))
    {
        this->mi_inputType = VIRTUALKEYBOARD_INPUT_LINEEDIT;
    }
    else if (this->mw_textEdit = qobject_cast<QTextEdit *>(w_new))
    {
        this->mi_inputType = VIRTUALKEYBOARD_INPUT_TEXTEDIT;
    }
    else if (this->mw_plainTextEdit = qobject_cast<QPlainTextEdit *>(w_new))
    {
        this->mi_inputType = VIRTUALKEYBOARD_INPUT_PLAINTEXTEDIT;
    }
    else
        // If the widget is not of a supported type, we fall back on the default widget
        this->setInputWidget(NULL, this->mw_defaultInputWidget);
}


void VirtualKeyboard::keyPressed(int i_indexKey)
{
    if (this->mi_inputType == VIRTUALKEYBOARD_INPUT_LINEEDIT)
    {
        this->mw_lineEdit->insert(this->mlistw_principalKeys.at(i_indexKey)->text());
    }
    if (this->mi_inputType == VIRTUALKEYBOARD_INPUT_PLAINTEXTEDIT)
    {
        this->mw_plainTextEdit->insertPlainText(this->mlistw_principalKeys.at(i_indexKey)->text());
    }
    if (this->mi_inputType == VIRTUALKEYBOARD_INPUT_TEXTEDIT)
    {
        this->mw_textEdit->insertPlainText(this->mlistw_principalKeys.at(i_indexKey)->text());
    }
}


void VirtualKeyboard::on_pushButton_principalKey_space_clicked()
{
    if (this->mi_inputType == VIRTUALKEYBOARD_INPUT_LINEEDIT)
    {
        this->mw_lineEdit->insert(" ");
    }
    if (this->mi_inputType == VIRTUALKEYBOARD_INPUT_PLAINTEXTEDIT)
    {
        this->mw_plainTextEdit->insertPlainText(" ");
    }
    if (this->mi_inputType == VIRTUALKEYBOARD_INPUT_TEXTEDIT)
    {
        this->mw_textEdit->insertPlainText(" ");
    }
}


void VirtualKeyboard::on_pushButton_principalKey_backspace_clicked()
{
    if (this->mi_inputType == VIRTUALKEYBOARD_INPUT_LINEEDIT)
    {
        this->mw_lineEdit->backspace();
    }
    if (this->mi_inputType == VIRTUALKEYBOARD_INPUT_PLAINTEXTEDIT)
    {
        this->mw_plainTextEdit->textCursor().deletePreviousChar();
    }
    if (this->mi_inputType == VIRTUALKEYBOARD_INPUT_TEXTEDIT)
    {
        this->mw_textEdit->textCursor().deletePreviousChar();
    }
}


void VirtualKeyboard::on_pushButton_principalKey_caps_clicked()
{
    this->toggleCapsLock();
}


void VirtualKeyboard::on_pushButton_principalKey_numbers_clicked()
{
    this->toggleNumbers();
}


void VirtualKeyboard::on_pushButton_principalKey_punctuation_clicked()
{
    this->togglePunctuation();
}


void VirtualKeyboard::on_pushButton_principalKey_enter_clicked()
{
    // TODO Implement this slot
}


void VirtualKeyboard::on_pushButton_secondaryKey_copy_clicked()
{
    if (this->mi_inputType == VIRTUALKEYBOARD_INPUT_LINEEDIT)
    {
        this->mw_lineEdit->copy();
    }
    if (this->mi_inputType == VIRTUALKEYBOARD_INPUT_PLAINTEXTEDIT)
    {
        this->mw_plainTextEdit->copy();
    }
    if (this->mi_inputType == VIRTUALKEYBOARD_INPUT_TEXTEDIT)
    {
        this->mw_textEdit->copy();
    }
}


void VirtualKeyboard::on_pushButton_secondaryKey_cut_clicked()
{
    if (this->mi_inputType == VIRTUALKEYBOARD_INPUT_LINEEDIT)
    {
        this->mw_lineEdit->cut();
    }
    if (this->mi_inputType == VIRTUALKEYBOARD_INPUT_PLAINTEXTEDIT)
    {
        this->mw_plainTextEdit->cut();
    }
    if (this->mi_inputType == VIRTUALKEYBOARD_INPUT_TEXTEDIT)
    {
        this->mw_textEdit->cut();
    }
}


void VirtualKeyboard::on_pushButton_secondaryKey_paste_clicked()
{
    if (this->mi_inputType == VIRTUALKEYBOARD_INPUT_LINEEDIT)
    {
        this->mw_lineEdit->paste();
    }
    if (this->mi_inputType == VIRTUALKEYBOARD_INPUT_PLAINTEXTEDIT)
    {
        this->mw_plainTextEdit->paste();
    }
    if (this->mi_inputType == VIRTUALKEYBOARD_INPUT_TEXTEDIT)
    {
        this->mw_textEdit->paste();
    }
}