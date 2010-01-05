#include "note.h"
#include "settings.h"

#include <QtDebug>

#include <QTextStream>
#include <QClipboard>
#include <QApplication>

Note::Note(const QFileInfo& fileinfo)
	: file_info(fileinfo), text_edit(0), html_edit(0), content_changed(false)
{
	type = ((file_info.suffix()=="htm")||(file_info.suffix()=="html"))?type_html:type_text;//detecting note's type
	file.setFileName(file_info.absoluteFilePath());
	note_title = file_info.baseName();
	//
	load(); //loading note's content
	switch(type)
	{
	case type_text:
	case type_html:
		text_edit->setMouseTracking(settings.getNoteLinksOpen());
		connect(&settings, SIGNAL(NoteLinkOpenChanged()), this, SLOT(noteLinkOpenChanged()));
		//
		text_edit->setFont(settings.getNoteFont());
		connect(&settings, SIGNAL(NoteFontChanged()), this, SLOT(noteFontChanged()));
		//
		connect(text_edit, SIGNAL(textChanged()), this, SLOT(contentChanged()));
		//
		if(type==type_html)
		{
			connect(html_edit, SIGNAL(currentCharFormatChanged(const QTextCharFormat &)),
				this, SLOT(currentCharFormatChanged(const QTextCharFormat &)));
		}
		break;
	default:
		break;
	}
}

Note::~Note()
{
	if(text_edit) delete text_edit;
	if(html_edit) delete html_edit;
}

//Reading file
void Note::load()
{
	file.close();
	switch(type)
	{
	case type_text:
	case type_html:
		if(type==type_text) text_edit = new TextEdit<QTextEdit>();
		else if(type==type_html) html_edit = new TextEdit<QTextBrowser>();
		if(file.open(QIODevice::ReadOnly | QIODevice::Text))
		{
			QTextStream in(&file);
			QString text = in.readAll();
			if(type==type_text) text_edit->setText(text);
			else if(type==type_html) html_edit->setText(text);
			file.close();
		}
		else if(file.open(QIODevice::WriteOnly | QIODevice::Text)) file.close(); //If file don't exist, we creating it
		break;
	default: break;
	}
}

//Saving note
void Note::save(bool forced)
{
	if(!(content_changed || forced)) return; //If file doesn't need in saving, exiting from function
	file.close();
	if(!file.open(QFile::WriteOnly | QFile::Text)) return;
	QTextStream out(&file);
	switch(type)
	{
		case type_text:
			out << text_edit->text();
			break;
		case type_html:
			out << html_edit->text();
			break;
		default:
			break;
	}
	file.close();
	content_changed = false;
}

//Renaming note
void Note::rename(const QString& new_name)
{
	file.close();
	note_title = new_name;
	QString suffix = file_info.completeSuffix();
	QString fullname = QString("%1.%2").arg(note_title).arg(suffix);
	QString absolute_file_path = file_info.dir().absoluteFilePath(fullname);
	file.rename(absolute_file_path);
	file_info.setFile(file);
}

//Moving note to another folder
void Note::move(const QString& new_dirname)
{
	file.close();
	QString filename = file_info.fileName();
	QString absolute_file_path = QDir(new_dirname).absoluteFilePath(filename);
	file.rename(absolute_file_path);
	file_info.setFile(file);
}

//Removing note
bool Note::remove()
{
	file.close();
	return file.remove();
}

//Returning widget (it's can be placed to tabwidget)
QWidget* Note::widget()
{
	switch(type)
	{
		case type_text:
			return text_edit;
		case type_html:
			return html_edit;
	}
}

//Coping note's content to clipboard
void Note::copy() const
{
	QClipboard* clipboard = QApplication::clipboard();
	switch(type)
	{
		case type_text: clipboard->setText(text_edit->text()); break;
		case type_html: clipboard->setText(html_edit->text()); break;
	}
}

//Searching in a note's content
bool Note::find(const QString& text, bool next)
{
	switch(type)
	{
		case type_text:
		case type_html:
		{
			QTextEdit* editor;
			if(type==type_text) editor=text_edit;
			else if(type==type_html) editor=html_edit;
			//
			if(next) editor->setTextCursor(QTextCursor()); //search next
			else editor->unsetCursor(); //new search
			return editor->find(text);
		}
		default: return false;
	}
}

//Sending signal about changing format of a text under cursor
void Note::currentCharFormatChanged(const QTextCharFormat& format)
{
	emit formatChanged(format.font());
}

//Returning format of selected text
void Note::setSelFormat(const QTextCharFormat& format)
{
	if(type!=type_html) return;
	QTextCursor cursor = html_edit->textCursor();
	if(!cursor.hasSelection()) cursor.select(QTextCursor::WordUnderCursor);
	cursor.mergeCharFormat(format);
}

//Applying format to selected text
const QTextCharFormat Note::getSelFormat() const
{
	if(type!=type_html) return QTextCharFormat();
	QTextCursor cursor = html_edit->textCursor();
	return cursor.charFormat();
}

//------------------------------------------------------------------------------

void Note::contentChanged()
{
	content_changed = true;
}

void Note::noteLinkOpenChanged()
{
	bool is_link_open = settings.getNoteLinksOpen();
	QTextEdit* editor;
	if(type==type_text) editor=text_edit;
	else if(type==type_html) editor=html_edit;
	editor->setMouseTracking(is_link_open);
	if(!is_link_open)
	{
		editor->setExtraSelections(QList<QTextEdit::ExtraSelection>());
		editor->viewport()->setCursor(Qt::IBeamCursor);
	}
}

//If notes' font changed in the preferences, applying font to note
void Note::noteFontChanged()
{
	if(type==type_text) text_edit->setFont(settings.getNoteFont());
	else if(type==type_html) html_edit->setFont(settings.getNoteFont());
}
